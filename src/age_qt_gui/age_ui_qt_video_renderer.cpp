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

    // shader program
    qt_init_shader_program(m_program, ":/age_ui_qt_render_vsh.glsl", ":/age_ui_qt_render_fsh.glsl");

    // vertex buffer
    std::array vertices{
        qt_vertex_data{QVector3D(0, 0, 0), QVector2D(0, 0)},
        qt_vertex_data{QVector3D(0, 1, 0), QVector2D(0, 1)},
        qt_vertex_data{QVector3D(1, 0, 0), QVector2D(1, 0)},
        qt_vertex_data{QVector3D(1, 1, 0), QVector2D(1, 1)},
    };

    m_vertices.create();
    m_vertices.bind();
    m_vertices.allocate(vertices.data(), static_cast<int>(sizeof_array(vertices)));

    // index buffer
    std::array<GLushort, 4> indices{0, 1, 2, 3};

    m_indices.create();
    m_indices.bind();
    m_indices.allocate(indices.data(), static_cast<int>(sizeof_array(indices)));
}

age::qt_video_renderer::~qt_video_renderer()
{
    m_indices.destroy();
    m_vertices.destroy();
    LOG("");
}



void age::qt_video_renderer::update_matrix(const QSize& emulator_screen, const QSize& viewport)
{
    LOG(emulator_screen.width() << " x " << emulator_screen.height() << " on " << viewport.width() << " x " << viewport.height());

    double viewport_ratio = 1. * viewport.width() / viewport.height();
    double screen_ratio   = 1. * emulator_screen.width() / emulator_screen.height();

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



void age::qt_video_renderer::render(const QList<GLuint>& textures_to_render)
{
    m_program.bind();
    m_program.setUniformValue("u_projection", m_projection);

    m_vertices.bind();
    m_indices.bind();

    qt_use_float_attribute_buffer(m_program, "a_vertex", 0, 3, sizeof(qt_vertex_data));
    qt_use_float_attribute_buffer(m_program, "a_texcoord", sizeof(QVector3D), 2, sizeof(qt_vertex_data));

    for (int i = 0; i < textures_to_render.size(); ++i)
    {
        m_program.setUniformValue("u_color", QVector4D(1, 1, 1, 1.F / (static_cast<float>(i) + 1)));
        glBindTexture(GL_TEXTURE_2D, textures_to_render[i]);
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, nullptr);
    }
}
