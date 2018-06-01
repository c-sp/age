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

#ifndef AGE_UI_QT_EMULATOR_HPP
#define AGE_UI_QT_EMULATOR_HPP

//!
//! \file
//!

#include <memory> // std::shared_ptr

#include <QByteArray>
#include <QString>

#include <age_types.hpp>
#include <emulator/age_emulator.hpp>

#include "age_ui_qt.hpp"
#include "age_ui_qt_user_value_store.hpp"



namespace age
{

class qt_emulator
{
public:

    qt_emulator(const QByteArray &rom, bool force_dmg, std::shared_ptr<qt_user_value_store> user_value_store);
    ~qt_emulator();

    std::shared_ptr<emulator> get_emulator();

private:

    static uint64 hash(const uint8_vector &vector);
    static uint8_vector to_vector(const QByteArray &byte_array);

    QString m_ram_key;
    std::shared_ptr<emulator> m_emulator;
    std::shared_ptr<qt_user_value_store> m_user_value_store;
};

} // namespace age



#endif // AGE_UI_QT_EMULATOR_HPP
