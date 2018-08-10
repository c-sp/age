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
#ifndef AGE_UI_QT_RENDERER_HPP
#define AGE_UI_QT_RENDERER_HPP

//!
//! \file
//!

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QSize>

#include <age_types.hpp>
#include <gfx/age_pixel.hpp>

#include "age_ui_qt.hpp"



namespace age
{

class qt_renderer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:

    qt_renderer(QWidget *parent = nullptr);
    ~qt_renderer() override;

    uint get_fps() const;

public slots:

    void set_emulator_screen_size(uint width, uint height);
    void new_video_frame(pixel_vector new_video_frame);

    void set_blend_video_frames(uint num_video_frames_to_blend);
    void set_filter_chain(qt_filter_vector filter_chain);
    void set_bilinear_filter(bool set_bilinear_filter);

protected:

    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

private:

    void update_projection();

    QSize m_emulator_screen = {1, 1};
    QSize m_current_viewport = {1, 1};
    QMatrix4x4 m_projection;

    QOpenGLShaderProgram m_program;
    QOpenGLBuffer m_vertices;
    QOpenGLBuffer m_indices;
};

} // namespace age



#endif // AGE_UI_QT_RENDERER_HPP
