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

#ifndef AGE_UI_QT_SIMULATOR_HPP
#define AGE_UI_QT_SIMULATOR_HPP

//!
//! \file
//!

#include "age_ui_qt.hpp"
#include "age_ui_qt_user_value_store.hpp"



namespace age
{

class qt_simulator
{
public:

    qt_simulator(const QByteArray &rom, bool force_dmg, std::shared_ptr<qt_user_value_store> user_value_store);
    ~qt_simulator();

    std::shared_ptr<simulator> get_simulator();

private:

    static uint64 hash(const uint8_vector &vector);
    static uint8_vector to_vector(const QByteArray &byte_array);

    QString m_ram_key;
    std::shared_ptr<simulator> m_simulator;
    std::shared_ptr<qt_user_value_store> m_user_value_store;
};

} // namespace age



#endif // AGE_UI_QT_SIMULATOR_HPP
