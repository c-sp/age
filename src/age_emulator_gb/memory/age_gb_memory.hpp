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

#ifndef AGE_GB_MEMORY_HPP
#define AGE_GB_MEMORY_HPP

//!
//! \file
//!

#include <functional>
#include <string>
#include <variant>

#include <age_types.hpp>

#include "../common/age_gb_clock.hpp"



namespace age
{

    constexpr int gb_cart_rom_bank_size  = 0x4000;
    constexpr int gb_cart_ram_bank_size  = 0x2000;
    constexpr int gb_work_ram_bank_size  = 0x1000;
    constexpr int gb_work_ram_size       = 8 * age::gb_work_ram_bank_size;
    constexpr int gb_video_ram_bank_size = 0x2000;



    class gb_memory
    {
        AGE_DISABLE_COPY(gb_memory);
        AGE_DISABLE_MOVE(gb_memory);

    public:
        explicit gb_memory(const uint8_vector& cart_rom, const gb_clock& clock);
        ~gb_memory() = default;

        void init_vram(bool for_cgb_device);

        [[nodiscard]] const uint8_t* get_video_ram() const;
        [[nodiscard]] const uint8_t* get_rom_header() const;
        [[nodiscard]] std::string    get_cartridge_title() const;

        [[nodiscard]] uint8_vector get_persistent_ram() const;
        void                       set_persistent_ram(const uint8_vector& source);

        [[nodiscard]] uint8_t read_byte(uint16_t address);
        [[nodiscard]] uint8_t read_svbk() const;
        [[nodiscard]] uint8_t read_vbk() const;

        void write_byte(uint16_t address, uint8_t value);
        void write_svbk(uint8_t value);
        void write_vbk(uint8_t value);



    private:
        static void    no_mbc_write(gb_memory& memory, uint16_t address, uint8_t value);
        static void    cart_ram_write(gb_memory& memory, uint16_t address, uint8_t value);
        static uint8_t cart_ram_read(gb_memory& memory, uint16_t address);

        struct gb_mbc1_data
        {
            uint8_t m_bank1;
            uint8_t m_bank2;
            bool    m_mode1;
        };
        static void mbc1_write(gb_memory& memory, uint16_t address, uint8_t value);

        static void    mbc2_write(gb_memory& memory, uint16_t address, uint8_t value);
        static void    mbc2_cart_ram_write(gb_memory& memory, uint16_t address, uint8_t value);
        static uint8_t mbc2_cart_ram_read(gb_memory& memory, uint16_t address);

        static void mbc3_write(gb_memory& memory, uint16_t address, uint8_t value);

        struct gb_mbc5_data
        {
            uint8_t m_2000;
            uint8_t m_3000;
        };
        static void mbc5_write(gb_memory& memory, uint16_t address, uint8_t value);
        static void mbc5_rumble_write(gb_memory& memory, uint16_t address, uint8_t value);



        [[nodiscard]] unsigned get_offset(uint16_t address) const;
        void                   set_ram_accessible(uint8_t value);
        void                   set_rom_banks(int low_bank_id, int high_bank_id);
        void                   set_ram_bank(int bank_id);

        // logging code is header-only to allow for compile time optimization
        [[nodiscard]] gb_log_message_stream log() const
        {
            return m_clock.log(gb_log_category::lc_memory);
        }

        using gb_mbc_data = std::variant<gb_mbc1_data, gb_mbc5_data>;
        using gb_fn_write = std::function<void(gb_memory&, uint16_t, uint8_t)>;
        using gb_fn_read  = std::function<uint8_t(gb_memory&, uint16_t)>;

        const gb_clock& m_clock; //!< used only for logging
        gb_mbc_data     m_mbc_data;
        gb_fn_write     m_mbc_write;
        gb_fn_write     m_cart_ram_write;
        gb_fn_read      m_cart_ram_read;

        const int16_t m_num_cart_rom_banks;
        const int16_t m_num_cart_ram_banks;
        const bool    m_has_battery;
        bool          m_mbc1_multi_cart  = false;
        bool          m_cart_ram_enabled = false;
        uint8_t       m_svbk             = 0xF8;
        uint8_t       m_vbk              = 0xF8;

        const int           m_cart_ram_offset;
        const int           m_work_ram_offset;
        const int           m_video_ram_offset;
        uint8_vector        m_memory;
        std::array<int, 16> m_offsets{};
    };

} // namespace age



#endif // AGE_GB_MEMORY_HPP
