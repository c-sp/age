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

#ifndef AGE_GB_TYPES_HPP
#define AGE_GB_TYPES_HPP

//!
//! \file
//!

#include <age_types.hpp>

#include <string>
#include <unordered_set>



namespace age
{
    //!
    //! \brief Configures the device (Game Boy or Game Boy Color) to emulate.
    //!
    //! Setting a specific device is usually only required for running tests.
    //! The end user should let the emulator auto detect the device for the
    //! respective rom.
    //!
    enum class gb_device_type
    {
        //!
        //! Auto-detect the device based on the respective rom.
        //!
        auto_detect,

        //!
        //! Emulate a DMG device.
        //!
        dmg,

        //!
        //! Emulate a CGB-A/B/C/D device.
        //!
        cgb_abcd,

        //!
        //! Emulate a CGB-E device.
        //!
        cgb_e,
    };



    enum class gb_colors_hint
    {
        //!
        //! Use the default color calculation for the emulated device.
        //! DMG colors will be greenish,
        //! CGB colors are calculated as described here:
        //! https://forums.libretro.com/t/real-gba-and-ds-phat-colors/1540/190
        //!
        default_colors,

        //!
        //! Use grey shades instead of green shades for DMG emulation
        //! (0x000000, 0x555555, 0xAAAAAA and 0xFFFFFF).
        //!
        //! For CGB emulation this value is ignored and default CGB colors are
        //! used.
        //!
        dmg_greyscale,

        //!
        //! CGB colors are calculated compatible to Matt Currie's
        //! acid2 and Mealybug Tearoom test roms.
        //!
        //! For DMG emulation this value is ignored and default DMG colors are
        //! used.
        //!
        cgb_acid2,

        //!
        //! CGB colors are calculated compatible to Gambatte test roms.
        //!
        //! For DMG emulation this value is ignored and default DMG colors are
        //! used.
        //!
        cgb_gambatte
    };



    //!
    //! \brief Struct containing parts of the Game Boy CPU state.
    //!
    //! This struct was implemented for evaluating test rom results.
    //!
    struct gb_test_info
    {
        bool m_ld_b_b = false;

        uint8_t m_a = 0;
        uint8_t m_b = 0;
        uint8_t m_c = 0;
        uint8_t m_d = 0;
        uint8_t m_e = 0;
        uint8_t m_h = 0;
        uint8_t m_l = 0;
    };



    enum class gb_log_category
    {
        lc_clock,
        lc_cpu,
        lc_events,
        lc_hdma,
        lc_interrupts,
        lc_lcd,
        lc_lcd_oam,
        lc_lcd_oam_dma,
        lc_lcd_registers,
        lc_lcd_vram,
        lc_memory,
        lc_serial,
        lc_sound,
        lc_sound_registers,
        lc_timer,
    };

    using gb_log_categories = std::unordered_set<gb_log_category>;

    struct gb_log_entry
    {
        gb_log_entry(gb_log_category category, int64_t clock, int div_clock, std::string message)
            : m_category(category),
              m_clock(clock),
              m_div_clock(div_clock),
              m_message(std::move(message))
        {}

        gb_log_category m_category;
        int64_t         m_clock;
        uint16_t        m_div_clock;
        std::string     m_message;
    };

} // namespace age



#endif // AGE_GB_TYPES_HPP
