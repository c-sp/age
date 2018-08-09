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

#include <QRectF>

#include <age_debug.hpp>

#include "age_ui_qt_renderer.hpp"

#if 1
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif



//---------------------------------------------------------
//
//   constructor & destructor
//
//---------------------------------------------------------

age::qt_renderer::qt_renderer(QWidget *parent)
    : QOpenGLWidget(parent)
{
}



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

age::uint age::qt_renderer::get_fps() const
{
    //! \todo implement age::qt_renderer::get_fps
    return 0;
}



//---------------------------------------------------------
//
//   public slots
//
//---------------------------------------------------------

void age::qt_renderer::set_emulator_screen_size(uint width, uint height)
{
    LOG(width << ", " << height);
    m_emulator_screen = QSize(width, height);

    update_projection(); // calculate new projection matrix
    update(); // trigger paintGL()
}

void age::qt_renderer::new_video_frame(pixel_vector new_video_frame)
{
    LOG("#pixel: " << new_video_frame.size());
    //! \todo implement age::qt_renderer::new_video_frame
}



void age::qt_renderer::set_blend_video_frames(uint num_video_frames_to_blend)
{
    LOG(num_video_frames_to_blend);
    //! \todo implement age::qt_renderer::set_blend_video_frames
}

void age::qt_renderer::set_filter_chain(qt_filter_vector filter_chain)
{
    LOG("#filters: " << filter_chain.size());
    //! \todo implement age::qt_renderer::set_filter_chain
}

void age::qt_renderer::set_bilinear_filter(bool set_bilinear_filter)
{
    LOG(set_bilinear_filter);
    //! \todo implement age::qt_renderer::set_bilinear_filter
}



//---------------------------------------------------------
//
//   protected methods
//
//---------------------------------------------------------

void age::qt_renderer::initializeGL()
{
    initializeOpenGLFunctions();

    // clear color
    glClearColor(0, 0, 0, 1);
}



void age::qt_renderer::resizeGL(int width, int height)
{
    LOG("viewport size: " << width << " x " << height);
    m_current_viewport = QSize(width, height);

    update_projection();
}



void age::qt_renderer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

void age::qt_renderer::update_projection()
{
    double viewport_ratio = 1. * m_current_viewport.width() / m_current_viewport.height();
    double screen_ratio = 1. * m_emulator_screen.width() / m_emulator_screen.height();

    QRectF proj;
    if (viewport_ratio > screen_ratio)
    {
        double diff = viewport_ratio - screen_ratio;
        AGE_ASSERT(diff > 0);
        proj = QRectF(-.5 * diff, 0, 1 + diff, 1); // x, y, width, height
    }
    else
    {
        double diff = (1 / viewport_ratio) - (1 / screen_ratio);
        AGE_ASSERT(diff > 0);
        proj = QRectF(0, -.5 * diff, 1, 1 + diff); // x, y, width, height
    }

    m_projection.setToIdentity();
    m_projection.ortho(proj);
}
