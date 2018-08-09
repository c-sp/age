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

#include <memory>

#include <QApplication>

#include <age_types.hpp>

#include "age_ui_qt_emulator.hpp"
#include "age_ui_qt_main_window.hpp"

// allow AGE types to be used as event parameter
// (e.g. for connecting signals and slots with that parameter across multiple threads)
Q_DECLARE_METATYPE(std::shared_ptr<age::qt_emulator>)
Q_DECLARE_METATYPE(age::uint)
Q_DECLARE_METATYPE(age::qt_downsampler_quality)



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // register special types to allow queueing arguments of that type
    // (e.g. for connecting signals and slots with that parameter across multiple threads)
    qRegisterMetaType<std::shared_ptr<age::qt_emulator>>();
    qRegisterMetaType<age::uint>();
    qRegisterMetaType<age::qt_downsampler_quality>();

    age::qt_main_window w;
    w.show();

    return a.exec();
}
