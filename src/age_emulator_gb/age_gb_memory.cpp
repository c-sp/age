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

#include <algorithm>

#include <age_debug.hpp>

#include "age_gb_memory.hpp"

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif

#if 0
#define BANK_LOG(x) LOG(x)
#else
#define BANK_LOG(x)
#endif





//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

bool age::gb_memory::is_cgb() const
{
    return m_cgb;
}

const age::uint8* age::gb_memory::get_video_ram() const
{
    return m_memory.data() + m_video_ram_offset;
}

std::string age::gb_memory::get_cartridge_title() const
{
    const char *buffer = reinterpret_cast<const char*>(m_memory.data() + gb_cia_ofs_title);
    std::string result = {buffer, 16};
    return result;
}



age::uint8_vector age::gb_memory::get_persistent_ram() const
{
    uint8_vector result;

    if (m_has_battery)
    {
        uint cart_ram_size = m_num_cart_ram_banks * gb_cart_ram_bank_size;
        result.resize(cart_ram_size);

        std::copy(begin(m_memory) + m_cart_ram_offset,
                  begin(m_memory) + m_cart_ram_offset + cart_ram_size,
                  begin(result));
    }
    else
    {
        result = empty_uint8_vector;
    }

    LOG("returning persistent ram of size " << result.size());
    return result;
}

void age::gb_memory::set_persistent_ram(const uint8_vector &source)
{
    if (m_has_battery)
    {
        uint cart_ram_size = m_num_cart_ram_banks * gb_cart_ram_bank_size;

        // copy persistent ram
        uint bytes_to_copy = std::min(cart_ram_size, source.size());
        LOG("copying " << bytes_to_copy << " bytes into the persistent cartridge ram");

        std::copy(begin(source),
                  begin(source) + bytes_to_copy,
                  begin(m_memory) + m_cart_ram_offset);

        // fill up with zeroes, if the source vector is smaller than the actual persistent ram
        if (cart_ram_size > source.size())
        {
            LOG("filling up " << (cart_ram_size - source.size()) << " bytes of persistent cartridge ram with zeroes");

            std::fill(begin(m_memory) + m_cart_ram_offset + source.size(),
                      begin(m_memory) + m_cart_ram_offset + cart_ram_size,
                      0);
        }
    }
}



age::uint8 age::gb_memory::read_byte(uint16 address) const
{
    AGE_ASSERT(address < 0xFE00);
    uint offset = get_offset(address);
    uint8 value = m_memory[offset];
    return value;
}

age::uint8 age::gb_memory::read_svbk() const
{
    return m_svbk;
}

age::uint8 age::gb_memory::read_vbk() const
{
    return m_vbk;
}



void age::gb_memory::write_byte(uint16 address, uint8 value)
{
    AGE_ASSERT(address < 0xFE00);

    if (address < 0x8000)
    {
        m_mbc_write(*this, address, value);
    }
    else
    {
        uint offset = get_offset(address);
        m_memory[offset] = value;
    }
}

void age::gb_memory::write_svbk(uint8 value)
{
    m_svbk = value | 0xF8;

    uint bank_id = value & 0x07;
    if (bank_id == 0)
    {
        bank_id = 1;
    }
    BANK_LOG("svbk bank id " << bank_id);

    uint offset = bank_id * gb_internal_ram_bank_size;
    m_offsets[0xD] = m_internal_ram_offset + offset - 0xD000;
    m_offsets[0xF] = m_internal_ram_offset + offset - 0xF000;
}

void age::gb_memory::write_vbk(uint8 value)
{
    m_vbk = value | 0xFE;
    BANK_LOG("svbk bank id " << (uint)(m_vbk & 1));

    uint offset = (m_vbk & 0x01)  * gb_video_ram_bank_size;
    m_offsets[8] = m_video_ram_offset + offset - 0x8000;
    m_offsets[9] = m_offsets[8];
}





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

bool age::gb_memory::has_battery(const uint8_vector &rom)
{
    bool result = false;

    uint8 cart_type = safe_get(rom, gb_cia_ofs_type);
    switch (cart_type)
    {
        case 0x03:
        case 0x06:
        case 0x09:
        case 0x0D:
        case 0x13:
        case 0x1B:
        case 0x1E:
            result = true;
    }

    return result;
}

age::uint age::gb_memory::get_num_cart_rom_banks(const uint8_vector &cart_rom)
{
    uint result;

    uint8 rom_banks = safe_get(cart_rom, gb_cia_ofs_rom_size);
    switch (rom_banks)
    {
        //case 0x00:
        default:   result = 2; break;
        case 0x01: result = 4; break;
        case 0x02: result = 8; break;
        case 0x03: result = 16; break;
        case 0x04: result = 32; break;
        case 0x05: result = 64; break;
        case 0x06: result = 128; break;
        case 0x07: result = 256; break;
        case 0x08: result = 512; break;
        case 0x52: result = 72; break;
        case 0x53: result = 80; break;
        case 0x54: result = 96; break;
    }

    LOG("cartridge has " << result << " rom bank(s)");
    return result;
}

age::uint age::gb_memory::get_num_cart_ram_banks(const uint8_vector &cart_rom)
{
    uint result;

    uint8 ram_banks = safe_get(cart_rom, gb_cia_ofs_ram_size);
    switch (ram_banks)
    {
        //case 0x00:
        default:   result = 0; break;
        case 0x01: result = 1; break; // actually only 2048 bytes, but one whole bank is easier to handle
        case 0x02: result = 1; break;
        case 0x03: result = 4; break;
        case 0x04: result = 16; break;
    }

    LOG("cartridge has " << result << " ram bank(s)");
    return result;
}

age::uint8 age::gb_memory::safe_get(const uint8_vector &vector, uint offset)
{
    return (vector.size() > offset) ? vector[offset] : 0;
}



age::uint age::gb_memory::get_offset(uint16 address) const
{
    uint offset = m_offsets[address >> 12];
    offset += address;
    AGE_ASSERT(offset < m_memory.size());
    return offset;
}

void age::gb_memory::set_rom_bank(uint bank_id)
{
    if (bank_id >= m_num_cart_rom_banks)
    {
        BANK_LOG("adjusting rom bank id " << bank_id << " to " << (bank_id & (m_num_cart_rom_banks - 1)) << " (max is " << (m_num_cart_rom_banks - 1) << ")");
        bank_id &= m_num_cart_rom_banks - 1;
    }

    m_offsets[4] = bank_id * gb_cart_rom_bank_size - 0x4000;
    m_offsets[5] = m_offsets[4];
    m_offsets[6] = m_offsets[4];
    m_offsets[7] = m_offsets[4];
    BANK_LOG("switched to rom bank " << bank_id);
}

void age::gb_memory::set_ram_bank(uint bank_id)
{
    if (bank_id >= m_num_cart_ram_banks)
    {
        BANK_LOG("adjusting ram bank id " << bank_id << " to " << (bank_id & (m_num_cart_ram_banks - 1)) << " (max is " << (m_num_cart_ram_banks - 1) << ")");
        bank_id &= m_num_cart_ram_banks - 1;
    }

    m_offsets[0xA] = m_cart_ram_offset + bank_id * gb_cart_ram_bank_size - 0xA000;
    m_offsets[0xB] = m_offsets[0xA];
    BANK_LOG("switched to ram bank " << bank_id);
}





//---------------------------------------------------------
//
//   private methods: mbc
//
//---------------------------------------------------------

std::function<void(age::gb_memory&, age::uint, age::uint)> age::gb_memory::get_mbc_write_function(gb_mbc_data &mbc, uint8 mbc_type)
{
    std::function<void(gb_memory&, uint, uint)> result;

    switch (mbc_type)
    {
        //case 0x00:
        //case 0x08:
        //case 0x09:
        default:
            result = &write_to_mbc_no_op;
            break;

        case 0x01:
        case 0x02:
        case 0x03:
            mbc.m_mbc1_2000 = 1;
            mbc.m_mbc1_4000 = 0;
            mbc.m_mbc1_mode16_8 = true;
            result = &write_to_mbc1;
            break;

        case 0x05:
        case 0x06:
            result = &write_to_mbc2;
            break;

        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            result = &write_to_mbc3;
            break;

        case 0x19:
        case 0x1A:
        case 0x1B:
            mbc.m_mbc5_2000 = 1;
            mbc.m_mbc5_3000 = 0;
            result = &write_to_mbc5;
            break;

        case 0x1C:
        case 0x1D:
        case 0x1E:
            mbc.m_mbc5_2000 = 1;
            mbc.m_mbc5_3000 = 0;
            result = &write_to_mbc5_rumble;
            break;
    }

    return result;
}



void age::gb_memory::write_to_mbc_no_op(gb_memory&, uint, uint)
{
}



void age::gb_memory::write_to_mbc1(gb_memory &memory, uint offset, uint value)
{
    AGE_ASSERT(offset < 0x8000);

    switch (offset & 0x6000)
    {
        case 0x0000:
            // (de)activate current ram bank
            return;

        case 0x2000:
            // select rom bank (lower 5 bits)
            memory.m_mbc_data.m_mbc1_2000 = ((value & 0x1F) == 0) ? value + 1 : value;
            break;

        case 0x4000:
            // select rom/ram bank
            memory.m_mbc_data.m_mbc1_4000 = value;
            break;

        case 0x6000:
            // select MBC1 mode
            memory.m_mbc_data.m_mbc1_mode16_8 = (value & 0x01) > 0;
            break;
    }

    // update current rom & ram bank for mode 16/8
    uint rom_bank_id;
    uint ram_bank_id;
    if (memory.m_mbc_data.m_mbc1_mode16_8)
    {
        rom_bank_id = (memory.m_mbc_data.m_mbc1_2000 & 0x1F) + ((memory.m_mbc_data.m_mbc1_4000 & 0x03) << 5);
        ram_bank_id = 0;
    }
    // update current rom & ram bank for mode 4/32
    else
    {
        rom_bank_id = memory.m_mbc_data.m_mbc1_2000 & 0x1F;
        ram_bank_id = memory.m_mbc_data.m_mbc1_4000 & 0x03;
    }

    // set rom & ram bank
    AGE_ASSERT((rom_bank_id & 0x1F) > 0);
    memory.set_rom_bank(rom_bank_id);
    memory.set_ram_bank(ram_bank_id);
}



void age::gb_memory::write_to_mbc2(gb_memory &memory, uint offset, uint value)
{
    AGE_ASSERT(offset < 0x8000);

    // switch rom bank
    if ((offset & 0x6000) == 0x2000)
    {
        if ((offset & 0x0100) > 0)
        {
            uint rom_bank_id = value & 0x0F;
            if (rom_bank_id == 0)
            {
                rom_bank_id = 1;
            }
            memory.set_rom_bank(rom_bank_id);
        }
    }
}



void age::gb_memory::write_to_mbc3(gb_memory &memory, uint offset, uint value)
{
    AGE_ASSERT(offset < 0x8000);

    switch (offset & 0x6000)
    {
        case 0x0000:
            // (de)activate current ram bank
            break;

        case 0x2000:
        {
            // select rom bank
            uint rom_bank_id = value & 0x7F;
            if (rom_bank_id == 0)
            {
                rom_bank_id = 1;
            }
            memory.set_rom_bank(rom_bank_id);
            break;
        }

        case 0x4000:
            // select ram bank
            memory.set_ram_bank(value & 0x03);
            break;

        case 0x6000:
            // latch real time clock
            break;
    }
}



void age::gb_memory::write_to_mbc5(gb_memory &memory, uint offset, uint value)
{
    AGE_ASSERT(offset < 0x8000);

    switch (offset & 0x6000)
    {
        case 0x0000:
            // (de)activate current ram bank
            break;

        case 0x2000:
            // select rom bank (lower 5 bits)
            if ((offset & 0x1000) == 0)
            {
                memory.m_mbc_data.m_mbc5_2000 = value;
            }
            else
            {
                memory.m_mbc_data.m_mbc5_3000 = value;
            }
            memory.set_rom_bank((static_cast<uint>(memory.m_mbc_data.m_mbc5_3000 & 0x01) << 8) + memory.m_mbc_data.m_mbc5_2000);
            break;

        case 0x4000:
            // select ram bank
            memory.set_ram_bank(value & 0x0F);
            break;
    }
}



void age::gb_memory::write_to_mbc5_rumble(gb_memory &memory, uint offset, uint value)
{
    AGE_ASSERT(offset < 0x8000);

    // clear rumble motor bit
    if ((offset & 0x6000) == 0x4000)
    {
        value &= 0x07;
    }

    // behave like MBC5
    write_to_mbc5(memory, offset, value);
}
