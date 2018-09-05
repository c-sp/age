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

#define RAM_ACCESSIBLE(value) ((value & 0x0F) == 0x0A)
#define IS_MBC_RAM(address) ((address & 0xE000) == 0xA000)

namespace
{

constexpr age::uint16_t gb_cia_ofs_title = 0x0134;

}





//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

age::gb_mode age::gb_memory::get_mode() const
{
    return m_mode;
}

const age::uint8_t* age::gb_memory::get_video_ram() const
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
        int cart_ram_size = m_num_cart_ram_banks * gb_cart_ram_bank_size;

        AGE_ASSERT(cart_ram_size > 0);
        result.resize(static_cast<unsigned>(cart_ram_size));

        std::copy(begin(m_memory) + m_cart_ram_offset,
                  begin(m_memory) + m_cart_ram_offset + cart_ram_size,
                  begin(result));
    }
    else
    {
        result = uint8_vector();
    }

    LOG("returning persistent ram of size " << result.size());
    return result;
}

void age::gb_memory::set_persistent_ram(const uint8_vector &source)
{
    if (m_has_battery)
    {
        AGE_ASSERT(source.size() <= int_max);
        int source_size = static_cast<int>(source.size());
        int cart_ram_size = m_num_cart_ram_banks * gb_cart_ram_bank_size;

        // copy persistent ram
        int bytes_to_copy = std::min(cart_ram_size, source_size);
        LOG("copying " << bytes_to_copy << " bytes into the persistent cartridge ram");

        std::copy(begin(source),
                  begin(source) + bytes_to_copy,
                  begin(m_memory) + m_cart_ram_offset);

        // fill up with zeroes, if the source vector is smaller than the actual persistent ram
        if (cart_ram_size > source_size)
        {
            LOG("filling up " << (cart_ram_size - source_size) << " bytes of persistent cartridge ram with zeroes");

            std::fill(begin(m_memory) + m_cart_ram_offset + source_size,
                      begin(m_memory) + m_cart_ram_offset + cart_ram_size,
                      0);
        }
    }
}



age::uint8_t age::gb_memory::read_byte(uint16_t address) const
{
    AGE_ASSERT(address < 0xFE00);
    uint8_t value = (!IS_MBC_RAM(address) || m_mbc_ram_accessible) ? m_memory[get_offset(address)] : 0xFF;
    return value;
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
    AGE_ASSERT(address < 0xFE00);

    if (address < 0x8000)
    {
        m_mbc_writer(*this, address, value);
    }
    else if (!IS_MBC_RAM(address) || m_mbc_ram_accessible)
    {
        unsigned offset = get_offset(address);
        m_memory[offset] = value;
    }
}

void age::gb_memory::write_svbk(uint8_t value)
{
    m_svbk = value | 0xF8;

    int bank_id = value & 0x07;
    if (bank_id == 0)
    {
        bank_id = 1;
    }
    BANK_LOG("svbk bank id " << bank_id);

    int offset = bank_id * gb_internal_ram_bank_size;
    m_offsets[0xD] = m_internal_ram_offset + offset - 0xD000;
    m_offsets[0xF] = m_internal_ram_offset + offset - 0xF000;
}

void age::gb_memory::write_vbk(uint8_t value)
{
    m_vbk = value | 0xFE;
    BANK_LOG("svbk bank id " << AGE_LOG_DEC(m_vbk & 1));

    int offset = (m_vbk & 0x01) * gb_video_ram_bank_size;
    m_offsets[8] = m_video_ram_offset + offset - 0x8000;
    m_offsets[9] = m_offsets[8];
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

    AGE_ASSERT(offset >= 0);
    AGE_ASSERT(static_cast<unsigned>(offset) < m_memory.size());

    return static_cast<unsigned>(offset);
}

void age::gb_memory::set_rom_banks(int low_bank_id, int high_bank_id)
{
    AGE_ASSERT(low_bank_id >= 0);
    AGE_ASSERT(high_bank_id >= 0);
    AGE_ASSERT(m_num_cart_rom_banks >= 0);

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

    BANK_LOG("switched to rom banks " << AGE_LOG_HEX(low_bank_id) << ", " << AGE_LOG_HEX(high_bank_id));
}

void age::gb_memory::set_ram_bank(int bank_id)
{
    AGE_ASSERT(bank_id >= 0);

    bank_id &= m_num_cart_ram_banks - 1;

    m_offsets[0xA] = m_cart_ram_offset + bank_id * gb_cart_ram_bank_size - 0xA000;
    m_offsets[0xB] = m_offsets[0xA];

    BANK_LOG("switched to ram bank " << AGE_LOG_HEX(bank_id));
}





//---------------------------------------------------------
//
//   private methods: mbc
//
//---------------------------------------------------------

age::gb_memory::mbc_writer age::gb_memory::get_mbc_writer(gb_mbc_data &mbc, uint8_t mbc_type)
{
    mbc_writer result;

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
            mbc.m_mbc1.m_bank1 = 1;
            mbc.m_mbc1.m_bank2 = 0;
            mbc.m_mbc1.m_mode1 = false;
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
            mbc.m_mbc5.m_2000 = 1;
            mbc.m_mbc5.m_3000 = 0;
            result = &write_to_mbc5;
            break;

        case 0x1C:
        case 0x1D:
        case 0x1E:
            mbc.m_mbc5.m_2000 = 1;
            mbc.m_mbc5.m_3000 = 0;
            result = &write_to_mbc5_rumble;
            break;
    }

    return result;
}



void age::gb_memory::write_to_mbc_no_op(gb_memory&, uint16_t, uint8_t)
{
}



void age::gb_memory::write_to_mbc1(gb_memory &memory, uint16_t offset, uint8_t value)
{
    AGE_ASSERT(offset < 0x8000);

    switch (offset & 0x6000)
    {
        case 0x0000:
            // (de)activate ram
            memory.m_mbc_ram_accessible = RAM_ACCESSIBLE(value);
            return;

        case 0x2000:
            // select rom bank (lower 5 bits)
            memory.m_mbc_data.m_mbc1.m_bank1 = ((value & 0x1F) == 0) ? value + 1 : value;
            break;

        case 0x4000:
            // select rom/ram bank
            memory.m_mbc_data.m_mbc1.m_bank2 = value;
            break;

        case 0x6000:
            // select MBC1 mode:
            //  - mode 0: bank2 affects 0x4000-0x7FFF
            //  - mode 1: bank2 affects 0x0000-0x7FFF, 0xA000-0xBFFF
            memory.m_mbc_data.m_mbc1.m_mode1 = (value & 0x01) > 0;
            break;
    }

    //
    // verified by mooneye-gb tests
    //
    // - The value written to 0x4000 is used for rom bank switching
    //   independent of the MBC1 mode.
    // - With MBC1 mode 1 the value written to 0x4000 switches the
    //   rom bank at 0x0000.
    //
    //      emulator-only/mbc1/rom_8Mb
    //      emulator-only/mbc1/rom_16Mb
    //

    int mbc_high_bits = memory.m_mbc_data.m_mbc1.m_bank2 & 0x03;

    int high_rom_bits = mbc_high_bits << (memory.m_mbc1_multi_cart ? 4 : 5);
    int low_rom_bank_id = memory.m_mbc_data.m_mbc1.m_mode1 ? high_rom_bits : 0;
    int high_rom_bank_id = high_rom_bits + (memory.m_mbc_data.m_mbc1.m_bank1 & (memory.m_mbc1_multi_cart ? 0x0F : 0x1F));

    int ram_bank_id = memory.m_mbc_data.m_mbc1.m_mode1 ? mbc_high_bits : 0;

    // set rom & ram banks
    AGE_ASSERT(memory.m_mbc1_multi_cart || ((high_rom_bank_id & 0x1F) > 0));
    memory.set_rom_banks(low_rom_bank_id, high_rom_bank_id);
    memory.set_ram_bank(ram_bank_id);
}



void age::gb_memory::write_to_mbc2(gb_memory &memory, uint16_t offset, uint8_t value)
{
    AGE_ASSERT(offset < 0x8000);

    // (de)activate ram
    if (offset < 0xFFF)
    {
        memory.m_mbc_ram_accessible = RAM_ACCESSIBLE(value);
    }

    // switch rom bank
    else if ((offset & 0x6000) == 0x2000)
    {
        if ((offset & 0x0100) > 0)
        {
            int rom_bank_id = value & 0x0F;
            memory.set_rom_banks(0, (rom_bank_id == 0) ? 1 : rom_bank_id);
        }
    }
}



void age::gb_memory::write_to_mbc3(gb_memory &memory, uint16_t offset, uint8_t value)
{
    AGE_ASSERT(offset < 0x8000);

    switch (offset & 0x6000)
    {
        case 0x0000:
            // (de)activate ram
            memory.m_mbc_ram_accessible = RAM_ACCESSIBLE(value);
            break;

        case 0x2000:
        {
            // select rom bank
            int rom_bank_id = value & 0x7F;
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



void age::gb_memory::write_to_mbc5(gb_memory &memory, uint16_t offset, uint8_t value)
{
    AGE_ASSERT(offset < 0x8000);

    switch (offset & 0x6000)
    {
        case 0x0000:
            // (de)activate ram
            memory.m_mbc_ram_accessible = RAM_ACCESSIBLE(value);
            break;

        case 0x2000:
            // select rom bank (lower 5 bits)
            if ((offset & 0x1000) == 0)
            {
                memory.m_mbc_data.m_mbc5.m_2000 = value;
            }
            else
            {
                memory.m_mbc_data.m_mbc5.m_3000 = value;
            }
            memory.set_rom_banks(0, ((memory.m_mbc_data.m_mbc5.m_3000 & 0x01) << 8) + memory.m_mbc_data.m_mbc5.m_2000);
            break;

        case 0x4000:
            // select ram bank
            memory.set_ram_bank(value & 0x0F);
            break;
    }
}



void age::gb_memory::write_to_mbc5_rumble(gb_memory &memory, uint16_t offset, uint8_t value)
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
