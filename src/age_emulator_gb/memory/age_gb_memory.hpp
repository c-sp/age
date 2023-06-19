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

#include "../common/age_gb_clock.hpp"

#include <age_types.hpp>

#include <functional>
#include <string>
#include <variant>



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
        explicit gb_memory(const uint8_vector& cart_rom, const gb_clock& clock, bool is_cgb_device);
        ~gb_memory() = default;

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

        void update_state();
        void set_back_clock(int clock_cycle_offset);



    private:
        static void    no_mbc_write(gb_memory& memory, uint16_t address, uint8_t value);
        static void    cart_ram_write(gb_memory& memory, uint16_t address, uint8_t value);
        static uint8_t cart_ram_read(gb_memory& memory, uint16_t address);

        struct gb_mbc1_data
        {
            uint8_t m_bank1;
            uint8_t m_bank2;
            bool    m_mode1;
            bool    m_multicart;
        };
        static void mbc1_write(gb_memory& memory, uint16_t address, uint8_t value);

        static void    mbc2_write(gb_memory& memory, uint16_t address, uint8_t value);
        static void    mbc2_cart_ram_write(gb_memory& memory, uint16_t address, uint8_t value);
        static uint8_t mbc2_cart_ram_read(gb_memory& memory, uint16_t address);

        struct gb_mbc3rtc_data
        {
            uint8_array<5> m_rtc_reg{};
            uint8_array<5> m_rtc_reg_latched{};
            int            m_clks_last_update   = 0;
            int            m_clks_sec_remainder = 0;
            uint8_t        m_mapped_rtc_reg     = 0; //!< the current RTC register mapped to 0xA000
            uint8_t        m_last_latch_write   = 0;
        };
        static void    mbc3_write(gb_memory& memory, uint16_t address, uint8_t value);
        static void    mbc3rtc_write(gb_memory& memory, uint16_t address, uint8_t value);
        static void    mbc3rtc_cart_ram_write(gb_memory& memory, uint16_t address, uint8_t value);
        static uint8_t mbc3rtc_cart_ram_read(gb_memory& memory, uint16_t address);
        static void    mbc3rtc_update(gb_memory& memory);

        struct gb_mbc5_data
        {
            uint8_t m_2000;
            uint8_t m_3000;
        };
        static void mbc5_write(gb_memory& memory, uint16_t address, uint8_t value);
        static void mbc5_rumble_write(gb_memory& memory, uint16_t address, uint8_t value);

        static void    mbc7_write(gb_memory& memory, uint16_t address, uint8_t value);
        static void    mbc7_cart_ram_write(gb_memory& memory, uint16_t address, uint8_t value);
        static uint8_t mbc7_cart_ram_read(gb_memory& memory, uint16_t address);



        // logging code is header-only to allow for compile time optimization
        [[nodiscard]] gb_log_message_stream log() const
        {
            return m_clock.log(gb_log_category::lc_memory);
        }
        [[nodiscard]] gb_log_message_stream log_mbc() const
        {
            auto msg = log();
            if (m_log_mbc)
            {
                msg << "(" << m_log_mbc << ") ";
            }
            return msg;
        }

        template<typename T>
        T& get_mbc_data()
        {
            // workaround for older STL implementations
            // (we actually want to use std::get<gb_mbc1_data> here ...)
            auto* p_mbc_data = std::get_if<T>(&m_mbc_data);
            return *p_mbc_data;
        }

        [[nodiscard]] unsigned get_offset(uint16_t address) const;
        void                   set_cart_ram_enabled(uint8_t value);
        void                   set_rom_banks(int low_bank_id, int high_bank_id);
        void                   set_ram_bank(int bank_id);



        using gb_mbc_data      = std::variant<gb_mbc1_data, gb_mbc3rtc_data, gb_mbc5_data>;
        using gb_fn_write_byte = std::function<void(gb_memory&, uint16_t, uint8_t)>;
        using gb_fn_read_byte  = std::function<uint8_t(gb_memory&, uint16_t)>;

        const gb_clock&  m_clock;
        const char*      m_log_mbc = nullptr; //!< used only for logging
        gb_mbc_data      m_mbc_data;
        gb_fn_write_byte m_mbc_write;
        gb_fn_write_byte m_cart_ram_write;
        gb_fn_read_byte  m_cart_ram_read;

        const int16_t m_num_cart_rom_banks;
        const int16_t m_num_cart_ram_banks;
        const bool    m_has_battery;
        bool          m_cart_ram_enabled = false; //!< also used as "feature enabled" e.g. for MBC3-RTC
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
