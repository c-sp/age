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

#include <QList>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QSharedPointer>
#include <QSize>
#include <QString>
#include <QVector2D>
#include <QVector3D>

#include <age_types.hpp>
#include <gfx/age_pixel.hpp>

#include "age_ui_qt.hpp"



namespace age
{

struct qt_vertex_data
{
    QVector3D position;
    QVector2D texCoord;
};

void qt_init_shader_program(QOpenGLShaderProgram &program, const QString &vertex_shader_file, const QString &fragment_shader_file);



class qt_video_renderer : private QOpenGLFunctions
{
public:

    qt_video_renderer();
    ~qt_video_renderer();

    void update_matrix(const QSize &emulator_screen, const QSize &viewport);
    void render(const QList<GLuint> &textures_to_render);

private:

    QOpenGLShaderProgram m_program;
    QOpenGLBuffer m_vertices;
    QOpenGLBuffer m_indices;
    QMatrix4x4 m_projection;
};



class qt_video_post_processor : private QOpenGLFunctions
{
public:

    qt_video_post_processor();
    ~qt_video_post_processor();

    void set_native_frame_size(const QSize &size);
    void set_texture_filter(bool bilinear_filter);
    void set_post_processing_filter(const qt_filter_vector &filter);

    void add_new_frame(const pixel_vector &frame);

    QList<GLuint> get_frame_textures(int last_N_frames) const;

private:

    struct processing_step
    {
        QOpenGLShaderProgram *m_program = nullptr;
        QSharedPointer<QOpenGLFramebufferObject> m_buffer;
    };

    void set_min_mag_filter(GLuint texture_id, bool bilinear);
    void create_post_processor();
    bool post_process_frames() const;
    void post_process_frame(int frame_idx);

    static QList<QSharedPointer<QOpenGLFramebufferObject>> create_frame_buffers(int buffers_to_create, const QSize &buffer_size);
    static bool add_step(QList<processing_step> &post_processor, QOpenGLShaderProgram *program, const QSize &result_frame_size);

    QSize m_native_frame_size = {1, 1};
    bool m_bilinear_filter = false;
    qt_filter_vector m_post_processing_filter;

    int m_new_frame_idx = 0;
    QList<QSharedPointer<QOpenGLTexture>> m_native_frames;

    QList<QSharedPointer<QOpenGLFramebufferObject>> m_processed_frames;
    QList<processing_step> m_post_processor;
    QOpenGLShaderProgram m_program_emboss3x3;
    QOpenGLBuffer m_vertices;
    QOpenGLBuffer m_indices;
};



class qt_video_output : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

    // public interface

public:

    qt_video_output(QWidget *parent = nullptr);
    ~qt_video_output() override;

    uint get_fps() const;

public slots:

    void set_emulator_screen_size(uint width, uint height);
    void new_frame(std::shared_ptr<const age::pixel_vector> new_frame);

    void set_blend_frames(uint num_frames_to_blend);
    void set_post_processing_filter(qt_filter_vector filter);
    void set_bilinear_filter(bool bilinear_filter);


    // OpenGL rendering

protected:

    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

private:

    void run_if_initialized(std::function<void()> function_to_run);

    std::unique_ptr<qt_video_renderer> m_renderer = nullptr;
    std::unique_ptr<qt_video_post_processor> m_post_processor = nullptr;

    QSize m_emulator_screen = {1, 1};
    uint m_num_frames_to_blend = 1;
    bool m_bilinear_filter = false;
    qt_filter_vector m_post_processing_filter;


    // frame event handling

private:

    void new_frame_slot(std::shared_ptr<const pixel_vector> new_frame);
    Q_INVOKABLE void process_new_frame();

    std::shared_ptr<const pixel_vector> m_new_frame = nullptr;
    uint m_frames_discarded = 0;
};

} // namespace age



#endif // AGE_UI_QT_RENDERER_HPP
