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

#include <cstring> // strncmp

#include <QApplication>
#include <QCoreApplication>

#include <age_types.hpp>

#include "age_ui_qt_emulator.hpp"
#include "age_ui_qt_main_window.hpp"

//
// allow AGE types to be used as event parameter
// (e.g. for connecting signals and slots with that parameter across multiple threads)
//
// IMPORTANT:
//  - register these types using qRegisterMetaType (see below)
//  - use fully qualified types when connecting signals and slots
//
Q_DECLARE_METATYPE(age::int16_t)
Q_DECLARE_METATYPE(QSharedPointer<age::qt_emulator>)
Q_DECLARE_METATYPE(age::qt_downsampler_quality)
Q_DECLARE_METATYPE(QSharedPointer<const age::pixel_vector>)



static void evaluate_opengl_args(int argc, char* argv[])
{
    // Qt dynamic OpenGL implementation loading:
    // http://blog.qt.io/blog/2014/11/27/qt-weekly-21-dynamic-opengl-implementation-loading-in-qt-5-4/

    bool force_opengl    = false;
    bool force_opengl_es = false;

    for (int i = 0; i < argc; ++i)
    {
        force_opengl    = force_opengl || (0 == strncmp("--opengl", argv[i], 9));
        force_opengl_es = force_opengl_es || (0 == strncmp("--opengl-es", argv[i], 12));
    }
    if (force_opengl && force_opengl_es)
    {
        fprintf(stderr, "--opengl and --opengl-es cannot be used together");
        ::exit(EXIT_FAILURE);
    }

    if (force_opengl)
    {
        AGE_LOG("setting attribute Qt::AA_UseDesktopOpenGL")
        QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    }
    if (force_opengl_es)
    {
        AGE_LOG("setting attribute Qt::AA_UseOpenGLES")
        QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    }
}



int main(int argc, char* argv[])
{
    evaluate_opengl_args(argc, argv);

    QApplication a(argc, argv);

    qRegisterMetaType<age::int16_t>("int16_t");
    qRegisterMetaType<QSharedPointer<age::qt_emulator>>();
    qRegisterMetaType<age::qt_downsampler_quality>();
    qRegisterMetaType<QSharedPointer<const age::pixel_vector>>();

    age::qt_main_window w;
    w.show();

    return a.exec();
}
