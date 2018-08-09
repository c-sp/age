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
}

void age::qt_renderer::new_video_frame(pixel_vector new_video_frame)
{
    LOG("#pixel: " << new_video_frame.size());
}



void age::qt_renderer::set_blend_video_frames(uint num_video_frames_to_blend)
{
    LOG(num_video_frames_to_blend);
}

void age::qt_renderer::set_filter_chain(qt_filter_vector filter_chain)
{
    LOG("#filters: " << filter_chain.size());
}

void age::qt_renderer::set_bilinear_filter(bool set_bilinear_filter)
{
    LOG(set_bilinear_filter);
}



//---------------------------------------------------------
//
//   protected methods
//
//---------------------------------------------------------

void age::qt_renderer::initializeGL()
{
    initializeOpenGLFunctions();
    LOG("OpenGL functions initialized");
}

void age::qt_renderer::resizeGL(int w, int h)
{
    LOG(w << ", " << h);
}

void age::qt_renderer::paintGL()
{
    LOG("");
    glClear(GL_COLOR_BUFFER_BIT);
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------
