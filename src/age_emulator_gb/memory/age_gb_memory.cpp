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

#include "age_gb_memory.hpp"

#include <algorithm>
#include <cassert>

namespace
{
    constexpr age::uint16_t gb_cia_ofs_title = 0x0134;

    bool is_cartridge_ram(uint16_t address)
    {
        return (address & 0xE000) == 0xA000;
    }

} // namespace





//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

const age::uint8_t* age::gb_memory::get_video_ram() const
{
    return &m_memory[m_video_ram_offset];
}

const age::uint8_t* age::gb_memory::get_rom_header() const
{
    return m_memory.data();
}

std::string age::gb_memory::get_cartridge_title() const
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const char* buffer = reinterpret_cast<const char*>(&m_memory[gb_cia_ofs_title]);
    std::string result = {buffer, 16};
    return result;
}



age::uint8_vector age::gb_memory::get_persistent_ram() const
{
    uint8_vector result;

    if (m_has_battery && (m_num_cart_ram_banks > 0))
    {
        int cart_ram_size = m_num_cart_ram_banks * gb_cart_ram_bank_size;

        assert(cart_ram_size > 0);
        result.resize(static_cast<unsigned>(cart_ram_size));

        std::copy(begin(m_memory) + m_cart_ram_offset,
                  begin(m_memory) + m_cart_ram_offset + cart_ram_size,
                  begin(result));
    }
    else
    {
        result = uint8_vector();
    }

    return result;
}

void age::gb_memory::set_persistent_ram(const uint8_vector& source)
{
    if (m_has_battery && (m_num_cart_ram_banks > 0))
    {
        assert(source.size() <= int_max);
        int source_size   = static_cast<int>(source.size());
        int cart_ram_size = m_num_cart_ram_banks * gb_cart_ram_bank_size;

        // copy persistent ram
        int bytes_to_copy = std::min(cart_ram_size, source_size);

        std::copy(begin(source),
                  begin(source) + bytes_to_copy,
                  begin(m_memory) + m_cart_ram_offset);

        // fill up with zeroes, if the source vector is smaller than the actual persistent ram
        if (cart_ram_size > source_size)
        {
            std::fill(begin(m_memory) + m_cart_ram_offset + source_size,
                      begin(m_memory) + m_cart_ram_offset + cart_ram_size,
                      0);
        }
    }
}



age::uint8_t age::gb_memory::read_byte(uint16_t address)
{
    assert(address < 0xFE00);
    // rom, video ram & work ram
    if (!is_cartridge_ram(address))
    {
        return m_memory[get_offset(address)];
    }
    // cartridge ram not readable
    if (!m_cart_ram_enabled)
    {
        log_mbc() << "read [" << log_hex16(address) << "] = 0xFF: cartridge RAM disabled";
        return 0xFF;
    }
    // read cartridge ram
    return m_cart_ram_read(*this, address);
}

age::uint8_t age::gb_memory::read_svbk() const
{
    return m_svbk;
}

age::uint8_t age::gb_memory::read_vbk() const
{
    return m_vbk;
}



void age::gb_memory::write_byte(uint16_t address, uint8_t value)
{
    assert(address < 0xFE00);
    // access MBC
    if (address < 0x8000)
    {
        log_mbc() << "writing " << log_hex16(address) << " = " << log_hex8(value);
        m_mbc_write(*this, address, value);
        return;
    }
    // write video ram & work ram
    if (!is_cartridge_ram(address))
    {
        m_memory[get_offset(address)] = value;
        return;
    }
    // cartridge ram not writable
    if (!m_cart_ram_enabled)
    {
        log_mbc() << "ignoring write [" << log_hex16(address) << "] = " << log_hex8(value) << " (cartridge RAM disabled)";
        return;
    }
    // write cartridge ram
    m_cart_ram_write(*this, address, value);
}

void age::gb_memory::write_svbk(uint8_t value)
{
    m_svbk = value | 0xF8;

    int bank_id = value & 0x07;
    if (bank_id == 0)
    {
        bank_id = 1;
    }
    log() << "switch to SVBK bank " << bank_id;

    int offset     = bank_id * gb_work_ram_bank_size;
    m_offsets[0xD] = m_work_ram_offset + offset - 0xD000;
    m_offsets[0xF] = m_work_ram_offset + offset - 0xF000;
}

void age::gb_memory::write_vbk(uint8_t value)
{
    m_vbk = value | 0xFE;

    auto bank = m_vbk & 1;
    log() << "switch to VBK bank " << bank;

    auto offset  = bank * gb_video_ram_bank_size;
    m_offsets[8] = m_video_ram_offset + offset - 0x8000;
    m_offsets[9] = m_offsets[8];
}



void age::gb_memory::update_state()
{
    auto* mbc3rtc_data = std::get_if<gb_mbc3rtc_data>(&m_mbc_data);
    if (mbc3rtc_data == nullptr)
    {
        return;
    }
    mbc3rtc_update(*this);
}

void age::gb_memory::set_back_clock(int clock_cycle_offset)
{
    auto* mbc3rtc_data = std::get_if<gb_mbc3rtc_data>(&m_mbc_data);
    if (mbc3rtc_data == nullptr)
    {
        return;
    }
    gb_set_back_clock_cycle(mbc3rtc_data->m_clks_last_update, clock_cycle_offset);
}





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

unsigned age::gb_memory::get_offset(uint16_t address) const
{
    auto offset = m_offsets[address >> 12];
    offset += address;

    assert(offset >= 0);
    assert(static_cast<unsigned>(offset) < m_memory.size());

    return static_cast<unsigned>(offset);
}

void age::gb_memory::set_cart_ram_enabled(uint8_t value)
{
    m_cart_ram_enabled = (value & 0x0F) == 0x0A;
    log_mbc() << "enable cartridge ram = " << log_hex8(value)
              << " (cartridge ram " << (m_cart_ram_enabled ? "enabled" : "disabled") << ")";
}

void age::gb_memory::set_rom_banks(int low_bank_id, int high_bank_id)
{
    assert(low_bank_id >= 0);
    assert(high_bank_id >= 0);
    assert(m_num_cart_rom_banks >= 0);
    assert((m_num_cart_rom_banks & (m_num_cart_rom_banks - 1)) == 0); // just 1 bit set
    low_bank_id &= m_num_cart_rom_banks - 1;
    high_bank_id &= m_num_cart_rom_banks - 1;

    m_offsets[0] = low_bank_id * gb_cart_rom_bank_size;
    m_offsets[1] = m_offsets[0];
    m_offsets[2] = m_offsets[0];
    m_offsets[3] = m_offsets[0];

    m_offsets[4] = high_bank_id * gb_cart_rom_bank_size - 0x4000;
    m_offsets[5] = m_offsets[4];
    m_offsets[6] = m_offsets[4];
    m_offsets[7] = m_offsets[4];

    log_mbc() << "switch to rom banks " << log_hex(low_bank_id) << " @ 0x0000-0x3FFF, "
              << log_hex(high_bank_id) << " @ 0x4000-0x7FFF";
}

void age::gb_memory::set_ram_bank(int bank_id)
{
    assert(bank_id >= 0);
    assert(m_num_cart_ram_banks >= 0);
    assert((m_num_cart_ram_banks & (m_num_cart_ram_banks - 1)) == 0); // just 1 bit set
    bank_id &= m_num_cart_ram_banks - 1;

    m_offsets[0xA] = m_cart_ram_offset + bank_id * gb_cart_ram_bank_size - 0xA000;
    m_offsets[0xB] = m_offsets[0xA];

    log_mbc() << "switch to ram bank " << log_hex(bank_id);
}



void age::gb_memory::no_mbc_write(gb_memory& memory, uint16_t address, uint8_t value)
{
    memory.log_mbc() << "ignoring write [" << log_hex16(address) << "] = " << log_hex8(value)
                     << " (cartridge has no MBC)";
}

void age::gb_memory::cart_ram_write(gb_memory& memory, uint16_t address, uint8_t value)
{
    if (memory.m_num_cart_ram_banks > 0)
    {
        memory.m_memory[memory.get_offset(address)] = value;
    }
}

age::uint8_t age::gb_memory::cart_ram_read(gb_memory& memory, uint16_t address)
{
    return (memory.m_num_cart_ram_banks > 0)
               ? memory.m_memory[memory.get_offset(address)]
               : 0xFF;
}
