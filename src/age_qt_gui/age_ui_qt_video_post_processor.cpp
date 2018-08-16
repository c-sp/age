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
{
    LOG("");
    initializeOpenGLFunctions();

    set_native_frame_size(m_native_frame_size);
    AGE_ASSERT(m_frames.size() == qt_video_frame_history_size);
}

age::qt_video_post_processor::~qt_video_post_processor()
{
    m_frames.clear();
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
    m_frames.clear();

    for (size_t i = 0; i < qt_video_frame_history_size; ++i)
    {
        video_frame frame;
        frame.m_native = QSharedPointer<QOpenGLTexture>::create(QOpenGLTexture::Target2D);
        frame.m_native->setFormat(tx_format);
        frame.m_native->setSize(m_native_frame_size.width(), m_native_frame_size.height());
        frame.m_native->allocateStorage(tx_pixel_format, tx_pixel_type);
        set_min_mag_filter(frame.m_native->textureId());

        m_frames.append(frame);
    }

    set_post_processing_filter(m_post_processing_filter);
}

void age::qt_video_post_processor::set_texture_filter(bool bilinear_filter)
{
    LOG(bilinear_filter);
    m_bilinear_filter = bilinear_filter;

    for (int i = 0; i < m_frames.size(); ++i)
    {
        set_min_mag_filter(m_frames[i].m_native->textureId());
        if (post_process_frames())
        {
            set_min_mag_filter(m_frames[i].m_post_processed->texture());
        }
    }
}

void age::qt_video_post_processor::set_post_processing_filter(const qt_filter_vector &filter)
{
    LOG("#filters: " << filter.size());
    m_post_processing_filter = filter;

    // remove current post-processed frames
    for (int i = 0; i < m_frames.size(); ++i)
    {
        m_frames[i].m_post_processed = nullptr;
    }

    // create new post-processor
    create_post_processor();

    // post-process current frames
    if (post_process_frames())
    {
        for (int i = 0; i < m_frames.size(); ++i)
        {
            post_process_frame(m_frames[i]);
        }
    }
}



void age::qt_video_post_processor::add_new_frame(const pixel_vector &pixel_data)
{
    AGE_ASSERT(m_frames.size() > 0);
    AGE_ASSERT(m_new_frame_idx >= 0);
    AGE_ASSERT(m_new_frame_idx < m_frames.size());

    video_frame &frame = m_frames[m_new_frame_idx];
    frame.m_native->bind();
    frame.m_native->setData(tx_pixel_format, tx_pixel_type, pixel_data.data());

    if (post_process_frames())
    {
        post_process_frame(frame);
    }

    ++m_new_frame_idx;
    m_new_frame_idx = (m_new_frame_idx >= m_frames.size()) ? 0 : m_new_frame_idx;
}



QList<GLuint> age::qt_video_post_processor::get_frame_textures(int last_N_frames) const
{
    AGE_ASSERT(m_frames.size() > 0);
    AGE_ASSERT(m_new_frame_idx >= 0);
    AGE_ASSERT(m_new_frame_idx < m_frames.size());

    last_N_frames = qMax(last_N_frames, 1);
    last_N_frames = qMin(last_N_frames, m_frames.size());

    AGE_ASSERT(last_N_frames > 0);
    AGE_ASSERT(last_N_frames <= m_frames.size());

    QList<GLuint> result;
    int frame_idx = m_new_frame_idx;

    for (int i = 0; i < last_N_frames; ++i)
    {
        frame_idx = ((frame_idx > 0) ? frame_idx : m_frames.size()) - 1;

        result.append(post_process_frames()
                      ? m_frames[frame_idx].m_post_processed->texture()
                      : m_frames[frame_idx].m_native->textureId());
    }

    return result;
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

void age::qt_video_post_processor::set_min_mag_filter(GLuint texture_id)
{
    // This should be the only method for setting min/mag filters on
    // QOpenGLTexture and QOpenGLFramebufferObject instances.
    // However, since QOpenGLFramebufferObject does not expose any methods
    // for setting these filters we use pure OpenGL functions instead.

    auto min_filter = GL_LINEAR; // always use linear filter for rendering downscaled texture
    auto mag_filter = m_bilinear_filter ? GL_LINEAR : GL_NEAREST;

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
}



void age::qt_video_post_processor::create_post_processor()
{
    //! \todo create_post_processor
}



bool age::qt_video_post_processor::post_process_frames() const
{
    //! \todo post_process_frames: better way?
    return m_frames[0].m_post_processed != nullptr;
}



void age::qt_video_post_processor::post_process_frame(video_frame &frame)
{
    AGE_ASSERT(frame.m_post_processed != nullptr);

    //! \todo post-process frame
}
