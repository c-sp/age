//
// Copyright 2018 Christoph Sprenger
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "age_ui_qt_video.hpp"

#if 1
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif

namespace {

// we don't really use the alpha channel but OpenGL ES would not work without it
constexpr QOpenGLTexture::TextureFormat tx_format = QOpenGLTexture::RGBAFormat;
constexpr QOpenGLTexture::PixelFormat tx_pixel_format = QOpenGLTexture::RGBA;
constexpr QOpenGLTexture::PixelType tx_pixel_type = QOpenGLTexture::UInt8;

}



//---------------------------------------------------------
//
//   constructor & destructor
//
//---------------------------------------------------------

age::qt_video_post_processor::qt_video_post_processor()
    : m_indices(QOpenGLBuffer::IndexBuffer)
{
    LOG("");
    initializeOpenGLFunctions();

    set_native_frame_size(m_native_frame_size);
    AGE_ASSERT(m_native_frames.size() == qt_video_frame_history_size);

    qt_init_shader_program(m_program_emboss3x3, ":/age_ui_qt_emboss3x3_vsh.glsl", ":/age_ui_qt_emboss3x3_fsh.glsl");

    // vertex buffer
    QVector3D vertices[] = {
        QVector3D(0, 0, 0),
        QVector3D(0, 1, 0),
        QVector3D(1, 0, 0),
        QVector3D(1, 1, 0),
    };

    m_vertices.create();
    m_vertices.bind();
    m_vertices.allocate(vertices, 4 * sizeof(QVector3D));

    // index buffer
    GLushort indices[] = {0, 1, 2, 3};

    m_indices.create();
    m_indices.bind();
    m_indices.allocate(indices, 4 * sizeof(GLushort));
}

age::qt_video_post_processor::~qt_video_post_processor()
{
    m_indices.destroy();
    m_vertices.destroy();
    LOG("");
}



//---------------------------------------------------------
//
//   public interface
//
//---------------------------------------------------------

void age::qt_video_post_processor::set_native_frame_size(const QSize &size)
{
    LOG(size.width() << " x " << size.height());
    m_native_frame_size = size;

    m_new_frame_idx = 0;
    m_native_frames.clear();

    for (size_t i = 0; i < qt_video_frame_history_size; ++i)
    {
        QSharedPointer<QOpenGLTexture> frame = QSharedPointer<QOpenGLTexture>::create(QOpenGLTexture::Target2D);
        frame->setFormat(tx_format);
        frame->setSize(m_native_frame_size.width(), m_native_frame_size.height());
        frame->allocateStorage(tx_pixel_format, tx_pixel_type);
        set_min_mag_filter(frame->textureId(), m_bilinear_filter);

        m_native_frames.append(frame);
    }

    set_post_processing_filter(m_post_processing_filter);
}

void age::qt_video_post_processor::set_texture_filter(bool bilinear_filter)
{
    LOG(bilinear_filter);
    m_bilinear_filter = bilinear_filter;

    // set min/mag filter for native frames
    for (int i = 0; i < m_native_frames.size(); ++i)
    {
        set_min_mag_filter(m_native_frames[i]->textureId(), m_bilinear_filter);
    }

    // set min/mag filter for post-processed frames
    for (int i = 0; i < m_processed_frames.size(); ++i)
    {
        set_min_mag_filter(m_processed_frames[i]->texture(), m_bilinear_filter);
    }
}

void age::qt_video_post_processor::set_post_processing_filter(const qt_filter_vector &filter)
{
    LOG("#filters: " << filter.size());
    m_post_processing_filter = filter;

    create_post_processor();
}



void age::qt_video_post_processor::add_new_frame(const pixel_vector &pixel_data)
{
    AGE_ASSERT(m_native_frames.size() > 0);
    AGE_ASSERT(m_new_frame_idx >= 0);
    AGE_ASSERT(m_new_frame_idx < m_native_frames.size());

    m_native_frames[m_new_frame_idx]->bind();
    m_native_frames[m_new_frame_idx]->setData(tx_pixel_format, tx_pixel_type, pixel_data.data());

    if (post_process_frames())
    {
        post_process_frame(m_new_frame_idx);
    }

    ++m_new_frame_idx;
    m_new_frame_idx = (m_new_frame_idx >= m_native_frames.size()) ? 0 : m_new_frame_idx;
}



QList<GLuint> age::qt_video_post_processor::get_frame_textures(int last_N_frames) const
{
    last_N_frames = qMax(last_N_frames, 1);
    last_N_frames = qMin(last_N_frames, m_native_frames.size());
    AGE_ASSERT(last_N_frames > 0);
    AGE_ASSERT(last_N_frames <= m_native_frames.size());

    QList<GLuint> result;
    int frame_idx = m_new_frame_idx;

    for (int i = 0; i < last_N_frames; ++i)
    {
        frame_idx = ((frame_idx > 0) ? frame_idx : m_native_frames.size()) - 1;

        result.append(post_process_frames()
                      ? m_processed_frames[frame_idx]->texture()
                      : m_native_frames[frame_idx]->textureId());
    }

    return result;
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

void age::qt_video_post_processor::set_min_mag_filter(GLuint texture_id, bool bilinear)
{
    // This should be the only method for setting min/mag filters on
    // QOpenGLTexture and QOpenGLFramebufferObject instances.
    // However, since QOpenGLFramebufferObject does not expose any methods
    // for setting these filters we use pure OpenGL functions instead.

    auto min_filter = GL_LINEAR; // always use linear filter for rendering downscaled texture
    auto mag_filter = bilinear ? GL_LINEAR : GL_NEAREST;

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
}



void age::qt_video_post_processor::create_post_processor()
{
    // cleanup current post-processing data
    m_processed_frames.clear();
    m_post_processor.clear();
    AGE_ASSERT(!post_process_frames());

    // create a new post-processor
    // (disable post-processing if that fails)
    QList<processing_step> post_processor;
    QSize result_frame_size = m_native_frame_size;

    for (size_t i = 0; i < m_post_processing_filter.size(); ++i)
    {
        qt_filter filter = m_post_processing_filter[i];

        bool step_added = false;

        switch (filter)
        {
            case qt_filter::emboss3x3:
                result_frame_size *= get_qt_filter_factor(filter); //! \todo move outside of switch
                step_added = add_step(post_processor, &m_program_emboss3x3, result_frame_size);
                LOG("added emboss3x3: " << step_added);
                break;

            default: //! \todo remove
                step_added = true;
                break;
        }

        if (!step_added)
        {
            post_processor.clear(); // indicate failure
            break;
        }
    }

    // create a new frame buffer object for every frame
    // (disable post-processing if that fails)
    QList<QSharedPointer<QOpenGLFramebufferObject>> processed_frames;

    if (!post_processor.empty())
    {
        post_processor.last().m_buffer = nullptr; // the last buffer is created for every frame
        LOG("creating " << m_native_frames.size() << " frame buffer objects (" << result_frame_size.width() << " x " << result_frame_size.height() << ")");

        processed_frames = create_frame_buffers(m_native_frames.size(), result_frame_size);
        AGE_ASSERT(processed_frames.size() == m_native_frames.size());
    }

    // post-process current frames if creating the post processor was successful
    if (!post_processor.empty() && !processed_frames.empty())
    {
        LOG("activating post-processor, processing current frames");

        m_post_processor = post_processor;
        m_processed_frames = processed_frames;
        AGE_ASSERT(post_process_frames());

        for (int i = 0; i < m_native_frames.size(); ++i)
        {
            post_process_frame(i);
        }
    }
}



bool age::qt_video_post_processor::post_process_frames() const
{
    AGE_ASSERT(m_post_processor.empty() == m_processed_frames.empty());
    return !m_post_processor.empty();
}



void age::qt_video_post_processor::post_process_frame(int frame_idx)
{
    AGE_ASSERT(!m_post_processor.empty());
    AGE_ASSERT(m_native_frames.size() == m_processed_frames.size());
    AGE_ASSERT(frame_idx >= 0);
    AGE_ASSERT(frame_idx < m_native_frames.size());

    m_post_processor.last().m_buffer = m_processed_frames[frame_idx];
    set_min_mag_filter(m_native_frames[frame_idx]->textureId(), false);
    set_min_mag_filter(m_processed_frames[frame_idx]->texture(), false);

    GLuint texture_id = m_native_frames[frame_idx]->textureId();
    QSize texture_size = m_native_frame_size;

    QMatrix4x4 projection;
    projection.ortho(QRectF(0, 0, 1, 1));

    for (int i = 0; i < m_post_processor.size(); ++i)
    {
        processing_step &step = m_post_processor[i];
        step.m_buffer->bind(); // render to this buffer

        step.m_program->bind();
        step.m_program->setUniformValue("u_projection", projection);
        step.m_program->setUniformValue("u_texture_size", QVector2D(texture_size.width(), texture_size.height()));

        m_vertices.bind();
        m_indices.bind();

        int vertexLocation = step.m_program->attributeLocation("a_vertex");
        step.m_program->enableAttributeArray(vertexLocation);
        step.m_program->setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(QVector3D));

        glBindTexture(GL_TEXTURE_2D, texture_id);
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, nullptr);

        step.m_buffer->release();

        texture_id = step.m_buffer->texture();
        texture_size = step.m_buffer->size();
    }

    m_post_processor.last().m_buffer = nullptr;
    set_min_mag_filter(m_native_frames[frame_idx]->textureId(), m_bilinear_filter);
    set_min_mag_filter(m_processed_frames[frame_idx]->texture(), m_bilinear_filter);
}



QList<QSharedPointer<QOpenGLFramebufferObject>> age::qt_video_post_processor::create_frame_buffers(int buffers_to_create, const QSize &buffer_size)
{
    QList<QSharedPointer<QOpenGLFramebufferObject>> frame_buffers;

    for (int i = 0; i < buffers_to_create; ++i)
    {
        QSharedPointer<QOpenGLFramebufferObject> buffer = QSharedPointer<QOpenGLFramebufferObject>::create(buffer_size);
        if (!buffer->isValid())
        {
            break;
        }
        frame_buffers.append(buffer);
    }

    return (frame_buffers.size() == buffers_to_create) ? frame_buffers : QList<QSharedPointer<QOpenGLFramebufferObject>>();
}

bool age::qt_video_post_processor::add_step(QList<processing_step> &post_processor, QOpenGLShaderProgram *program, const QSize &result_frame_size)
{
    QSharedPointer<QOpenGLFramebufferObject> buffer = QSharedPointer<QOpenGLFramebufferObject>::create(result_frame_size);
    if (!buffer->isValid())
    {
        return false;
    }

    processing_step step;
    step.m_program = program;
    step.m_buffer = buffer;

    post_processor.append(step);
    return true;
}
