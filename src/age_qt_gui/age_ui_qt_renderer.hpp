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

#include <memory> // std::shared_ptr

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
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

    // public interface

public:

    qt_renderer(QWidget *parent = nullptr);
    ~qt_renderer() override;

    uint get_fps() const;

public slots:

    void set_emulator_screen_size(uint width, uint height);
    void new_frame(std::shared_ptr<const age::pixel_vector> new_frame);

    void set_blend_frames(uint num_frames_to_blend);
    void set_filter_chain(qt_filter_vector filter_chain);
    void set_bilinear_filter(bool set_bilinear_filter);



    // OpenGL rendering

protected:

    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

private:

    void update_projection_matrix();
    void allocate_textures();

    QSize m_emulator_screen = {1, 1};
    QSize m_current_viewport = {1, 1};
    QMatrix4x4 m_projection;

    QOpenGLShaderProgram m_program;
    QOpenGLBuffer m_vertices;
    QOpenGLBuffer m_indices;

    std::unique_ptr<QOpenGLTexture> m_last_frame_texture = nullptr;



    // new frame event handling

private:

    void new_frame_slot(std::shared_ptr<const pixel_vector> new_frame);
    Q_INVOKABLE void process_new_frame();

    std::shared_ptr<const pixel_vector> m_new_frame = nullptr;
    uint m_frames_discarded = 0;
};

} // namespace age



#endif // AGE_UI_QT_RENDERER_HPP
