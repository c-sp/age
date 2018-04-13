//
// Copyright (c) 2010-2018 Christoph Sprenger
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

#include "age_ui_qt_gl_renderer.hpp"



//---------------------------------------------------------
//
//   object creation & destruction
//
//---------------------------------------------------------

age::qt_gl_renderer::texture::texture()
{
    m_opengl_3.initializeOpenGLFunctions();
    m_opengl_3.glGenTextures(1, &m_texture_id);

    set_bilinear(false);
    m_opengl_3.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_opengl_3.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    assure_size(1, 1);
}

age::qt_gl_renderer::texture::~texture()
{
    m_opengl_3.glDeleteTextures(1, &m_texture_id);
}



//---------------------------------------------------------
//
//   read-only methods
//
//---------------------------------------------------------

GLuint age::qt_gl_renderer::texture::get_width() const
{
    return m_width;
}

GLuint age::qt_gl_renderer::texture::get_height() const
{
    return m_height;
}

GLuint age::qt_gl_renderer::texture::get_id() const
{
    return m_texture_id;
}

void age::qt_gl_renderer::texture::bind()
{
    m_opengl_3.glBindTexture(GL_TEXTURE_2D, m_texture_id);
}



//---------------------------------------------------------
//
//   modifying methods
//
//---------------------------------------------------------

void age::qt_gl_renderer::texture::update(const pixel_vector &pixel_data)
{
    AGE_ASSERT(m_width * m_height <= pixel_data.size());

    bind();
    m_opengl_3.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data.data());
}

void age::qt_gl_renderer::texture::set_bilinear(bool bilinear)
{
    m_filter_param = bilinear ? GL_LINEAR : GL_NEAREST;
    m_opengl_3.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filter_param);
    m_opengl_3.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filter_param);
}

bool age::qt_gl_renderer::texture::assure_size(GLuint width, GLuint height)
{
    AGE_ASSERT(width > 0);
    AGE_ASSERT(height > 0);

    bool result = true;

    if ((width != m_width) || (height != m_height))
    {
        m_width = width;
        m_height = height;

        bind();
        clear();

        GLenum error = m_opengl_3.glGetError();
        result = error == GL_NO_ERROR;
    }

    return result;
}

void age::qt_gl_renderer::texture::clear()
{
    pixel p;
    pixel_vector pv(m_width * m_height, p);

    update(pv);
}
