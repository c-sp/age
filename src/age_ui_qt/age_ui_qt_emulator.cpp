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
    std::shared_ptr<gb_emulator> gb_sim = std::allocate_shared<gb_emulator>(std::allocator<gb_emulator>(), rom, force_dmg);
    LOG("emulator created");

    // load persistent ram, if this cartridge supports it
    if (!gb_sim->get_persistent_ram().empty())
    {
        // create the user value identifier string for loading the persistent ram
        {
            std::stringstream ram_id;
            ram_id << gb_sim->get_emulator_title() << "_";
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
            gb_sim->set_persistent_ram(to_vector(ram));
        }
    }

    // done
    m_emulator = gb_sim;
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
