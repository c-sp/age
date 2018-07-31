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

#ifndef AGE_GB_MEMORY_HPP
#define AGE_GB_MEMORY_HPP

//!
//! \file
//!

#include <functional>
#include <string>

#include <age_non_copyable.hpp>
#include <age_types.hpp>
#include <emulator/age_gb_types.hpp>



namespace age
{

constexpr uint gb_cart_rom_bank_size = 0x4000;
constexpr uint gb_cart_ram_bank_size = 0x2000;
constexpr uint gb_internal_ram_bank_size = 0x1000;
constexpr uint gb_video_ram_bank_size = 0x2000;

constexpr uint gb_internal_ram_size = 8 * gb_internal_ram_bank_size;
constexpr uint gb_video_ram_size = 2 * gb_video_ram_bank_size;

// cartridge information area

//constexpr uint gb_header_size = 0x0150;
constexpr uint gb_cia_ofs_title = 0x0134;
constexpr uint gb_cia_ofs_cgb = 0x0143;
//constexpr uint gb_cia_ofs_licensee_new_low = 0x0144;
//constexpr uint gb_cia_ofs_licensee_new_high = 0x0145;
//constexpr uint gb_cia_ofs_sgb = 0x0146;
constexpr uint gb_cia_ofs_type = 0x0147;
constexpr uint gb_cia_ofs_rom_size = 0x0148;
constexpr uint gb_cia_ofs_ram_size = 0x0149;
//constexpr uint gb_cia_ofs_destination = 0x014A;
//constexpr uint gb_cia_ofs_licensee = 0x014B;
//constexpr uint gb_cia_ofs_version = 0x014C;
//constexpr uint gb_cia_ofs_header_checksum = 0x014D;
//constexpr uint gb_cia_ofs_global_checksum_low = 0x014E;
//constexpr uint gb_cia_ofs_global_checksum_high = 0x014F;



class gb_memory : public non_copyable
{
public:

    gb_mode get_mode() const;
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

    gb_memory(const uint8_vector &cart_rom, gb_hardware hardware);



private:

    typedef std::function<void(gb_memory&, uint, uint)> mbc_writer;

    typedef union {
        struct {
            uint m_mbc1_bank1;
            uint m_mbc1_bank2;
            bool m_mbc1_mode1;
        };
        struct {
            uint m_mbc5_2000;
            uint m_mbc5_3000;
        };
    } gb_mbc_data;

    static bool has_battery(const uint8_vector &cart_rom);
    static gb_mode calculate_mode(gb_hardware hardware, const uint8_vector &cart_rom);
    static uint get_num_cart_rom_banks(const uint8_vector &cart_rom);
    static uint get_num_cart_ram_banks(const uint8_vector &cart_rom);
    static uint8 safe_get(const uint8_vector &vector, uint offset);
    static uint32 crc32(uint8_vector::const_iterator begin, uint8_vector::const_iterator end);

    mbc_writer get_mbc_writer(gb_mbc_data &mbc, const uint8_vector &cart_rom);
    static void write_to_mbc_no_op(gb_memory &memory, uint offset, uint value);
    static void write_to_mbc1(gb_memory &memory, uint offset, uint value);
    static void write_to_mbc2(gb_memory &memory, uint offset, uint value);
    static void write_to_mbc3(gb_memory &memory, uint offset, uint value);
    static void write_to_mbc5(gb_memory &memory, uint offset, uint value);
    static void write_to_mbc5_rumble(gb_memory &memory, uint offset, uint value);



    uint get_offset(uint16 address) const;
    void set_rom_banks(uint low_bank_id, uint high_bank_id);
    void set_ram_bank(uint bank_id);

    gb_mbc_data m_mbc_data;
    bool m_mbc1_multi_cart = false;
    bool m_mbc_ram_accessible = false;
    mbc_writer m_mbc_writer;

    const uint m_num_cart_rom_banks;
    const uint m_num_cart_ram_banks;
    const bool m_has_battery;
    const gb_mode m_mode;
    uint8 m_svbk = 0xF8;
    uint8 m_vbk = 0xF8;

    const uint m_cart_ram_offset;
    const uint m_internal_ram_offset;
    const uint m_video_ram_offset;
    uint8_vector m_memory;
    std::array<uint, 16> m_offsets = {{0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0}};
};

} // namespace age



#endif // AGE_GB_MEMORY_HPP
