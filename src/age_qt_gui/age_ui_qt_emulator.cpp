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

#include <QVariant>

#include <age_debug.hpp>
#include <age_utilities.hpp>
#include <emulator/age_gb_emulator.hpp>

#include "age_ui_qt_emulator.hpp"

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif



//---------------------------------------------------------
//
//   object constructor & destructor
//
//---------------------------------------------------------

age::qt_emulator::qt_emulator(const QByteArray &rom_contents, gb_hardware hardware, QSharedPointer<qt_user_value_store> user_value_store)
    : m_user_value_store(user_value_store)
{
    // create emulator
    uint8_vector rom(rom_contents.begin(), rom_contents.end());
    QSharedPointer<gb_emulator> gb_emu = QSharedPointer<gb_emulator>(new gb_emulator(rom, hardware));
    LOG("emulator created");

    // load persistent ram, if supported by this cartridge
    if (!gb_emu->get_persistent_ram().empty())
    {
        // create the user value identifier string for loading the persistent ram
        m_ram_key = QString(gb_emu->get_emulator_title().c_str())
                .append("_")
                .append(QString::number(crc32(rom), 16))
                .append(".ram");
        LOG("persistent ram key is " << m_ram_key);

        // load persistent ram from user values
        QVariant ram_value = m_user_value_store->get_value(m_ram_key);
        QByteArray ram = ram_value.toByteArray();
        LOG("loaded " << ram.size() << " bytes of persistent ram");

        if (ram.size() > 0)
        {
            uint8_vector ram_vec(ram.begin(), ram.end());
            gb_emu->set_persistent_ram(ram_vec);
        }
    }

    // done
    m_emulator = gb_emu;
}



age::qt_emulator::~qt_emulator()
{
    // save persistent ram
    if (m_ram_key.length() > 0)
    {
        LOG("storing persistent ram with key " << m_ram_key);

        uint8_vector ram = m_emulator->get_persistent_ram();

        AGE_ASSERT(ram.size() <= int_max);
        int ram_size = static_cast<int>(ram.size());
        const char *ram_data = reinterpret_cast<const char*>(ram.data());

        QByteArray ram_array = {ram_data, ram_size};
        m_user_value_store->set_value(m_ram_key, ram_array);
    }
}



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

QSharedPointer<age::emulator> age::qt_emulator::get_emulator()
{
    return m_emulator;
}
