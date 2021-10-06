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

#include <algorithm>

#include <age_debug.hpp>

#include "age_gb_memory.hpp"

namespace
{
    constexpr age::uint16_t gb_cia_ofs_title = 0x0134;

    uint16_t mbc2_ram_address(uint16_t address)
    {
        uint16_t ram_offset = address - 0xA000;
        return 0xA000 + (ram_offset & 0x1FF);
    }

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
    const char* buffer = reinterpret_cast<const char*>(&m_memory[gb_cia_ofs_title]);
    std::string result = {buffer, 16};
    return result;
}



age::uint8_vector age::gb_memory::get_persistent_ram() const
{
    uint8_vector result;

    if (m_has_battery)
    {
        int cart_ram_size = m_num_cart_ram_banks * gb_cart_ram_bank_size;

        AGE_ASSERT(cart_ram_size > 0)
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
    if (m_has_battery)
    {
        AGE_ASSERT(source.size() <= int_max)
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



age::uint8_t age::gb_memory::read_byte(uint16_t address) const
{
    AGE_ASSERT(address < 0xFE00)
    // rom & internal ram
    if (!is_cartridge_ram(address))
    {
        return m_memory[get_offset(address)];
    }
    // cartridge ram not readable
    if (!m_cart_ram_enabled)
    {
        log() << "read [" << log_hex16(address) << "] = 0xFF: cartridge RAM disabled";
        return 0xFF;
    }
    // read MBC2 ram
    if (m_is_mbc2)
    {
        return m_memory[get_offset(mbc2_ram_address(address))] | 0xF0;
    }
    // read regular cartridge ram
    return m_memory[get_offset(address)];
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
    AGE_ASSERT(address < 0xFE00)
    // access MBC
    if (address < 0x8000)
    {
        m_mbc_writer(*this, address, value);
        return;
    }
    // write internal ram
    if (!is_cartridge_ram(address))
    {
        m_memory[get_offset(address)] = value;
        return;
    }
    // cartridge ram not writable
    if (!m_cart_ram_enabled)
    {
        log() << "write [" << log_hex16(address) << "] = " << log_hex8(value) << " ignored: cartridge RAM disabled";
        return;
    }
    // write MBC2 ram
    if (m_is_mbc2)
    {
        m_memory[get_offset(mbc2_ram_address(address))] = value | 0xF0;
    }
    // write regular cartridge ram
    m_memory[get_offset(address)] = value;
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

    int offset     = bank_id * gb_internal_ram_bank_size;
    m_offsets[0xD] = m_internal_ram_offset + offset - 0xD000;
    m_offsets[0xF] = m_internal_ram_offset + offset - 0xF000;
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





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

unsigned age::gb_memory::get_offset(uint16_t address) const
{
    auto offset = m_offsets[address >> 12];
    offset += address;

    AGE_ASSERT(offset >= 0)
    AGE_ASSERT(static_cast<unsigned>(offset) < m_memory.size())

    return static_cast<unsigned>(offset);
}

void age::gb_memory::set_ram_accessible(uint8_t value)
{
    m_cart_ram_enabled = ((value & 0x0F) == 0x0A) && (m_num_cart_ram_banks > 0);
    log() << "enable cartridge ram = " << log_hex8(value)
          << " (cartridge ram " << (m_cart_ram_enabled ? "enabled" : "disabled") << ")";
}

void age::gb_memory::set_rom_banks(int low_bank_id, int high_bank_id)
{
    AGE_ASSERT(low_bank_id >= 0)
    AGE_ASSERT(high_bank_id >= 0)
    AGE_ASSERT(m_num_cart_rom_banks >= 0)

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

    log() << "switch to rom banks " << log_hex(low_bank_id) << " @ 0x0000-0x3FFF, "
          << log_hex(high_bank_id) << " @ 0x4000-0x7FFF";
}

void age::gb_memory::set_ram_bank(int bank_id)
{
    AGE_ASSERT(bank_id >= 0)

    bank_id &= m_num_cart_ram_banks - 1;

    m_offsets[0xA] = m_cart_ram_offset + bank_id * gb_cart_ram_bank_size - 0xA000;
    m_offsets[0xB] = m_offsets[0xA];

    log() << "switch to ram bank " << log_hex(bank_id);
}





//---------------------------------------------------------
//
//   private methods: mbc
//
//---------------------------------------------------------

age::gb_memory::mbc_writer age::gb_memory::get_mbc_writer(gb_mbc_data& mbc, uint8_t mbc_type)
{
    mbc_writer result;

    switch (mbc_type)
    {
        //case 0x00:
        //case 0x08:
        //case 0x09:
        default:
            return write_to_mbc_no_op;

        case 0x01:
        case 0x02:
        case 0x03:
            mbc = gb_mbc1_data{.m_bank1 = 1, .m_bank2 = 0, .m_mode1 = false};
            return write_to_mbc1;

        case 0x05:
        case 0x06:
            return write_to_mbc2;

        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            return write_to_mbc3;

        case 0x19:
        case 0x1A:
        case 0x1B:
            mbc = gb_mbc5_data{.m_2000 = 1, .m_3000 = 0};
            return write_to_mbc5;

        case 0x1C:
        case 0x1D:
        case 0x1E:
            mbc = gb_mbc5_data{.m_2000 = 1, .m_3000 = 0};
            return write_to_mbc5_rumble;
    }
}



void age::gb_memory::write_to_mbc_no_op(gb_memory& memory, uint16_t offset, uint8_t value)
{
    AGE_UNUSED(memory);
    AGE_UNUSED(offset);
    AGE_UNUSED(value);
    memory.log() << "write to [" << log_hex16(offset) << "] = " << log_hex8(value) << " ignored: no MBC configured";
}



void age::gb_memory::write_to_mbc1(gb_memory& memory, uint16_t offset, uint8_t value)
{
    AGE_ASSERT(offset < 0x8000)

    // workaround for older STL implementations
    // (we actually want to use std::get<gb_mbc1_data> here ...)
    auto* p_mbc_data = std::get_if<gb_mbc1_data>(&memory.m_mbc_data);
    auto& mbc_data   = *p_mbc_data;

    switch (offset & 0x6000)
    {
        case 0x0000:
            // (de)activate ram
            memory.set_ram_accessible(value);
            return;

        case 0x2000:
            // select rom bank (lower 5 bits)
            mbc_data.m_bank1 = ((value & 0x1F) == 0) ? value + 1 : value;
            break;

        case 0x4000:
            // select rom/ram bank
            mbc_data.m_bank2 = value;
            break;

        case 0x6000:
            // select MBC1 mode:
            //  - mode 0: bank2 affects 0x4000-0x7FFF
            //  - mode 1: bank2 affects 0x0000-0x7FFF, 0xA000-0xBFFF
            mbc_data.m_mode1 = (value & 0x01) > 0;
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

    int mbc_high_bits = mbc_data.m_bank2 & 0x03;

    int high_rom_bits    = mbc_high_bits << (memory.m_mbc1_multi_cart ? 4 : 5);
    int low_rom_bank_id  = mbc_data.m_mode1 ? high_rom_bits : 0;
    int high_rom_bank_id = high_rom_bits + (mbc_data.m_bank1 & (memory.m_mbc1_multi_cart ? 0x0FU : 0x1FU));

    int ram_bank_id = mbc_data.m_mode1 ? mbc_high_bits : 0;

    // set rom & ram banks
    AGE_ASSERT(memory.m_mbc1_multi_cart || ((high_rom_bank_id & 0x1F) > 0))
    memory.set_rom_banks(low_rom_bank_id, high_rom_bank_id);
    memory.set_ram_bank(ram_bank_id);
}



void age::gb_memory::write_to_mbc2(gb_memory& memory, uint16_t offset, uint8_t value)
{
    AGE_ASSERT(offset < 0x8000)

    // writes to $4000-$7FFF have no effect
    if (offset >= 0x4000)
    {
        return;
    }

    // (de)activate ram
    if ((offset & 0x100) == 0)
    {
        memory.set_ram_accessible(value);
    }

    // switch rom bank
    else
    {
        int rom_bank_id = value & 0x0F;
        memory.set_rom_banks(0, (rom_bank_id == 0) ? 1 : rom_bank_id);
    }
}



void age::gb_memory::write_to_mbc3(gb_memory& memory, uint16_t offset, uint8_t value)
{
    AGE_ASSERT(offset < 0x8000)

    switch (offset & 0x6000)
    {
        case 0x0000:
            // (de)activate ram
            memory.set_ram_accessible(value);
            break;

        case 0x2000: {
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



void age::gb_memory::write_to_mbc5(gb_memory& memory, uint16_t offset, uint8_t value)
{
    AGE_ASSERT(offset < 0x8000)

    // workaround for older STL implementations
    // (we actually want to use std::get<gb_mbc1_data> here ...)
    auto* p_mbc_data = std::get_if<gb_mbc5_data>(&memory.m_mbc_data);
    auto& mbc_data   = *p_mbc_data;

    switch (offset & 0x6000)
    {
        case 0x0000:
            // (de)activate ram
            memory.set_ram_accessible(value);
            break;

        case 0x2000:
            // select rom bank (lower 5 bits)
            if ((offset & 0x1000) == 0)
            {
                mbc_data.m_2000 = value;
            }
            else
            {
                mbc_data.m_3000 = value;
            }
            memory.set_rom_banks(0, ((mbc_data.m_3000 & 0x01) << 8) + mbc_data.m_2000);
            break;

        case 0x4000:
            // select ram bank
            memory.set_ram_bank(value & 0x0F);
            break;
    }
}



void age::gb_memory::write_to_mbc5_rumble(gb_memory& memory, uint16_t offset, uint8_t value)
{
    AGE_ASSERT(offset < 0x8000)

    // clear rumble motor bit
    if ((offset & 0x6000) == 0x4000)
    {
        value &= 0x07;
    }

    // behave like MBC5
    write_to_mbc5(memory, offset, value);
}
