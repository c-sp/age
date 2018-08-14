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

#include <algorithm> // std::min, std::max, std::copy

#include <QRectF>
#include <QSurfaceFormat>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

#include <age_debug.hpp>

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





//---------------------------------------------------------
//
//   constructor & destructor
//
//---------------------------------------------------------

age::qt_video_output::qt_video_output(QWidget *parent)
    : QOpenGLWidget(parent)
{
    // Which OpenGL Version is being used?
    // https://stackoverflow.com/questions/41021681/qt-how-to-detect-which-version-of-opengl-is-being-used
    LOG("OpenGL Module Type: " << QOpenGLContext::openGLModuleType()
        << " (LibGL " << QOpenGLContext::LibGL << ", LibGLES " << QOpenGLContext::LibGLES << ")");

    LOG("format version: " << format().majorVersion() << "." << format().minorVersion());
    LOG("format options: " << format().options());
}

age::qt_video_output::~qt_video_output()
{
    makeCurrent();

    m_renderer = nullptr;
    m_frame_texture.clear();

    doneCurrent();
}





//---------------------------------------------------------
//
//   public interface
//
//---------------------------------------------------------

age::uint age::qt_video_output::get_fps() const
{
    //! \todo implement age::qt_renderer::get_fps
    return 0;
}



void age::qt_video_output::set_emulator_screen_size(uint w, uint h)
{
    LOG(w << ", " << h);
    m_emulator_screen = QSize(w, h);

    update_if_initialized([this]
    {
        m_renderer->set_matrix(m_emulator_screen, QSize(width(), height()));
        allocate_textures();
    });
}

void age::qt_video_output::new_frame(std::shared_ptr<const age::pixel_vector> new_frame)
{
    new_frame_slot(new_frame);
}



void age::qt_video_output::set_blend_frames(uint num_frames_to_blend)
{
    LOG(num_frames_to_blend);

    num_frames_to_blend = std::max(num_frames_to_blend, static_cast<uint>(1));
    num_frames_to_blend = std::min(num_frames_to_blend, qt_video_frame_history_size);
    m_num_frames_to_blend = num_frames_to_blend;

    update(); // trigger paintGL()
}

void age::qt_video_output::set_filter_chain(qt_filter_vector filter_chain)
{
    LOG("#filters: " << filter_chain.size());
    //! \todo implement age::qt_renderer::set_filter_chain
}

void age::qt_video_output::set_bilinear_filter(bool bilinear_filter)
{
    LOG(bilinear_filter);
    m_bilinear_filter = bilinear_filter;

    update_if_initialized([this]
    {
        set_texture_filter();
    });
}





//---------------------------------------------------------
//
//   OpenGL rendering
//
//---------------------------------------------------------

void age::qt_video_output::initializeGL()
{
    initializeOpenGLFunctions();

    LOG("format version: " << format().majorVersion() << "." << format().minorVersion());
    LOG("GL_VERSION: " << glGetString(GL_VERSION));
    LOG("GL_SHADING_LANGUAGE_VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION));

    // OpenGL configuration
    glClearColor(0, 0, 0, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // create renderer
    m_renderer = std::make_unique<qt_video_renderer>();
    m_renderer->set_matrix(m_emulator_screen, QSize(width(), height()));

    // create textures
    allocate_textures();
}

void age::qt_video_output::resizeGL(int width, int height)
{
    LOG(width << " x " << height);
    m_renderer->set_matrix(m_emulator_screen, QSize(width, height));
}

void age::qt_video_output::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    std::vector<std::shared_ptr<QOpenGLTexture>> textures_to_render;
    std::copy(begin(m_frame_texture), begin(m_frame_texture) + m_num_frames_to_blend, back_inserter(textures_to_render));

    m_renderer->render(textures_to_render);
}



void age::qt_video_output::update_if_initialized(std::function<void()> update_func)
{
    if (nullptr != m_renderer)
    {
        makeCurrent();
        update_func();
        doneCurrent();
        update(); // trigger paintGL()
    }
}





//---------------------------------------------------------
//
//   new frame event handling
//
//---------------------------------------------------------

void age::qt_video_output::new_frame_slot(std::shared_ptr<const pixel_vector> new_frame)
{
    if (new_frame == nullptr)
    {
        return;
    }

    // if multiple frame are queued,
    // we handle only the last one and discard all others
    // (this should not happen if the machine can handle the load)
    if (m_new_frame == nullptr)
    {
        // call process_new_frame() after all events scheduled so far (with the same priority) have been processed
        //  -> this requires an event loop
        QMetaObject::invokeMethod(this, "process_new_frame", Qt::QueuedConnection);
    }
    else
    {
        ++m_frames_discarded;
        LOG(m_frames_discarded << " frame(s) discarded (total)");
    }
    m_new_frame = new_frame;
}

void age::qt_video_output::process_new_frame()
{
    update_if_initialized([this]
    {
        // the oldest frame-texture will store the new frame
        std::shared_ptr<QOpenGLTexture> tmp = m_frame_texture[m_frame_texture.size() - 1];
        tmp->bind();
        tmp->setData(tx_pixel_format, tx_pixel_type, m_new_frame->data());

        // move all other frames so that the new frame can be placed at the front
        for (size_t i = m_frame_texture.size() - 1; i > 0; --i)
        {
            m_frame_texture[i] = m_frame_texture[i - 1];
        }
        m_frame_texture[0] = tmp;
    });

    m_new_frame = nullptr; // allow next frame to be processed
}



void age::qt_video_output::allocate_textures()
{
    LOG("");

    m_frame_texture.clear();
    for (size_t i = 0; i < qt_video_frame_history_size; ++i)
    {
        auto texture = std::make_shared<QOpenGLTexture>(QOpenGLTexture::Target2D);
        texture->setFormat(tx_format);
        texture->setSize(m_emulator_screen.width(), m_emulator_screen.height());
        texture->allocateStorage(tx_pixel_format, tx_pixel_type);

        m_frame_texture.push_back(texture);
    }

    set_texture_filter();
}

void age::qt_video_output::set_texture_filter()
{
    LOG(m_bilinear_filter);

    auto min_filter = QOpenGLTexture::Linear; // always use linear filter for rendering downscaled texture
    auto mag_filter = m_bilinear_filter ? QOpenGLTexture::Linear : QOpenGLTexture::Nearest;

    std::for_each(begin(m_frame_texture), end(m_frame_texture), [&](auto &texture)
    {
        texture->setMinMagFilters(min_filter, mag_filter);
    });
}
