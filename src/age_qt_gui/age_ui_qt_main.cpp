//
// Copyright (c) 2010-2018 Christoph Sprenger
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

#include <memory>

#include <QApplication>

#include <age_types.hpp>

#include "age_ui_qt_emulator.hpp"
#include "age_ui_qt_main_window.hpp"

// allow special types to be used as event parameter
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
    w.start();

    return a.exec();
}
