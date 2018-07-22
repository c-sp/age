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

#include <algorithm>
#include <iomanip> // std::quoted
#include <ios> // std::hex
#include <limits>
#include <sstream>

#include <QVariant>

#include <age_debug.hpp>
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

age::qt_emulator::qt_emulator(const QByteArray &rom_contents, bool force_dmg, std::shared_ptr<qt_user_value_store> user_value_store)
    : m_user_value_store(user_value_store)
{
    // create emulator
    uint8_vector rom = to_vector(rom_contents);
    gb_model model = force_dmg ? gb_model::dmg : gb_model::auto_detect;
    std::shared_ptr<gb_emulator> gb_emu = std::allocate_shared<gb_emulator>(std::allocator<gb_emulator>(), rom, model);
    LOG("emulator created");

    // load persistent ram, if this cartridge supports it
    if (!gb_emu->get_persistent_ram().empty())
    {
        // create the user value identifier string for loading the persistent ram
        {
            std::stringstream ram_id;
            ram_id << gb_emu->get_emulator_title() << "_";
            ram_id << std::hex << hash(rom) << ".ram";
            m_ram_key = {ram_id.str().c_str()};
        }
        LOG("persistent ram key is " << std::quoted(m_ram_key.toStdString()));

        // load persistent ram from user values
        QVariant ram_value = m_user_value_store->get_value(m_ram_key);
        QByteArray ram = ram_value.toByteArray();
        LOG("loaded " << ram.size() << " bytes of persistent ram");

        if (ram.size() > 0)
        {
            gb_emu->set_persistent_ram(to_vector(ram));
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
        LOG("storing persistent ram with key " << std::quoted(m_ram_key.toStdString()));

        uint8_vector ram = m_emulator->get_persistent_ram();
        const char *ram_data = reinterpret_cast<const char*>(ram.data());
        int ram_size = static_cast<int>(ram.size());
        AGE_ASSERT(ram.size() <= std::numeric_limits<int>::max());

        QByteArray ram_array = {ram_data, ram_size};
        m_user_value_store->set_value(m_ram_key, ram_array);
    }
}





//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

std::shared_ptr<age::emulator> age::qt_emulator::get_emulator()
{
    return m_emulator;
}





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

age::uint64 age::qt_emulator::hash(const uint8_vector &vector)
{
    uint64 result = 0;

    for (uint8 byte : vector)
    {
        // based on boost:
        // http://www.boost.org/doc/libs/1_63_0/boost/functional/hash/hash.hpp
        result ^= byte + 0x9e3779b9 + (result << 6) + (result >> 2);
    }

    return result;
}



age::uint8_vector age::qt_emulator::to_vector(const QByteArray &byte_array)
{
    // this looks redundant, but I don't see any other way
    // to convert a QByteArray to an std::vector ...

    uint8_vector result;
    result.resize(byte_array.size());
    std::copy(byte_array.constData(), byte_array.constData() + byte_array.size(), begin(result));

    return result;
}
