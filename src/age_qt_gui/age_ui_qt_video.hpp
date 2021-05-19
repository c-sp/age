//
// Copyright 2020 Christoph Sprenger
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

#include <functional> // std::function

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

    void qt_init_shader_program(QOpenGLShaderProgram& program, const QString& vertex_shader_file, const QString& fragment_shader_file);
    void qt_use_float_attribute_buffer(QOpenGLShaderProgram& program, const char* attribute_name, int offset, int tuple_size, int stride = 0);



    class qt_video_renderer : private QOpenGLFunctions
    {
    public:
        qt_video_renderer();
        ~qt_video_renderer();

        void update_matrix(const QSize& emulator_screen, const QSize& viewport);
        void render(const QList<GLuint>& textures_to_render);

    private:
        QOpenGLShaderProgram m_program;
        QOpenGLBuffer        m_vertices;
        QOpenGLBuffer        m_indices;
        QMatrix4x4           m_projection;
    };



    class qt_video_post_processor : private QOpenGLFunctions
    {
    public:
        qt_video_post_processor();
        ~qt_video_post_processor();

        void set_native_frame_size(const QSize& size);
        void set_texture_filter(bool bilinear_filter);
        void set_post_processing_filter(const qt_filter_list& filter_list);

        void add_new_frame(const pixel_vector& frame);

        [[nodiscard]] QList<GLuint> get_frame_textures(int last_N_frames) const;

    private:
        struct processing_step
        {
            QOpenGLShaderProgram*                    m_program = nullptr;
            QSharedPointer<QOpenGLFramebufferObject> m_buffer;
        };

        void set_min_mag_filter(GLuint texture_id, bool min_linear, bool mag_linear);
        void set_wrap_mode(GLuint texture_id);

        [[nodiscard]] bool post_process_frames() const;
        void               post_process_frame(int frame_idx);

        void                                            create_post_processor();
        QList<QSharedPointer<QOpenGLFramebufferObject>> create_frame_buffers(int buffers_to_create, const QSize& buffer_size);
        bool                                            add_step(QList<processing_step>& post_processor, QOpenGLShaderProgram* program, const QSize& result_frame_size);

        QSize          m_native_frame_size{1, 1};
        bool           m_bilinear_filter = false;
        qt_filter_list m_filter_list;

        int                                   m_new_frame_idx = 0;
        QList<QSharedPointer<QOpenGLTexture>> m_native_frames;

        QList<QSharedPointer<QOpenGLFramebufferObject>> m_processed_frames;
        QList<processing_step>                          m_post_processor;
        QOpenGLShaderProgram                            m_program_scale2x;
        QOpenGLShaderProgram                            m_program_scale2x_age;
        QOpenGLShaderProgram                            m_program_gauss3x3_s;
        QOpenGLShaderProgram                            m_program_gauss3x3_t;
        QOpenGLShaderProgram                            m_program_gauss5x5_s;
        QOpenGLShaderProgram                            m_program_gauss5x5_t;
        QOpenGLShaderProgram                            m_program_emboss3x3;
        QOpenGLShaderProgram                            m_program_emboss5x5;
        QOpenGLBuffer                                   m_vertices;
        QOpenGLBuffer                                   m_indices;
    };



    class qt_video_output : public QOpenGLWidget, protected QOpenGLFunctions
    {
        Q_OBJECT

        // public interface

    public:
        explicit qt_video_output(QWidget* parent = nullptr);
        ~qt_video_output() override;

    signals:

        void fps(int fps);

    public slots:

        void set_emulator_screen_size(age::int16_t width, age::int16_t height);
        void new_frame(QSharedPointer<const age::pixel_vector> new_frame);

        void set_blend_frames(int num_frames_to_blend);
        void set_post_processing_filter(age::qt_filter_list filter_list);
        void set_bilinear_filter(bool bilinear_filter);


        // OpenGL rendering

    protected:
        void initializeGL() override;
        void resizeGL(int width, int height) override;
        void paintGL() override;

    private:
        void run_if_initialized(std::function<void()> function_to_run);

        QSharedPointer<qt_video_renderer>       m_renderer       = nullptr;
        QSharedPointer<qt_video_post_processor> m_post_processor = nullptr;

        QSize          m_emulator_screen     = {1, 1};
        int            m_num_frames_to_blend = 1;
        bool           m_bilinear_filter     = false;
        qt_filter_list m_filter_list;


        // frame event handling

    private slots:

        void update_fps();

    private:
        void             new_frame_slot(QSharedPointer<const pixel_vector> new_frame);
        Q_INVOKABLE void process_new_frame();

        QSharedPointer<const pixel_vector> m_new_frame        = nullptr;
        int                                m_frames_discarded = 0;
        int                                m_frame_counter    = 0;
    };

} // namespace age



#endif // AGE_UI_QT_RENDERER_HPP
