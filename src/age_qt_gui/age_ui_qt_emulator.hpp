//
// © 2017 Christoph Sprenger <https://github.com/c-sp>
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

#include <QByteArray>
#include <QString>

#include <emulator/age_gb_emulator.hpp>
#include <emulator/age_gb_types.hpp>

#include "age_ui_qt_user_value_store.hpp"



namespace age
{

    class qt_emulator
    {
    public:
        qt_emulator(const QByteArray& rom, gb_device_type device_type, QSharedPointer<qt_user_value_store> user_value_store);
        ~qt_emulator();

        QSharedPointer<gb_emulator> get_emulator();

    private:
        QString                             m_ram_key;
        QSharedPointer<gb_emulator>         m_emulator;
        QSharedPointer<qt_user_value_store> m_user_value_store;
    };

} // namespace age



#endif // AGE_UI_QT_EMULATOR_HPP
