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
#include <ios> // std::hex

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

#define SET_RAM_ACCESSIBLE(value) memory.m_mbc_ram_accessible = ((value & 0x0F) == 0x0A)
#define IS_MBC_RAM(address) ((address & 0xE000) == 0xA000)





//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

age::gb_mode age::gb_memory::get_mode() const
{
    return m_mode;
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
    uint8 value = (!IS_MBC_RAM(address) || m_mbc_ram_accessible) ? m_memory[get_offset(address)] : 0xFF;
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
        m_mbc_writer(*this, address, value);
    }
    else if (!IS_MBC_RAM(address) || m_mbc_ram_accessible)
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

age::gb_mode age::gb_memory::calculate_mode(age::gb_hardware hardware, const age::uint8_vector &cart_rom)
{
    bool cgb_flagged = safe_get(cart_rom, gb_cia_ofs_cgb) >= 0x80;

    gb_mode mode;
    switch (hardware)
    {
        case gb_hardware::dmg:
            // running a CGB rom on a DMG wil most likely not work,
            // but since it was configured like this ...
            mode = gb_mode::dmg;
            break;

        case gb_hardware::cgb:
            mode = cgb_flagged ? gb_mode::cgb : gb_mode::dmg_on_cgb;
            break;

        default:
            // auto-detect hardware
            mode = cgb_flagged ? gb_mode::cgb : gb_mode::dmg;
            break;
    }
    return mode;
}

age::uint age::gb_memory::get_num_cart_rom_banks(const uint8_vector &cart_rom)
{
    uint result;

    uint8 rom_banks = safe_get(cart_rom, gb_cia_ofs_rom_size);
    LOG("rom banks byte: 0x" << std::hex << (uint)rom_banks << std::dec);
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
    }

    LOG("cartridge has " << result << " rom bank(s)");
    return result;
}

age::uint age::gb_memory::get_num_cart_ram_banks(const uint8_vector &cart_rom)
{
    uint result;

    uint8 ram_banks = safe_get(cart_rom, gb_cia_ofs_ram_size);
    LOG("ram banks byte: 0x" << std::hex << (uint)ram_banks << std::dec);
    switch (ram_banks)
    {
        //case 0x00:
        default:   result = 0; break;
        case 0x01: result = 1; break; // actually only 2048 bytes, but one whole bank is easier to handle
        case 0x02: result = 1; break;
        case 0x03: result = 4; break;
        case 0x04: result = 16; break;
        case 0x05: result = 8; break;
    }

    LOG("cartridge has " << result << " ram bank(s)");
    return result;
}

age::uint8 age::gb_memory::safe_get(const uint8_vector &vector, uint offset)
{
    return (vector.size() > offset) ? vector[offset] : 0;
}

age::uint32 age::gb_memory::crc32(uint8_vector::const_iterator begin, uint8_vector::const_iterator end)
{
    uint32 crc = 0xFFFFFFFF;

    std::for_each(begin, end, [&](auto v)
    {
        crc ^= v;
        for (size_t i = 0; i < 8; ++i)
        {
            crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320 : crc >> 1;
        }
    });

    return ~crc;
}



age::uint age::gb_memory::get_offset(uint16 address) const
{
    uint offset = m_offsets[address >> 12];
    offset += address;
    AGE_ASSERT(offset < m_memory.size());
    return offset;
}

void age::gb_memory::set_rom_banks(uint low_bank_id, uint high_bank_id)
{
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

    BANK_LOG("switched to rom banks " << low_bank_id << "," << high_bank_id
             << " (0x" << std::hex << low_bank_id << ",0x" << high_bank_id << std::dec << ")");
}

void age::gb_memory::set_ram_bank(uint bank_id)
{
    bank_id &= m_num_cart_ram_banks - 1;

    m_offsets[0xA] = m_cart_ram_offset + bank_id * gb_cart_ram_bank_size - 0xA000;
    m_offsets[0xB] = m_offsets[0xA];

    BANK_LOG("switched to ram bank " << bank_id << " (0x" << std::hex << bank_id << std::dec << ")");
}





//---------------------------------------------------------
//
//   private methods: mbc
//
//---------------------------------------------------------

age::gb_memory::mbc_writer age::gb_memory::get_mbc_writer(gb_mbc_data &mbc, const uint8_vector &cart_rom)
{
    mbc_writer result;

    uint8 mbc_type = safe_get(cart_rom, gb_cia_ofs_type);
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
            mbc.m_mbc1_mode_4m_32k = false;
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
            // (de)activate ram
            SET_RAM_ACCESSIBLE(value);
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
            memory.m_mbc_data.m_mbc1_mode_4m_32k = (value & 0x01) > 0;
            break;
    }

    //
    // verified by mooneye-gb tests
    //
    // - The value written to 0x4000 is used for rom bank switching
    //   even if MBC1 has been switched to mode 4M-Rom/32K-Ram.
    // - With MBC1 mode 16M-Rom/8K-Ram the value written to 0x4000
    //   switches the rom bank at 0x0000.
    //
    //      emulator-only/mbc1/rom_8Mb
    //      emulator-only/mbc1/rom_16Mb
    //

    uint mbc_high_bits = memory.m_mbc_data.m_mbc1_4000 & 0x03;

    uint high_rom_bits = mbc_high_bits << (memory.m_mbc1_multi_cart ? 4 : 5);
    uint low_rom_bank_id = memory.m_mbc_data.m_mbc1_mode_4m_32k ? high_rom_bits : 0;
    uint high_rom_bank_id = high_rom_bits + (memory.m_mbc_data.m_mbc1_2000 & (memory.m_mbc1_multi_cart ? 0x0F : 0x1F));

    uint ram_bank_id = memory.m_mbc_data.m_mbc1_mode_4m_32k ? mbc_high_bits : 0;

    // set rom & ram banks
    AGE_ASSERT(memory.m_mbc1_multi_cart || ((high_rom_bank_id & 0x1F) > 0));
    memory.set_rom_banks(low_rom_bank_id, high_rom_bank_id);
    memory.set_ram_bank(ram_bank_id);
}



void age::gb_memory::write_to_mbc2(gb_memory &memory, uint offset, uint value)
{
    AGE_ASSERT(offset < 0x8000);

    // (de)activate ram
    if (offset < 0xFFF)
    {
        SET_RAM_ACCESSIBLE(value);
    }

    // switch rom bank
    else if ((offset & 0x6000) == 0x2000)
    {
        if ((offset & 0x0100) > 0)
        {
            uint rom_bank_id = value & 0x0F;
            memory.set_rom_banks(0, (rom_bank_id == 0) ? 1 : rom_bank_id);
        }
    }
}



void age::gb_memory::write_to_mbc3(gb_memory &memory, uint offset, uint value)
{
    AGE_ASSERT(offset < 0x8000);

    switch (offset & 0x6000)
    {
        case 0x0000:
            // (de)activate ram
            SET_RAM_ACCESSIBLE(value);
            break;

        case 0x2000:
        {
            // select rom bank
            uint rom_bank_id = value & 0x7F;
            if (rom_bank_id == 0)
            {
                rom_bank_id = 1;
            }
            memory.set_rom_banks(0, rom_bank_id);
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
            // (de)activate ram
            SET_RAM_ACCESSIBLE(value);
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
            memory.set_rom_banks(0, (static_cast<uint>(memory.m_mbc_data.m_mbc5_3000 & 0x01) << 8) + memory.m_mbc_data.m_mbc5_2000);
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
