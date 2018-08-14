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

#include <QFile>
#include <QOpenGLContext>

#include "age_ui_qt_renderer.hpp"



QString age::load_shader(const QString &file_name)
{
    // we need an OpenGL context for checking the OpenGL version
    bool is_opengl_es;
    {
        QOpenGLContext *ctx = QOpenGLContext::currentContext();
        if (nullptr == ctx)
        {
            qFatal("no OpenGL context available");
            return "";
        }
        is_opengl_es = ctx->isOpenGLES();
    }

    // load the shader code
    QString shader_code;
    {
        QFile file(file_name);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
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
    // According to the GLSL ES 1.0 spec the only type missing a default
    // precision qualifier is "float" in fragment shaders.
    // For simplicity reasons we add the qualifier for all shaders
    // (vertex and fragment).
    //
    // -> https://www.khronos.org/registry/OpenGL/specs/es/2.0/GLSL_ES_Specification_1.00.pdf
    //
    if (is_opengl_es) {
        result.append("precision mediump float;\n");
    }

    //
    // QOpenGLShaderProgram already prefixes all shaders
    // with #defines for "highp", "mediump" and "lowp".
    //
    // -> http://doc.qt.io/qt-5/qopenglshaderprogram.html#writing-portable-shaders
    //

    result.append(shader_code);
    return result;
}
