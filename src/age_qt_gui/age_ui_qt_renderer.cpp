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
#include <QSurfaceFormat>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

#include <age_debug.hpp>

#include "age_ui_qt_renderer.hpp"

#if 1
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif

struct VertexData
{
    QVector3D position;
    QVector2D texCoord;
};



constexpr const char *vshader =
        R"(
        // Set default precision to medium
        #ifdef GL_ES
        precision mediump int;
        precision mediump float;
        #endif

        uniform mat4 u_projection;
        uniform vec4 u_color; // used to blend frames

        attribute vec4 a_vertex;
        attribute vec2 a_texcoord;

        varying vec2 v_texcoord;
        varying vec4 v_color;

        void main()
        {
            gl_Position = u_projection * a_vertex;
            v_texcoord = a_texcoord;
            v_color = u_color;
        }
        )";

constexpr const char *fshader =
        R"(
        // Set default precision to medium
        #ifdef GL_ES
        precision mediump int;
        precision mediump float;
        #endif

        uniform sampler2D texture;

        varying vec2 v_texcoord;
        varying vec4 v_color;

        void main()
        {
            gl_FragColor = v_color * texture2D(texture, v_texcoord);
        }
        )";

constexpr QOpenGLTexture::PixelFormat tx_pixel_format = QOpenGLTexture::RGBA;
constexpr QOpenGLTexture::PixelType tx_pixel_type = QOpenGLTexture::UInt8;



//---------------------------------------------------------
//
//   constructor & destructor
//
//---------------------------------------------------------

age::qt_renderer::qt_renderer(QWidget *parent)
    : QOpenGLWidget(parent),
      m_indices(QOpenGLBuffer::IndexBuffer)
{
    // Which OpenGL Version is being used?
    // https://stackoverflow.com/questions/41021681/qt-how-to-detect-which-version-of-opengl-is-being-used
    LOG("OpenGL Module Type: " << QOpenGLContext::openGLModuleType()
        << " (LibGL " << QOpenGLContext::LibGL << ", LibGLES " << QOpenGLContext::LibGLES << ")");

    LOG("default format version: " << format().majorVersion() << "." << format().minorVersion());
    LOG("default format options: " << format().options());
}

age::qt_renderer::~qt_renderer()
{
    makeCurrent();

    m_vertices.destroy();
    m_indices.destroy();

    m_last_frame_texture = nullptr;

    doneCurrent();
}





//---------------------------------------------------------
//
//   public interface
//
//---------------------------------------------------------

age::uint age::qt_renderer::get_fps() const
{
    //! \todo implement age::qt_renderer::get_fps
    return 0;
}



void age::qt_renderer::set_emulator_screen_size(uint width, uint height)
{
    LOG(width << ", " << height);
    m_emulator_screen = QSize(width, height);

    // allocate texture only after initializeGL() has been called
    if (m_last_frame_texture != nullptr)
    {
        makeCurrent();
        allocate_textures();
        doneCurrent();
    }

    update_projection_matrix();

    update(); // trigger paintGL()
}

void age::qt_renderer::new_frame(std::shared_ptr<const age::pixel_vector> new_frame)
{
    new_frame_slot(new_frame);
}



void age::qt_renderer::set_blend_frames(uint num_frames_to_blend)
{
    LOG(num_frames_to_blend);
    //! \todo implement age::qt_renderer::set_blend_frames
}

void age::qt_renderer::set_filter_chain(qt_filter_vector filter_chain)
{
    LOG("#filters: " << filter_chain.size());
    //! \todo implement age::qt_renderer::set_filter_chain
}

void age::qt_renderer::set_bilinear_filter(bool bilinear_filter)
{
    m_bilinear_filter = bilinear_filter;

    // update textures only after initializeGL() has been called
    if (m_last_frame_texture != nullptr)
    {
        makeCurrent();
        set_texture_filter();
        doneCurrent();
    }
}





//---------------------------------------------------------
//
//   OpenGL rendering
//
//---------------------------------------------------------

void age::qt_renderer::initializeGL()
{
    LOG("format version: " << format().majorVersion() << "." << format().minorVersion());

    initializeOpenGLFunctions();

    // Log OpenGL Version information.
    // (OpenGL functions must be initialized)
    LOG("GL_VERSION: " << glGetString(GL_VERSION));
    LOG("GL_SHADING_LANGUAGE_VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION));

    // OpenGL configuration
    glClearColor(0, 0, 0, 1);
    glEnable(GL_TEXTURE_2D);

    // shader program (failures are logged by Qt)
    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vshader);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fshader);
    m_program.link();

    // vertex buffer
    VertexData vertices[] = {
        {QVector3D(0, 0, 0), QVector2D(0, 0)},
        {QVector3D(0, 1, 0), QVector2D(0, 1)},
        {QVector3D(1, 0, 0), QVector2D(1, 0)},
        {QVector3D(1, 1, 0), QVector2D(1, 1)},
    };

    m_vertices.create();
    m_vertices.bind();
    m_vertices.allocate(vertices, 4 * sizeof(VertexData));

    // index buffer
    GLushort indices[] = {0, 1, 2, 3};

    m_indices.create();
    m_indices.bind();
    m_indices.allocate(indices, 4 * sizeof(GLushort));

    // create textures
    allocate_textures();
}



void age::qt_renderer::resizeGL(int width, int height)
{
    LOG("viewport size: " << width << " x " << height);
    m_current_viewport = QSize(width, height);

    update_projection_matrix();
}



void age::qt_renderer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    m_program.bind();
    m_program.setUniformValue("u_projection", m_projection);
    m_program.setUniformValue("u_color", QVector4D(1, 1, 1, 1));

    m_vertices.bind();
    m_indices.bind();

    int vertexLocation = m_program.attributeLocation("a_vertex");
    m_program.enableAttributeArray(vertexLocation);
    m_program.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(VertexData));

    int texcoordLocation  = m_program.attributeLocation("a_texcoord");
    m_program.enableAttributeArray(texcoordLocation);
    m_program.setAttributeBuffer(texcoordLocation, GL_FLOAT, sizeof(QVector3D), 2, sizeof(VertexData));

    m_last_frame_texture->bind();
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, nullptr);
}



void age::qt_renderer::update_projection_matrix()
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
        AGE_ASSERT(diff >= 0);
        proj = QRectF(0, -.5 * diff, 1, 1 + diff); // x, y, width, height
    }

    m_projection.setToIdentity();
    m_projection.ortho(proj);
}





//---------------------------------------------------------
//
//   new frame event handling
//
//---------------------------------------------------------

void age::qt_renderer::new_frame_slot(std::shared_ptr<const pixel_vector> new_frame)
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

void age::qt_renderer::process_new_frame()
{
    makeCurrent();
    m_last_frame_texture->setData(tx_pixel_format, tx_pixel_type, m_new_frame->data());
    doneCurrent();

    m_new_frame = nullptr; // allow next frame to be processed

    update(); // trigger paintGL
}



void age::qt_renderer::allocate_textures()
{
    LOG("");

    m_last_frame_texture = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);

    m_last_frame_texture->setFormat(QOpenGLTexture::RGB8_UNorm);
    m_last_frame_texture->setSize(m_emulator_screen.width(), m_emulator_screen.height());
    m_last_frame_texture->allocateStorage(tx_pixel_format, tx_pixel_type);

    set_texture_filter();
}

void age::qt_renderer::set_texture_filter()
{
    LOG(m_bilinear_filter);

    auto mag_filter = m_bilinear_filter ? QOpenGLTexture::Linear : QOpenGLTexture::Nearest;
    // always use linear filter for rendering downscaled texture
    m_last_frame_texture->setMinMagFilters(QOpenGLTexture::Linear, mag_filter);
}
