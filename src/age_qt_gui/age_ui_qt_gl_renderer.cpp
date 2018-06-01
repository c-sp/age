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

#include <limits>

#include <QSize>

#include <age_debug.hpp>

#include "age_ui_qt_gl_renderer.hpp"

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif

constexpr qint64 qt_fps_update_millis = 500;

constexpr uint qt_renderer_stopped = 1;
constexpr uint qt_renderer_new_video_frame = 2;
constexpr uint qt_renderer_change_filter_chain = 4;
constexpr uint qt_renderer_change_bilinear = 8;
constexpr uint qt_renderer_change_viewport = 16;
constexpr uint qt_renderer_change_emulator_screen_size = 32;
constexpr uint qt_renderer_change_frames_to_blend = 64;



//---------------------------------------------------------
//
//   constructor & destructor
//
//---------------------------------------------------------

age::qt_gl_renderer::qt_gl_renderer(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::DoubleBuffer), parent)
{
    LOG("opengl version flags: " << QGLFormat::openGLVersionFlags());

    LOG("deactivating automatic buffer swapping");
    setAutoBufferSwap(false); // we swap manually

    makeCurrent();
    m_opengl_3.initializeOpenGLFunctions();
    m_max_texture_size = video_frame_manager::get_max_texture_size(m_opengl_3);
    doneCurrent();
    LOG("max texture size is " << m_max_texture_size);

    m_timer.start();
}

age::qt_gl_renderer::~qt_gl_renderer()
{
    LOG("waiting for render thread to finish");
    std::function<void(void)> tmp = [this] () {};
    locked_flag_operation(tmp, qt_renderer_stopped);
    wait();
}



void age::qt_gl_renderer::start()
{
    LOG("moving rendering context to render thread");
    doneCurrent();
    context()->moveToThread(this);

    LOG("starting render thread");
    QThread::start();
}



//---------------------------------------------------------
//
//   public const
//
//---------------------------------------------------------

GLint age::qt_gl_renderer::get_max_texture_size() const
{
    return m_max_texture_size;
}

age::uint age::qt_gl_renderer::get_fps() const
{
    // calculate new FPS, if necessary
    for (;;)
    {
        uint millis = m_timer.elapsed();
        uint last_millis = m_last_fps_millis;
        uint duration_millis = millis - last_millis;

        // nothing to do, if the last FPS calculation wasn't too long ago
        if (duration_millis < qt_fps_update_millis)
        {
            break;
        }

        // calculate current FPS, if this thread wins the race for m_last_fps_millis
        // (otherwise we simply check on duration_millis again)
        if (m_last_fps_millis.compare_exchange_strong(last_millis, millis))
        {
            uint frame_count = m_frame_counter.exchange(0);
            uint fps = frame_count * 1000 / duration_millis;
            m_fps = fps;
        }
    }

    // return current FPS
    return m_fps;
}



//---------------------------------------------------------
//
//   public non-const
//
//---------------------------------------------------------

void age::qt_gl_renderer::set_emulator_screen_size(uint width, uint height)
{
    AGE_ASSERT(width > 0);
    AGE_ASSERT(height > 0);
    AGE_ASSERT(width <= std::numeric_limits<GLuint>::max());
    AGE_ASSERT(height <= std::numeric_limits<GLuint>::max());

    std::function<void(uint, uint)> tmp = [this] (uint width, uint height)
    {
        m_events.m_screen_width = static_cast<GLuint>(width);
        m_events.m_screen_height = static_cast<GLuint>(height);
    };
    locked_flag_operation<uint, uint>(tmp, qt_renderer_change_emulator_screen_size, width, height);
}

void age::qt_gl_renderer::set_blend_video_frames(uint num_frames_to_blend)
{
    AGE_ASSERT(num_frames_to_blend > 0);
    AGE_ASSERT(num_frames_to_blend <= qt_video_frame_history_size);

    std::function<void(uint)> tmp = [this] (uint num_frames_to_blend)
    {
        m_events.m_video_frames_to_blend = num_frames_to_blend;
    };
    locked_flag_operation<uint>(tmp, qt_renderer_change_frames_to_blend, num_frames_to_blend);
}

void age::qt_gl_renderer::set_filter_chain(const qt_filter_vector &filter_chain)
{
    std::function<void(const qt_filter_vector&)> tmp = [this] (const qt_filter_vector &filter_chain)
    {
        m_events.m_filter_chain = filter_chain; // deep copy
    };
    locked_flag_operation<const qt_filter_vector&>(tmp, qt_renderer_change_filter_chain, filter_chain);
}

void age::qt_gl_renderer::set_bilinear_filter(bool use_bilinear_filter)
{
    std::function<void(bool)> tmp = [this] (bool use_bilinear_filter)
    {
        m_events.m_bilinear = use_bilinear_filter;
    };
    locked_flag_operation<bool>(tmp, qt_renderer_change_bilinear, use_bilinear_filter);
}

void age::qt_gl_renderer::add_video_frame(const pixel_vector &new_video_frame)
{
    std::shared_ptr<const pixel_vector> new_pixel_vector { new pixel_vector(new_video_frame) }; // deep copy

    std::function<void(std::shared_ptr<const pixel_vector>)> tmp = [this] (std::shared_ptr<const pixel_vector> new_pixel_vector)
    {
        m_events.m_new_video_frame = new_pixel_vector;
    };
    locked_flag_operation<std::shared_ptr<const pixel_vector>>(tmp, qt_renderer_new_video_frame, new_pixel_vector);
}



//---------------------------------------------------------
//
//   qt events
//
//---------------------------------------------------------

void age::qt_gl_renderer::glDraw() {
    // overloaded to prevent this thread from makeCurrent()
    // (do nothing, rendering done automatically)
    trigger_rendering();
}

void age::qt_gl_renderer::paintEvent(QPaintEvent*)
{
    // overloaded to prevent this thread from makeCurrent()
    // (do nothing, rendering done automatically)
    trigger_rendering();
}

void age::qt_gl_renderer::resizeEvent(QResizeEvent *eve)
{
    // overloaded to prevent this thread from makeCurrent()
    const QSize &new_size = eve->size();
    std::function<void(const QSize&)> tmp = [this] (const QSize &new_size)
    {
        m_events.m_viewport_width = new_size.width();
        m_events.m_viewport_height = new_size.height();
    };
    locked_flag_operation<const QSize&>(tmp, qt_renderer_change_viewport, new_size);
}





//---------------------------------------------------------
//
//   thread run method
//
//---------------------------------------------------------

void age::qt_gl_renderer::run()
{
    {
        // initialize opengl stuff
        makeCurrent();

        m_opengl_3.glEnable(GL_TEXTURE_2D);
        m_opengl_3.glEnable(GL_BLEND);
        m_opengl_3.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        video_frame_manager frame_manager;
        doneCurrent();

        events events_copy;

        // opengl thread loop
        bool stop = false;
        while (!stop)
        {

            // ---- locked actions

            {
                std::unique_lock<std::mutex> lock(m_event_mutex);

                // don't wait if we already know of a change
                // (otherwise we risk an unnoticed change)
                if (m_events.m_flags == 0)
                {
                    m_event_wait.wait(lock);
                }

                // copy events while locked
                events_copy = m_events;
                m_events.m_new_video_frame = std::shared_ptr<const pixel_vector>(); // "release" this pointer
                m_events.m_flags = 0;
            }

            // ----- unlocked actions

            makeCurrent();

            process_events(events_copy, frame_manager);
            render(frame_manager);

            doneCurrent();

            if (events_copy.is_flagged(qt_renderer_stopped))
            {
                stop = true;
            }
            ++m_frame_counter;
        }

        makeCurrent();
        // let frame_manager go out of scope which includes texture deletion
        // (we have to do this while owning the rendering context)
    }
    doneCurrent();
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

void age::qt_gl_renderer::render(video_frame_manager &frame_manager)
{
    m_opengl_3.glClear(GL_COLOR_BUFFER_BIT);
    m_opengl_3.glLoadIdentity();

    for (uint frame = 1; frame <= m_num_video_frames_to_blend; ++frame)
    {
        frame_manager.bind_frame(frame - 1);

        m_opengl_3.glColor4d(1.0, 1.0, 1.0, 1.0 / frame);
        m_opengl_3.glBegin(GL_TRIANGLE_STRIP);
        m_opengl_3.glTexCoord2f(0.0, 1.0); m_opengl_3.glVertex2f(0.0, 0.0);
        m_opengl_3.glTexCoord2f(0.0, 0.0); m_opengl_3.glVertex2f(0.0, 1.0);
        m_opengl_3.glTexCoord2f(1.0, 1.0); m_opengl_3.glVertex2f(1.0, 0.0);
        m_opengl_3.glTexCoord2f(1.0, 0.0); m_opengl_3.glVertex2f(1.0, 1.0);
        m_opengl_3.glEnd();
    }

    swapBuffers();
}

void age::qt_gl_renderer::trigger_rendering()
{
    m_event_wait.notify_all();
}



template<typename... ARGS>
void age::qt_gl_renderer::locked_flag_operation(std::function<void(ARGS...)> func, uint flag, ARGS... args)
{
    { // extra scope to keep the lock as short as possible
        std::lock_guard<std::mutex> lock(m_event_mutex);
        func(args...);
        m_events.m_flags |= flag;
    }
    trigger_rendering();
}



void age::qt_gl_renderer::process_events(events &eve, video_frame_manager &frame_manager)
{
    // change number of frames to blend
    if (eve.is_flagged(qt_renderer_change_frames_to_blend))
    {
        m_num_video_frames_to_blend = eve.m_video_frames_to_blend;
        LOG("number of frames to blend adjusted to " << m_num_video_frames_to_blend);
    }

    // bilinear changed
    if (eve.is_flagged(qt_renderer_change_bilinear))
    {
        frame_manager.set_bilinear(eve.m_bilinear);
    }

    // filter chain changed
    if (eve.is_flagged(qt_renderer_change_filter_chain))
    {
        frame_manager.set_filter_chain(eve.m_filter_chain);
    }

    // add new frame
    if (eve.is_flagged(qt_renderer_new_video_frame))
    {
        frame_manager.add_new_frame(*eve.m_new_video_frame);
    }

    // screen size changed -> update frame texture size
    if (eve.is_flagged(qt_renderer_change_emulator_screen_size))
    {
        frame_manager.set_frame_size(eve.m_screen_width, eve.m_screen_height);
        eve.m_flags |= qt_renderer_change_viewport; // we have to calculate a new viewport now
    }

    // viewport has to be changed
    if (eve.is_flagged(qt_renderer_change_viewport))
    {
        AGE_ASSERT(eve.m_viewport_width > 0);
        AGE_ASSERT(eve.m_viewport_height > 0);

        // find biggest rectangle fitting the ratio
        double ratio = static_cast<double>(frame_manager.get_frame_width()) / static_cast<double>(frame_manager.get_frame_height());
        GLsizei view_width = static_cast<GLsizei>(eve.m_viewport_height * ratio);
        GLsizei view_height = eve.m_viewport_height;
        if (view_width > eve.m_viewport_width)
        {
            view_width = eve.m_viewport_width;
            view_height = static_cast<GLsizei>(eve.m_viewport_width / ratio);
            if (view_height > eve.m_viewport_height)
            {
                view_height = eve.m_viewport_height; // may happen due to rounding errors
            }
        }
        GLint ofs_x = (eve.m_viewport_width - view_width) / 2;
        GLint ofs_y = (eve.m_viewport_height - view_height) / 2;

        // set viewport
        AGE_ASSERT(ofs_x >= 0);
        AGE_ASSERT(ofs_y >= 0);
        AGE_ASSERT(view_width > 0);
        AGE_ASSERT(view_height > 0);

        m_opengl_3.glViewport(ofs_x, ofs_y, view_width, view_height);

        // set projection matrix
        m_opengl_3.glMatrixMode(GL_PROJECTION);
        m_opengl_3.glLoadIdentity();
        m_opengl_3.glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
        m_opengl_3.glMatrixMode(GL_MODELVIEW);
    }
}
