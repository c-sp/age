//
// Copyright (c) 2010-2017 Christoph Sprenger
//
// This file is part of AGE ("Another Gameboy Emulator").
// <https://gitlab.com/csprenger/AGE>
//
// AGE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// AGE is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with AGE.  If not, see <http://www.gnu.org/licenses/>.
//

#include <algorithm>

#include <age_debug.hpp>

#include "age_ui_qt_gl_renderer.hpp"

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif



//---------------------------------------------------------
//
//   object creation & destruction
//
//---------------------------------------------------------

age::qt_gl_renderer::video_frame_manager::video_frame_manager()
{
    LOG("initializing opengl 3.0 functions");
    m_opengl_3.initializeOpenGLFunctions();
    m_max_texture_size = get_max_texture_size(m_opengl_3);

    m_opengl_3.glGenFramebuffers(1, &m_frame_buffer_id);
    LOG("frame buffer id is " << m_frame_buffer_id << ", glGetError = " << m_opengl_3.glGetError());

    m_opengl_3.glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer_id); // why is this necessary?
    m_opengl_3.glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

age::qt_gl_renderer::video_frame_manager::~video_frame_manager()
{
    LOG("deleting frame buffer " << m_frame_buffer_id);
    m_opengl_3.glDeleteFramebuffers(1, &m_frame_buffer_id);
}



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

GLint age::qt_gl_renderer::video_frame_manager::get_max_texture_size(QOpenGLFunctions_3_0 &m_opengl_3)
{
    GLint max_texture_size = -1;
    m_opengl_3.glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
    AGE_ASSERT(max_texture_size > 0);
    return max_texture_size;
}



GLuint age::qt_gl_renderer::video_frame_manager::get_frame_width() const
{
    return m_frame_width;
}

GLuint age::qt_gl_renderer::video_frame_manager::get_frame_height() const
{
    return m_frame_height;
}



void age::qt_gl_renderer::video_frame_manager::set_bilinear(bool bilinear)
{
    if (bilinear != m_bilinear)
    {
        LOG("changing bilinear filter to " << bilinear);

        m_bilinear = bilinear;

        std::for_each(begin(m_frames), end(m_frames),
                      [&] (typename frame_array::reference element)
        {
            element.m_base_texture.set_bilinear(m_bilinear);
            element.m_filtered_texture.set_bilinear(m_bilinear);
        });
    }
}



void age::qt_gl_renderer::video_frame_manager::set_frame_size(GLuint width, GLuint height)
{
    // first we clear all textures used for filtering, then we set the base texture's new size
    // (we do this in order to prevent out-of-memory errors when changing the base texture's size)

    AGE_ASSERT(width > 0);
    AGE_ASSERT(height > 0);

    m_shader_textures.clear();

    m_frame_width = width;
    m_frame_height = height;

    std::for_each(begin(m_frames), end(m_frames),
                  [&] (typename frame_array::reference element)
    {
        element.m_filtered_texture.assure_size(1, 1);
        element.m_base_texture.assure_size(m_frame_width, m_frame_height);
        element.m_base_texture.clear();
        element.m_filtered = false;
    });

    apply_shader_chain();
}



void age::qt_gl_renderer::video_frame_manager::set_filter_chain(const qt_filter_vector &filter_chain)
{
    m_shader_chain.clear();

    std::for_each(begin(filter_chain), end(filter_chain),
                  [&] (typename qt_filter_vector::const_reference element)
    {
        switch (element)
        {
        case qt_filter::scale2x:
            m_shader_chain.push_back(&m_shader_scale2x);
            break;

        case qt_filter::age_scale2x:
            m_shader_chain.push_back(&m_shader_age_scale2x);
            break;

        case qt_filter::gauss3x3:
            m_shader_chain.push_back(&m_shader_gauss3x3_first_pass);
            m_shader_chain.push_back(&m_shader_gauss3x3_second_pass);
            break;

        case qt_filter::gauss5x5:
            m_shader_chain.push_back(&m_shader_gauss5x5_first_pass);
            m_shader_chain.push_back(&m_shader_gauss5x5_second_pass);
            break;

        case qt_filter::emboss3x3:
            m_shader_chain.push_back(&m_shader_emboss3x3);
            break;

        case qt_filter::emboss5x5:
            m_shader_chain.push_back(&m_shader_emboss5x5);
            break;

        default:
            break;
        }
    });

    apply_shader_chain();
}



void age::qt_gl_renderer::video_frame_manager::add_new_frame(const pixel_vector &pixel_data)
{
    // jump to next frame
    ++m_current_frame;
    if (m_current_frame >= qt_video_frame_history_size)
    {
        m_current_frame = 0;
    }

    // update base texture
    AGE_ASSERT(pixel_data.size() >= m_frame_width * m_frame_height);
    frame &fr = m_frames[m_current_frame];
    fr.m_base_texture.update(pixel_data);
    fr.m_filtered = false;

    // do not update filtered texture here (will be done on demand during binding)
}



void age::qt_gl_renderer::video_frame_manager::bind_frame(uint frame_index)
{
    AGE_ASSERT(frame_index < qt_video_frame_history_size);

    // calculate absolute frame index
    uint index = m_current_frame - frame_index;
    if (index > qt_video_frame_history_size)
    {
        index += qt_video_frame_history_size;
    }

    AGE_ASSERT(index < qt_video_frame_history_size);
    frame &fr = m_frames[index];

    // filter frame, if not done yet
    filter_frame(fr);

    // bind filtered texture, if available (bind base texture otherwise)
    texture &tex = fr.m_filtered ? fr.m_filtered_texture : fr.m_base_texture;
    tex.bind();
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

void age::qt_gl_renderer::video_frame_manager::apply_shader_chain()
{
    // clear old stuff
    m_shader_textures.clear();
    m_scale_factor = m_shader_chain.empty() ? 0 : 1;

    // check chain for textures that are too big
    if (m_scale_factor > 0)
    {
        std::remove_if(begin(m_shader_chain), end(m_shader_chain),
                       [&] (typename std::vector<shader_program*>::reference element)
        {
            m_scale_factor *= element->get_scale_factor();

            GLuint new_width = m_frame_width * m_scale_factor;
            GLuint new_height = m_frame_height * m_scale_factor;

            bool result;
            if ((new_width > m_max_texture_size) || (new_height > m_max_texture_size))
            {
                LOG("removing filter from shader chain since the resulting texture would be too big: " << new_width << " x " << new_height);
                result = true;
            }
            else
            {
                result = false;
            }
            return result;
        });
    }

    // resize/allocate textures required for filtering
    try
    {
        // clear filtered textures, if we don't use any filter chain
        if (m_scale_factor == 0)
        {
            std::for_each(begin(m_frames), end(m_frames),
                          [&] (typename frame_array::reference element)
            {
                element.m_filtered_texture.assure_size(1, 1);
                element.m_filtered_texture.clear();
                element.m_filtered = false;
            });
        }

        // resize filtered textures, if we use a filter chain
        else
        {
            GLuint filtered_width = static_cast<GLuint>(m_frame_width * m_scale_factor);
            GLuint filtered_height = static_cast<GLuint>(m_frame_height * m_scale_factor);

            std::for_each(begin(m_frames), end(m_frames),
                          [&] (typename frame_array::reference element)
            {
                assure_size(element.m_filtered_texture, filtered_width, filtered_height);
                element.m_filtered = false;
            });
        }

        //  create textures for filtering
        if (m_shader_chain.size() > 1)
        {
            m_shader_textures = std::vector<texture>(m_shader_chain.size() - 1);

            GLuint texture_width = m_frame_width;
            GLuint texture_height = m_frame_height;

            for (uint i = 0, steps = m_shader_chain.size() - 1; i < steps; ++i)
            {
                uint factor = m_shader_chain[i]->get_scale_factor();

                texture_width *= factor;
                texture_height *= factor;

                assure_size(m_shader_textures[i], texture_width, texture_height);
            }
        }
    }
    catch (qt_filter)
    {
        // If this is thrown, resizing/allocating the textures needed for filtering failed.
        // In this case we simply don't use any frame filtering.
        LOG("resizing/allocating the textures required for frame filtering failed, discarding filter chain");
        m_shader_chain.clear();
        apply_shader_chain();
    }
}



void age::qt_gl_renderer::video_frame_manager::assure_size(texture &tex, GLuint width, GLuint height)
{
    bool result = tex.assure_size(width, height);
    if (!result)
    {
        throw qt_filter::scale2x;
    }
}



void age::qt_gl_renderer::video_frame_manager::filter_frame(frame &fr)
{
    if ((m_scale_factor > 0) && !fr.m_filtered)
    {
        m_opengl_3.glPushAttrib(GL_VIEWPORT_BIT);
        m_opengl_3.glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer_id);

        GLsizei source_texture_width = static_cast<GLsizei>(m_frame_width);
        GLsizei source_texture_height = static_cast<GLsizei>(m_frame_height);
        fr.m_base_texture.set_bilinear(false); // turn off bilinear filtering since it messes up some rendering results

        for (uint i = 0, steps = m_shader_chain.size() - 1; i <= steps; ++i)
        {
            // set framebuffer target texture
            texture &target = (i == steps) ? fr.m_filtered_texture : m_shader_textures[i];
            m_opengl_3.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.get_id(), 0);

            // prepare filter shader program (set uniforms & bind program)
            shader_program &filter = *m_shader_chain[i];
            GLfloat texture_width = static_cast<GLfloat>(source_texture_width);
            GLfloat texture_height = static_cast<GLfloat>(source_texture_height);
            filter.prepare(texture_width, texture_height);

            // set viewport
            GLsizei target_texture_width = static_cast<GLsizei>(source_texture_width * filter.get_scale_factor());
            GLsizei target_texture_height = static_cast<GLsizei>(source_texture_height * filter.get_scale_factor());
            m_opengl_3.glViewport(0, 0, target_texture_width, target_texture_height);

            // render filtered frame
            m_opengl_3.glColor4d(1.0, 1.0, 1.0, 1.0);
            m_opengl_3.glBegin(GL_TRIANGLE_STRIP);
            m_opengl_3.glTexCoord2d(0.0, 0.0); m_opengl_3.glVertex2d(0.0, 0.0);
            m_opengl_3.glTexCoord2d(0.0, 1.0); m_opengl_3.glVertex2d(0.0, 1.0);
            m_opengl_3.glTexCoord2d(1.0, 0.0); m_opengl_3.glVertex2d(1.0, 0.0);
            m_opengl_3.glTexCoord2d(1.0, 1.0); m_opengl_3.glVertex2d(1.0, 1.0);
            m_opengl_3.glEnd();

            // prepare next filter step
            source_texture_width  = target_texture_width;
            source_texture_height = target_texture_height;

            m_opengl_3.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
            target.bind(); // use target as source for next filter
        }

        m_opengl_3.glUseProgram(0);
        m_opengl_3.glBindFramebuffer(GL_FRAMEBUFFER, 0);
        m_opengl_3.glPopAttrib(); // viewport

        fr.m_base_texture.set_bilinear(m_bilinear);
        fr.m_filtered = true;
    }
}
