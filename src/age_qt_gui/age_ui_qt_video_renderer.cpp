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
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

#include "age_ui_qt_video.hpp"

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif



age::qt_video_renderer::qt_video_renderer()
    : m_indices(QOpenGLBuffer::IndexBuffer)
{
    LOG("");
    initializeOpenGLFunctions();

    qt_init_shader_program(m_program, ":/age_ui_qt_render_vshader.glsl", ":/age_ui_qt_render_fshader.glsl");

    // vertex buffer
    qt_vertex_data vertices[] = {
        {QVector3D(0, 0, 0), QVector2D(0, 0)},
        {QVector3D(0, 1, 0), QVector2D(0, 1)},
        {QVector3D(1, 0, 0), QVector2D(1, 0)},
        {QVector3D(1, 1, 0), QVector2D(1, 1)},
    };

    m_vertices.create();
    m_vertices.bind();
    m_vertices.allocate(vertices, 4 * sizeof(qt_vertex_data));

    // index buffer
    GLushort indices[] = {0, 1, 2, 3};

    m_indices.create();
    m_indices.bind();
    m_indices.allocate(indices, 4 * sizeof(GLushort));
}

age::qt_video_renderer::~qt_video_renderer()
{
    m_indices.destroy();
    m_vertices.destroy();
    LOG("");
}



void age::qt_video_renderer::update_matrix(const QSize &emulator_screen, const QSize &viewport)
{
    LOG(emulator_screen.width() << " x " << emulator_screen.height() << " on " << viewport.width() << " x " << viewport.height());

    double viewport_ratio = 1. * viewport.width() / viewport.height();
    double screen_ratio = 1. * emulator_screen.width() / emulator_screen.height();

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



void age::qt_video_renderer::render(const QList<GLuint> &textures_to_render)
{
    m_program.bind();
    m_program.setUniformValue("u_projection", m_projection);

    m_vertices.bind();
    m_indices.bind();

    int vertexLocation = m_program.attributeLocation("a_vertex");
    m_program.enableAttributeArray(vertexLocation);
    m_program.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(qt_vertex_data));

    int texcoordLocation  = m_program.attributeLocation("a_texcoord");
    m_program.enableAttributeArray(texcoordLocation);
    m_program.setAttributeBuffer(texcoordLocation, GL_FLOAT, sizeof(QVector3D), 2, sizeof(qt_vertex_data));

    for (int i = 0; i < textures_to_render.size(); ++i)
    {
        m_program.setUniformValue("u_color", QVector4D(1, 1, 1, 1.f / (i + 1)));
        glBindTexture(GL_TEXTURE_2D, textures_to_render[i]);
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, nullptr);
    }
}
