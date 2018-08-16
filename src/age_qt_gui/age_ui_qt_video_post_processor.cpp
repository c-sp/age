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

// we don't really use the alpha channel but OpenGL ES would not work without it
constexpr QOpenGLTexture::TextureFormat tx_format = QOpenGLTexture::RGBAFormat;
constexpr QOpenGLTexture::PixelFormat tx_pixel_format = QOpenGLTexture::RGBA;
constexpr QOpenGLTexture::PixelType tx_pixel_type = QOpenGLTexture::UInt8;





age::qt_video_post_processor::~qt_video_post_processor()
{
    m_frame_textures.clear();
    LOG("");
}



void age::qt_video_post_processor::set_base_texture_size(const QSize &size)
{
    LOG(size.width() << " x " << size.height());

    m_frame_textures.clear();
    for (size_t i = 0; i < qt_video_frame_history_size; ++i)
    {
        auto texture = QSharedPointer<QOpenGLTexture>::create(QOpenGLTexture::Target2D);
        texture->setFormat(tx_format);
        texture->setSize(size.width(), size.height());
        texture->allocateStorage(tx_pixel_format, tx_pixel_type);

        m_frame_textures.append(texture);
    }
}

void age::qt_video_post_processor::set_texture_filter(bool bilinear_filter)
{
    LOG(bilinear_filter);

    auto min_filter = QOpenGLTexture::Linear; // always use linear filter for rendering downscaled texture
    auto mag_filter = bilinear_filter ? QOpenGLTexture::Linear : QOpenGLTexture::Nearest;

    for (int i = 0; i < m_frame_textures.size(); ++i)
    {
        m_frame_textures[i]->setMinMagFilters(min_filter, mag_filter);
    }
}

void age::qt_video_post_processor::set_post_processing_filter(const qt_filter_vector &filter)
{
    LOG("#filters: " << filter.size());
    //! \todo implement age::qt_video_post_processor::set_post_processing_filter
}



void age::qt_video_post_processor::new_frame(const pixel_vector &frame)
{
    // the oldest frame-texture will store the new frame
    QSharedPointer<QOpenGLTexture> tmp = m_frame_textures[m_frame_textures.size() - 1];
    tmp->bind();
    tmp->setData(tx_pixel_format, tx_pixel_type, frame.data());

    // move all other frames so that the new frame can be placed at the front
    for (int i = m_frame_textures.size() - 1; i > 0; --i)
    {
        m_frame_textures[i] = m_frame_textures[i - 1];
    }
    m_frame_textures[0] = tmp;
}



QList<GLuint> age::qt_video_post_processor::get_last_frames(int num_frames) const
{
    num_frames = qMax(num_frames, 1);
    num_frames = qMin(num_frames, m_frame_textures.size());

    QList<GLuint> result;
    for (int i = 0; i < num_frames; ++i)
    {
        result.append(m_frame_textures[i]->textureId());
    }

    return result;
}
