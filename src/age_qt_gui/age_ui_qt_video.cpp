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

#include <QFile>
#include <QOpenGLContext>
#include <QRectF>
#include <QSurfaceFormat>
#include <QTimer>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

#include <age_debug.hpp>

#include "age_ui_qt_video.hpp"

#include <utility> // std::move

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif



namespace
{

    QString load_shader(const QString& file_name)
    {
        // we need an OpenGL context for checking the OpenGL version
        bool is_opengl_es;
        {
            QOpenGLContext* ctx = QOpenGLContext::currentContext();
            if (nullptr == ctx)
            {
                qFatal("no OpenGL context available");
            }
            is_opengl_es = ctx->isOpenGLES();
        }

        // load the shader code
        QString shader_code;
        {
            QFile file(file_name);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                qWarning() << "could not open file: " << file_name;
                return "";
            }

            shader_code = QString(file.readAll()); // QString created using fromUtf8()
            file.close();
        }

        //
        // The #version directive is added to the shader depending
        // on the current OpenGL implementation being used.
        //
        // We aim for high OpenGL compatibility by using shaders
        // requiring OpenGL ES 2.0 (OpenGL 2.1).
        //
        // -> https://en.wikipedia.org/wiki/OpenGL_Shading_Language#Versions
        //
        QString result = is_opengl_es ? "#version 100\n" : "#version 120\n";

        //
        // Add default precision qualifiers.
        //
        // According to the GLSL ES 1.0 spec vertex shaders default to
        // "highp" for float and int and fragment shaders default to
        // "mediump" for int (they are missing the default precision
        // qualifier for float).
        // For simplicity reasons we add the same qualifiers to all
        // shaders (vertex and fragment).
        //
        // -> https://www.khronos.org/registry/OpenGL/specs/es/2.0/GLSL_ES_Specification_1.00.pdf
        //
        // QOpenGLShaderProgram already prefixes all shaders
        // with #defines for "highp", "mediump" and "lowp",
        // so we can already use these keywords in our shader code.
        //
        // -> http://doc.qt.io/qt-5/qopenglshaderprogram.html#writing-portable-shaders
        //
        if (is_opengl_es)
        {
            result.append("precision mediump float;\n");
            result.append("precision mediump int;\n");
        }

        result.append(shader_code);
        return result;
    }

} // namespace

void age::qt_init_shader_program(QOpenGLShaderProgram& program, const QString& vertex_shader_file, const QString& fragment_shader_file)
{
    LOG("creating shader program from " << vertex_shader_file << " (vert) and " << fragment_shader_file << " (frag)");

    // failures are logged by Qt
    program.addShaderFromSourceCode(QOpenGLShader::Vertex, load_shader(vertex_shader_file));
    program.addShaderFromSourceCode(QOpenGLShader::Fragment, load_shader(fragment_shader_file));
    program.link();
}

void age::qt_use_float_attribute_buffer(QOpenGLShaderProgram& program, const char* attribute_name, int offset, int tuple_size, int stride)
{
    int attribute_location = program.attributeLocation(attribute_name);
    program.enableAttributeArray(attribute_location);
    program.setAttributeBuffer(attribute_location, GL_FLOAT, offset, tuple_size, stride);
}





//---------------------------------------------------------
//
//   constructor & destructor
//
//---------------------------------------------------------

age::qt_video_output::qt_video_output(QWidget* parent)
    : QOpenGLWidget(parent)
{
    // Which OpenGL Version is being used?
    // https://stackoverflow.com/questions/41021681/qt-how-to-detect-which-version-of-opengl-is-being-used
    LOG("OpenGL Module Type: " << QOpenGLContext::openGLModuleType()
                               << " (LibGL " << QOpenGLContext::LibGL << ", LibGLES " << QOpenGLContext::LibGLES << ")");

    LOG("format version: " << format().majorVersion() << "." << format().minorVersion());
    LOG("format options: " << format().options());

    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &qt_video_output::update_fps);

    timer->setTimerType(Qt::PreciseTimer); // if possible, use millisecond accuracy
    timer->start(1000 / stats_per_second);
}

age::qt_video_output::~qt_video_output()
{
    makeCurrent();

    m_renderer       = nullptr;
    m_post_processor = nullptr;

    doneCurrent();
    LOG("");
}



//---------------------------------------------------------
//
//   public interface
//
//---------------------------------------------------------

void age::qt_video_output::set_emulator_screen_size(int16_t w, int16_t h)
{
    LOG(w << ", " << h);
    m_emulator_screen = QSize(w, h);

    run_if_initialized([this] {
        m_renderer->update_matrix(m_emulator_screen, QSize(width(), height()));
        m_post_processor->set_native_frame_size(m_emulator_screen);
    });
}

void age::qt_video_output::new_frame(QSharedPointer<const age::pixel_vector> new_frame)
{
    new_frame_slot(std::move(new_frame));
}



void age::qt_video_output::set_blend_frames(int num_frames_to_blend)
{
    LOG(num_frames_to_blend);
    m_num_frames_to_blend = num_frames_to_blend;

    update(); // trigger paintGL()
}

void age::qt_video_output::set_post_processing_filter(qt_filter_list filter_list)
{
    LOG("#filters: " << filter_list.size());
    m_filter_list = std::move(filter_list);

    run_if_initialized([this] {
        m_post_processor->set_post_processing_filter(m_filter_list);
    });
}

void age::qt_video_output::set_bilinear_filter(bool bilinear_filter)
{
    LOG(bilinear_filter);
    m_bilinear_filter = bilinear_filter;

    run_if_initialized([this] {
        m_post_processor->set_texture_filter(m_bilinear_filter);
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
    m_renderer = QSharedPointer<qt_video_renderer>(new qt_video_renderer());
    m_renderer->update_matrix(m_emulator_screen, QSize(width(), height()));

    // create post processor
    m_post_processor = QSharedPointer<qt_video_post_processor>(new qt_video_post_processor());
    m_post_processor->set_native_frame_size(m_emulator_screen);
    m_post_processor->set_texture_filter(m_bilinear_filter);
    m_post_processor->set_post_processing_filter(m_filter_list);
}



void age::qt_video_output::resizeGL(int width, int height)
{
    LOG(width << " x " << height);
    m_renderer->update_matrix(m_emulator_screen, QSize(width, height));
}

void age::qt_video_output::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    m_renderer->render(m_post_processor->get_frame_textures(m_num_frames_to_blend));
}



void age::qt_video_output::run_if_initialized(std::function<void()> function_to_run)
{
    if (nullptr != m_post_processor)
    {
        makeCurrent();
        function_to_run();
        doneCurrent();
        update(); // trigger paintGL()
    }
}



//---------------------------------------------------------
//
//   frame event handling
//
//---------------------------------------------------------

void age::qt_video_output::update_fps()
{
    emit fps(m_frame_counter * stats_per_second);
    m_frame_counter = 0;
}

void age::qt_video_output::new_frame_slot(QSharedPointer<const pixel_vector> new_frame)
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
    run_if_initialized([this] {
        AGE_ASSERT(nullptr != m_new_frame)
        m_post_processor->add_new_frame(*m_new_frame);
        ++m_frame_counter;
    });

    m_new_frame = nullptr; // allow next frame to be processed
}
