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

#ifndef AGE_GB_MEMORY_HPP
#define AGE_GB_MEMORY_HPP

//!
//! \file
//!

#include <functional>

#include <age_non_copyable.hpp>

#include "age_gb.hpp"



namespace age
{

class gb_memory : public non_copyable
{
public:

    bool is_cgb() const;
    const uint8* get_video_ram() const;
    std::string get_cartridge_title() const;

    uint8_vector get_persistent_ram() const;
    void set_persistent_ram(const uint8_vector &source);

    uint8 read_byte(uint16 address) const;
    uint8 read_svbk() const;
    uint8 read_vbk() const;

    void write_byte(uint16 address, uint8 value);
    void write_svbk(uint8 value);
    void write_vbk(uint8 value);

    gb_memory(const uint8_vector &cart_rom, bool force_dmg);



private:

    typedef union {
        struct {
            uint m_mbc1_2000;
            uint m_mbc1_4000;
            bool m_mbc1_mode16_8;
        };
        struct {
            uint m_mbc5_2000;
            uint m_mbc5_3000;
        };
    } gb_mbc_data;

    static bool has_battery(const uint8_vector &cart_rom);
    static uint get_num_cart_rom_banks(const uint8_vector &cart_rom);
    static uint get_num_cart_ram_banks(const uint8_vector &cart_rom);
    static uint8 safe_get(const uint8_vector &vector, uint offset);

    std::function<void(gb_memory&, uint, uint)> get_mbc_write_function(gb_mbc_data &mbc, uint8 mbc_type);
    static void write_to_mbc_no_op(gb_memory &memory, uint offset, uint value);
    static void write_to_mbc1(gb_memory &memory, uint offset, uint value);
    static void write_to_mbc2(gb_memory &memory, uint offset, uint value);
    static void write_to_mbc3(gb_memory &memory, uint offset, uint value);
    static void write_to_mbc5(gb_memory &memory, uint offset, uint value);
    static void write_to_mbc5_rumble(gb_memory &memory, uint offset, uint value);



    uint get_offset(uint16 address) const;
    void set_rom_bank(uint bank_id);
    void set_ram_bank(uint bank_id);

    const uint m_num_cart_rom_banks;
    const uint m_num_cart_ram_banks;
    const bool m_has_battery;
    const bool m_cgb;
    uint8 m_svbk = 0xF8;
    uint8 m_vbk = 0xF8;

    gb_mbc_data m_mbc_data;
    const std::function<void(gb_memory&, uint, uint)> m_mbc_write;

    const uint m_cart_ram_offset;
    const uint m_internal_ram_offset;
    const uint m_video_ram_offset;
    uint8_vector m_memory;
    std::array<uint, 16> m_offsets = {{0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0}};
};

} // namespace age



#endif // AGE_GB_MEMORY_HPP
