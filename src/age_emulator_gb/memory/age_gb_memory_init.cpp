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
#include <random>

#include <age_debug.hpp>
#include <age_utilities.hpp>

#include "age_gb_memory.hpp"



namespace
{
    constexpr int gb_video_ram_size = 2 * age::gb_video_ram_bank_size;

    // memory dumps,
    // based on *.bin files used by gambatte tests and gambatte source code (initstate.cpp)

    constexpr const age::uint8_array<200> gb_sparse_vram_0010_dump
        = {{// 0x00 after every value:
            // 0xF0 0x00 0xF0 0x00 0xFC 0x00 0xFC 0x00 ...
            0xF0, 0xF0, 0xFC, 0xFC, 0xFC, 0xFC, 0xF3, 0xF3,
            0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C,
            0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0x00, 0xF3, 0xF3,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCF, 0xCF,
            0x00, 0x00, 0x0F, 0x0F, 0x3F, 0x3F, 0x0F, 0x0F,
            0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x0F, 0x0F,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF3, 0xF3,

            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0,
            0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xFF, 0xFF,
            0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC3, 0xC3,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFC,
            0xF3, 0xF3, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
            0x3C, 0x3C, 0xFC, 0xFC, 0xFC, 0xFC, 0x3C, 0x3C,
            0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3,
            0xF3, 0xF3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3,

            0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF,
            0x3C, 0x3C, 0x3F, 0x3F, 0x3C, 0x3C, 0x0F, 0x0F,
            0x3C, 0x3C, 0xFC, 0xFC, 0x00, 0x00, 0xFC, 0xFC,
            0xFC, 0xFC, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
            0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF0, 0xF0,
            0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF,
            0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xC3, 0xC3,
            0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xFC, 0xFC,

            0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C}};

    // cartridge information area

    //constexpr age::uint16_t gb_header_size = 0x0150;
    //constexpr age::uint16_t gb_cia_ofs_title = 0x0134;
    //constexpr age::uint16_t gb_cia_ofs_licensee_new_low = 0x0144;
    //constexpr age::uint16_t gb_cia_ofs_licensee_new_high = 0x0145;
    //constexpr age::uint16_t gb_cia_ofs_sgb = 0x0146;
    constexpr age::uint16_t gb_cia_ofs_type     = 0x0147;
    constexpr age::uint16_t gb_cia_ofs_rom_size = 0x0148;
    constexpr age::uint16_t gb_cia_ofs_ram_size = 0x0149;
    //constexpr age::uint16_t gb_cia_ofs_destination = 0x014A;
    //constexpr age::uint16_t gb_cia_ofs_licensee = 0x014B;
    //constexpr age::uint16_t gb_cia_ofs_version = 0x014C;
    //constexpr age::uint16_t gb_cia_ofs_header_checksum = 0x014D;
    //constexpr age::uint16_t gb_cia_ofs_global_checksum_low = 0x014E;
    //constexpr age::uint16_t gb_cia_ofs_global_checksum_high = 0x014F;

    age::uint8_t safe_get(const age::uint8_vector& vector, age::uint16_t offset)
    {
        return (vector.size() > offset) ? vector[offset] : 0;
    }

    void set_memory(age::uint8_vector& memory, int offset, age::uint8_t value)
    {
        AGE_ASSERT(offset >= 0)
        AGE_ASSERT(static_cast<unsigned>(offset) < memory.size())
        memory[static_cast<unsigned>(offset)] = value;
    }

    bool has_battery(const age::uint8_vector& rom)
    {
        auto cart_type = safe_get(rom, gb_cia_ofs_type);
        switch (cart_type)
        {
            case 0x03:
            case 0x06:
            case 0x09:
            case 0x0D:
            case 0x13:
            case 0x1B:
            case 0x1E:
            case 0x22: //! \todo really?
                return true;

            default:
                return false;
        }
    }

    bool is_mbc2(const age::uint8_vector& rom)
    {
        auto cart_type = safe_get(rom, gb_cia_ofs_type);
        switch (cart_type)
        {
            case 0x05:
            case 0x06:
                return true;

            default:
                return false;
        }
    }

    age::int16_t get_num_cart_rom_banks(const age::uint8_vector& cart_rom)
    {
        auto rom_banks = safe_get(cart_rom, gb_cia_ofs_rom_size);
        switch (rom_banks)
        {
            //case 0x00:
            default: return 2;
            case 0x01: return 4;
            case 0x02: return 8;
            case 0x03: return 16;
            case 0x04: return 32;
            case 0x05: return 64;
            case 0x06: return 128;
            case 0x07: return 256;
            case 0x08: return 512;
        }
    }

    age::int16_t get_num_cart_ram_banks(const age::uint8_vector& cart_rom)
    {
        if (is_mbc2(cart_rom))
        {
            return 1;
        }
        auto ram_banks = safe_get(cart_rom, gb_cia_ofs_ram_size);
        switch (ram_banks)
        {
            default: return 0;
            case 0x02: return 1;
            case 0x03: return 4;
            case 0x04: return 16;
            case 0x05: return 8;
        }
    }

    //
    // Check if this might be a multi-cart rom.
    //
    // We use the same heuristic as mooneye-gb:
    // If we find the Nintendo logo at least 3 times (menu + 2 games) at specific locations,
    // we flag this rom as multi-cart.
    //
    // Since only 8 MBit multi-cart roms are known,
    // we limit the search to roms of this size.
    //
    bool is_multicart_rom(const age::uint8_vector& rom)
    {
        int findings = 0;

        for (unsigned offset = 0; offset + 0x134 <= rom.size(); offset += 0x40000)
        {
            uint32_t crc = age::crc32(begin(rom) + offset + 0x104, begin(rom) + offset + 0x134);
            if (crc == 0x46195417)
            {
                ++findings;
            }
        }

        return findings >= 3;
    }

} // namespace



//---------------------------------------------------------
//
//   constructor
//
//---------------------------------------------------------

age::gb_memory::gb_memory(const uint8_vector& cart_rom, const gb_clock& clock, bool is_cgb_device)
    : m_clock(clock),
      m_num_cart_rom_banks(get_num_cart_rom_banks(cart_rom)),
      m_num_cart_ram_banks(get_num_cart_ram_banks(cart_rom)),
      m_has_battery(has_battery(cart_rom)),
      m_cart_ram_offset(m_num_cart_rom_banks * gb_cart_rom_bank_size),
      m_work_ram_offset(m_cart_ram_offset + m_num_cart_ram_banks * gb_cart_ram_bank_size),
      m_video_ram_offset(m_work_ram_offset + gb_work_ram_size)
{
    AGE_ASSERT(m_num_cart_rom_banks > 0)
    AGE_ASSERT(m_num_cart_ram_banks >= 0)
    AGE_ASSERT(m_cart_ram_offset > 0)
    AGE_ASSERT(m_work_ram_offset >= m_cart_ram_offset)
    AGE_ASSERT(m_video_ram_offset > m_work_ram_offset)

    switch (safe_get(cart_rom, gb_cia_ofs_type))
    {
        default:
            m_mbc_data       = gb_mbc1_data{.m_bank1 = 1, .m_bank2 = 0, .m_mode1 = false};
            m_mbc_write      = mbc1_write;
            m_cart_ram_write = cart_ram_write;
            m_cart_ram_read  = cart_ram_read;
            m_log_mbc        = "unknown MBC";
            break;

        case 0x00:
        case 0x08:
        case 0x09:
            m_mbc_write      = no_mbc_write;
            m_cart_ram_write = cart_ram_write;
            m_cart_ram_read  = cart_ram_read;
            m_log_mbc        = "no MBC";
            break;

        case 0x01:
        case 0x02:
        case 0x03: {
            bool multicart   = is_multicart_rom(cart_rom);
            m_mbc_data       = gb_mbc1_data{.m_bank1 = 1, .m_bank2 = 0, .m_mode1 = false, .m_multicart = multicart};
            m_mbc_write      = mbc1_write;
            m_cart_ram_write = cart_ram_write;
            m_cart_ram_read  = cart_ram_read;
            m_log_mbc        = multicart ? "MBC1M" : "MBC1";
            break;
        }

        case 0x05:
        case 0x06:
            m_mbc_write      = mbc2_write;
            m_cart_ram_write = mbc2_cart_ram_write;
            m_cart_ram_read  = mbc2_cart_ram_read;
            m_log_mbc        = "MBC2";
            break;

        case 0x0F:
        case 0x10:
            m_mbc_data       = gb_mbc3rtc_data{};
            m_mbc_write      = mbc3rtc_write;
            m_cart_ram_write = mbc3rtc_cart_ram_write;
            m_cart_ram_read  = mbc3rtc_cart_ram_read;
            m_log_mbc        = "MBC3-RTC";
            break;

        case 0x11:
        case 0x12:
        case 0x13:
            m_mbc_write      = mbc3_write;
            m_cart_ram_write = cart_ram_write;
            m_cart_ram_read  = cart_ram_read;
            m_log_mbc        = "MBC3";
            break;

        case 0x19:
        case 0x1A:
        case 0x1B:
            m_mbc_data       = gb_mbc5_data{.m_2000 = 1, .m_3000 = 0};
            m_mbc_write      = mbc5_write;
            m_cart_ram_write = cart_ram_write;
            m_cart_ram_read  = cart_ram_read;
            m_log_mbc        = "MBC5";
            break;

        case 0x1C:
        case 0x1D:
        case 0x1E:
            m_mbc_data       = gb_mbc5_data{.m_2000 = 1, .m_3000 = 0};
            m_mbc_write      = mbc5_rumble_write;
            m_cart_ram_write = cart_ram_write;
            m_cart_ram_read  = cart_ram_read;
            m_log_mbc        = "MBC5-rumble";
            break;

        case 0x22:
            m_mbc_write      = mbc7_write;
            m_cart_ram_write = mbc7_cart_ram_write;
            m_cart_ram_read  = mbc7_cart_ram_read;
            m_log_mbc        = "MBC7";
            break;
    }

    // 0x0000 - 0x3FFF : rom bank 0
    // 0x4000 - 0x7FFF : switchable rom bank
    set_rom_banks(0, 1);
    // 0x8000 - 0x9FFF : video ram
    write_vbk(0);
    // 0xA000 - 0xBFFF : switchable ram bank
    set_ram_bank(0);
    // 0xC000 - 0xDFFF : work ram
    // 0xE000 - 0xFE00 : work ram echo
    m_offsets[0xC] = m_work_ram_offset - 0xC000;
    m_offsets[0xE] = m_work_ram_offset - 0xE000;
    write_svbk(0);

    // allocate memory
    int cart_rom_size = m_num_cart_rom_banks * gb_cart_rom_bank_size;
    int cart_ram_size = m_num_cart_ram_banks * gb_cart_ram_bank_size;
    int memory_size   = cart_rom_size + cart_ram_size + gb_work_ram_size + gb_video_ram_size;
    AGE_ASSERT(memory_size > 0)

    log() << "allocating " << memory_size << " bytes total";
    m_memory = uint8_vector(static_cast<unsigned>(memory_size), 0);

    // copy rom
    int copy_rom_bytes = std::min(cart_rom_size, static_cast<int>(cart_rom.size()));
    log() << "copying " << copy_rom_bytes << " bytes of cartridge rom (rom size is " << cart_rom.size() << " bytes)";
    std::copy(begin(cart_rom), begin(cart_rom) + copy_rom_bytes, begin(m_memory));

    // init vram
    for (uint16_t i = 0, end = gb_sparse_vram_0010_dump.size(); i < end; ++i)
    {
        set_memory(m_memory, m_video_ram_offset + 0x10 + i * 2, gb_sparse_vram_0010_dump[i]);
    }
    if (!is_cgb_device)
    {
        set_memory(m_memory, m_video_ram_offset + 0x1910, 0x19);
        for (uint8_t i = 1; i <= 0x0C; ++i)
        {
            set_memory(m_memory, m_video_ram_offset + 0x1903 + i, i);
            set_memory(m_memory, m_video_ram_offset + 0x1923 + i, i + 0x0C);
        }
    }

    // randomize wram
    std::random_device                      device;
    std::mt19937                            generator(device());
    std::uniform_int_distribution<unsigned> distribution(0, 255);

    auto rng = [&distribution, &generator]() {
        return distribution(generator);
    };

    std::generate(begin(m_memory) + m_work_ram_offset,
                  begin(m_memory) + m_work_ram_offset + gb_work_ram_size,
                  rng);

    // log memory info
    log() << "cartridge:"
          << "\n    * type: " << m_log_mbc << ", " << log_hex8(safe_get(cart_rom, gb_cia_ofs_type))
          << "\n    * cgb compatibility: " << log_hex8(safe_get(cart_rom, 0x143))
          << "\n    * has " << m_num_cart_rom_banks << " rom bank(s): " << log_hex8(safe_get(cart_rom, gb_cia_ofs_rom_size))
          << "\n    * has " << m_num_cart_ram_banks << " ram bank(s): " << log_hex8(safe_get(cart_rom, gb_cia_ofs_ram_size))
          << "\n    * has " << (m_has_battery ? "a" : "no") << " battery: " << log_hex8(safe_get(cart_rom, gb_cia_ofs_type));
}
