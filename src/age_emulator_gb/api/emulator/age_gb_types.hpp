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
    //! \brief Configures the hardware (Game Boy or Game Boy Color) to emulate.
    //!
    //! Setting specific hardware is usually only required for running tests.
    //! The end user should probably let the emulator auto detect the hardware
    //! required by the rom to run.
    //!
    enum class gb_hardware
    {
        auto_detect,
        cgb,
        dmg
    };



    //!
    //! \brief The operating mode derived from hardware and cartridge capabilities.
    //!
    enum class gb_cart_mode
    {
        dmg,
        dmg_on_cgb,
        cgb
    };



    enum class gb_colors_hint
    {
        default_colors,
        dmg_greyscale,
        cgb_acid2
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
        lc_events,
        lc_timer,
    };

    using gb_log_categories = std::unordered_set<gb_log_category>;

    struct gb_log_entry
    {
        gb_log_entry(gb_log_category category, int clock, int div_offset, std::string message)
            : m_category(category),
              m_clock(clock),
              m_div_offset(div_offset),
              m_message(std::move(message))
        {}

        gb_log_category m_category;
        int             m_clock;
        int             m_div_offset;
        std::string     m_message;
    };

} // namespace age



#endif // AGE_GB_TYPES_HPP
