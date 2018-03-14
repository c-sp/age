//
// Copyright (c) 2010-2017 Christoph Sprenger
//
// This file is part of AGE ("Another Gameboy Emulator").
// <https://gitlab.com/csprenger/AGE>
//
// AGE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// AGE is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with AGE.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AGE_UI_QT_GL_RENDERER_HPP
#define AGE_UI_QT_GL_RENDERER_HPP

//!
//! \file
//!

#include <array>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include <QElapsedTimer>
#include <QGLFunctions>
#include <QGLWidget>
#include <QtGui/qopengl.h> // GLint, GLuint
#include <QOpenGLFunctions_3_0>
#include <QOpenGLShaderProgram>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QThread>

#include <age_graphics.hpp>
#include <age_non_copyable.hpp>
#include <age_types.hpp>

#include "age_ui_qt.hpp"



namespace age
{

class qt_gl_renderer : public QGLWidget, private QGLFunctions, private QThread
{
public:

    qt_gl_renderer(QWidget *parent = 0);
    virtual ~qt_gl_renderer() override;

    void start();

    GLint get_max_texture_size() const;
    uint get_fps() const;

    void set_emulator_screen_size(uint width, uint height);
    void add_video_frame(const pixel_vector &new_video_frame);
    void set_blend_video_frames(uint num_video_frames_to_blend);
    void set_filter_chain(const qt_filter_vector &filter_chain);
    void set_bilinear_filter(bool set_bilinear_filter);

protected:

    void glDraw() override;
    void paintEvent(QPaintEvent *eve) override;
    void resizeEvent(QResizeEvent *eve) override;

    void run() override;



private:

    class texture : public non_copyable
    {
    public:

        texture();
        ~texture();

        GLuint get_width() const;
        GLuint get_height() const;
        GLuint get_id() const;
        void bind();

        void update(const pixel_vector &pixel_data);
        void set_bilinear(bool bilinear);
        bool assure_size(GLuint width, GLuint height);
        void clear();

    private:

        void set_filter();

        QOpenGLFunctions_3_0 m_opengl_3;
        GLuint m_width = 0;
        GLuint m_height = 0;
        GLuint m_texture_id = 0;
        GLint m_filter_param = GL_NEAREST;
    };



    class shader_program : public non_copyable
    {
    public:

        shader_program(const char *vertex_shader_source, const char *fragment_shader_source, qt_filter filter);
        virtual ~shader_program();

        uint get_scale_factor() const;

        void prepare(GLfloat texture_width, GLfloat texture_height);

    protected:

        virtual void set_uniforms() {}

        QOpenGLShaderProgram m_shader_program;

    private:

        const uint m_scale_factor;
        int m_uniform_texture_size;
    };

    class shader_program_scale2x : public shader_program
    {
    public:

        shader_program_scale2x();

    protected:

        void set_uniforms() override;

    private:

        int m_uniform_eq_threshold;
    };

    class shader_program_age_scale2x : public shader_program
    {
    public:

        shader_program_age_scale2x();

    protected:

        void set_uniforms() override;

    private:

        int m_uniform_eq_threshold;
    };

    class shader_program_gauss3x3_first_pass : public shader_program
    {
    public:

        shader_program_gauss3x3_first_pass();
    };

    class shader_program_gauss3x3_second_pass : public shader_program
    {
    public:

        shader_program_gauss3x3_second_pass();
    };

    class shader_program_gauss5x5_first_pass : public shader_program
    {
    public:

        shader_program_gauss5x5_first_pass();
    };

    class shader_program_gauss5x5_second_pass : public shader_program
    {
    public:

        shader_program_gauss5x5_second_pass();
    };

    class shader_program_emboss3x3 : public shader_program
    {
    public:

        shader_program_emboss3x3();
    };

    class shader_program_emboss5x5 : public shader_program
    {
    public:

        shader_program_emboss5x5();
    };



    class video_frame_manager : public non_copyable
    {
    public:

        static GLint get_max_texture_size(QOpenGLFunctions_3_0 &m_opengl_3);

        video_frame_manager();
        ~video_frame_manager();

        GLuint get_frame_width() const;
        GLuint get_frame_height() const;

        void set_bilinear(bool bilinear);
        void set_frame_size(GLuint width, GLuint height);
        void set_filter_chain(const qt_filter_vector &filter_chain);
        void add_new_frame(const pixel_vector &pixel_data);
        void bind_frame(uint frame_index); // 0 == most recent frame

    private:

        struct frame
        {
            texture m_base_texture;
            texture m_filtered_texture;
            bool m_filtered;
        };

        typedef std::array<frame, qt_video_frame_history_size> frame_array;

        void apply_shader_chain();
        void assure_size(texture &tex, GLuint width, GLuint height);
        void filter_frame(frame &fr);

        QOpenGLFunctions_3_0 m_opengl_3;
        uint m_max_texture_size;
        shader_program_scale2x m_shader_scale2x;
        shader_program_age_scale2x m_shader_age_scale2x;
        shader_program_gauss3x3_first_pass m_shader_gauss3x3_first_pass;
        shader_program_gauss3x3_second_pass m_shader_gauss3x3_second_pass;
        shader_program_gauss5x5_first_pass m_shader_gauss5x5_first_pass;
        shader_program_gauss5x5_second_pass m_shader_gauss5x5_second_pass;
        shader_program_emboss3x3 m_shader_emboss3x3;
        shader_program_emboss5x5 m_shader_emboss5x5;

        std::vector<shader_program*> m_shader_chain;
        std::vector<texture> m_shader_textures;
        GLuint m_frame_buffer_id = 0;
        uint m_scale_factor = 0;

        bool m_bilinear = false;
        GLuint m_frame_width = 1;
        GLuint m_frame_height = 1;
        uint m_current_frame = 0;
        frame_array m_frames;
    };




    struct events
    {
        uint m_flags = 0;
        std::shared_ptr<const pixel_vector> m_new_video_frame;
        qt_filter_vector m_filter_chain;
        bool m_bilinear = false;
        GLsizei m_viewport_width = 1;
        GLsizei m_viewport_height = 1;
        GLuint m_screen_width = 1;
        GLuint m_screen_height = 1;
        uint m_video_frames_to_blend = 0;

        bool is_flagged(uint flag)
        {
            return (m_flags & flag) > 0;
        }
    };



    void render(video_frame_manager &frame_manager);
    void trigger_rendering();
    template<typename... ARGS>
    void locked_flag_operation(std::function<void(ARGS...)> func, uint flag, ARGS... args);
    void process_events(events &events, video_frame_manager &frame_manager);



    // set within constructor
    QOpenGLFunctions_3_0 m_opengl_3;
    QElapsedTimer m_timer;
    GLint m_max_texture_size = 0;

    // accessed using the event mutex
    std::mutex m_event_mutex; // used to communicate render options and new video frames to the render thread
    std::condition_variable m_event_wait;
    events m_events;

    // used only within the render thread
    uint m_current_video_frame = 0;
    uint m_num_video_frames_to_blend = 1;

    // fps data
    mutable atomic_uint m_fps = {0};
    mutable atomic_uint m_frame_counter = {0};
    mutable atomic_uint m_last_fps_millis = {0};
};

} // namespace age



#endif // AGE_UI_QT_GL_RENDERER_HPP
