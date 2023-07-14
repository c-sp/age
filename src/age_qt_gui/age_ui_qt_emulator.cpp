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

#include "age_ui_qt_emulator.hpp"

#include <age_utilities.hpp>
#include <emulator/age_gb_emulator.hpp>

#include <cassert>
#include <utility> // std::move

#include <QVariant>



//---------------------------------------------------------
//
//   object constructor & destructor
//
//---------------------------------------------------------

age::qt_emulator::qt_emulator(const QByteArray& rom_contents, gb_device_type device_type, QSharedPointer<qt_user_value_store> user_value_store)
    : m_user_value_store(std::move(user_value_store))
{
    // create emulator
    uint8_vector                rom(rom_contents.begin(), rom_contents.end());
    gb_log_categories           log_categories{gb_log_category::lc_memory};
    QSharedPointer<gb_emulator> gb_emu = QSharedPointer<gb_emulator>(new gb_emulator(rom, device_type, gb_colors_hint::default_colors, log_categories));

    // load persistent ram, if supported by this cartridge
    if (!gb_emu->get_persistent_ram().empty())
    {
        // create the user value identifier string for loading the persistent ram
        m_ram_key = QString(gb_emu->get_emulator_title().c_str())
                        .append("_")
                        .append(QString::number(crc32(rom), 16))
                        .append(".ram");

        // load persistent ram from user values
        QVariant   ram_value = m_user_value_store->get_value(m_ram_key);
        QByteArray ram       = ram_value.toByteArray();

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
        uint8_vector ram = m_emulator->get_persistent_ram();

        assert(ram.size() <= int_max);
        int         ram_size = static_cast<int>(ram.size());
        const char* ram_data = reinterpret_cast<const char*>(ram.data());

        QByteArray ram_array = {ram_data, ram_size};
        m_user_value_store->set_value(m_ram_key, ram_array);
    }
}



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

QSharedPointer<age::gb_emulator> age::qt_emulator::get_emulator()
{
    return m_emulator;
}
