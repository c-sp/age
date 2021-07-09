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

#include "common/age_gb_clock.hpp"



namespace age
{

    constexpr int gb_cart_rom_bank_size     = 0x4000;
    constexpr int gb_cart_ram_bank_size     = 0x2000;
    constexpr int gb_internal_ram_bank_size = 0x1000;
    constexpr int gb_video_ram_bank_size    = 0x2000;



    class gb_memory
    {
        AGE_DISABLE_COPY(gb_memory);
        AGE_DISABLE_MOVE(gb_memory);

    public:
        explicit gb_memory(const uint8_vector& cart_rom, const gb_clock& clock);
        ~gb_memory() = default;

        void init_vram(bool for_cgb_hardware);

        [[nodiscard]] const uint8_t* get_video_ram() const;
        [[nodiscard]] const uint8_t* get_rom_header() const;
        [[nodiscard]] std::string    get_cartridge_title() const;

        [[nodiscard]] uint8_vector get_persistent_ram() const;
        void                       set_persistent_ram(const uint8_vector& source);

        [[nodiscard]] uint8_t read_byte(uint16_t address) const;
        [[nodiscard]] uint8_t read_svbk() const;
        [[nodiscard]] uint8_t read_vbk() const;

        void write_byte(uint16_t address, uint8_t value);
        void write_svbk(uint8_t value);
        void write_vbk(uint8_t value);



    private:
        using mbc_writer = std::function<void(gb_memory&, uint16_t, uint8_t)>;

        struct gb_mbc1_data
        {
            uint8_t m_bank1;
            uint8_t m_bank2;
            bool    m_mode1;
        };

        struct gb_mbc5_data
        {
            uint8_t m_2000;
            uint8_t m_3000;
        };

        using gb_mbc_data = std::variant<gb_mbc1_data, gb_mbc5_data>;

        static mbc_writer get_mbc_writer(gb_mbc_data& mbc, uint8_t mbc_type);
        static void       write_to_mbc_no_op(gb_memory& memory, uint16_t offset, uint8_t value);
        static void       write_to_mbc1(gb_memory& memory, uint16_t offset, uint8_t value);
        static void       write_to_mbc2(gb_memory& memory, uint16_t offset, uint8_t value);
        static void       write_to_mbc3(gb_memory& memory, uint16_t offset, uint8_t value);
        static void       write_to_mbc5(gb_memory& memory, uint16_t offset, uint8_t value);
        static void       write_to_mbc5_rumble(gb_memory& memory, uint16_t offset, uint8_t value);



        [[nodiscard]] unsigned get_offset(uint16_t address) const;
        void                   set_ram_accessible(uint8_t value);
        void                   set_rom_banks(int low_bank_id, int high_bank_id);
        void                   set_ram_bank(int bank_id);

        // logging code is header-only to allow compile time optimization
        [[nodiscard]] gb_log_message_stream log() const
        {
            return m_clock.log(gb_log_category::lc_mbc);
        }

        const gb_clock& m_clock; //!< used only for logging

        // m_mbc_data must be initialized before m_mbc_writer!
        // Otherwise it's MBC-dependent value will be overwritten by
        // the initial value (see gb_memory constructor)
        gb_mbc_data m_mbc_data;
        mbc_writer  m_mbc_writer;

        const int16_t m_num_cart_rom_banks;
        const int16_t m_num_cart_ram_banks;
        const bool    m_has_battery;
        const bool    m_is_mbc2; //!< special RAM addressing
        bool          m_mbc1_multi_cart    = false;
        bool          m_mbc_ram_accessible = false;
        uint8_t       m_svbk               = 0xF8;
        uint8_t       m_vbk                = 0xF8;

        const int           m_cart_ram_offset;
        const int           m_internal_ram_offset;
        const int           m_video_ram_offset;
        uint8_vector        m_memory;
        std::array<int, 16> m_offsets = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    };

} // namespace age



#endif // AGE_GB_MEMORY_HPP
