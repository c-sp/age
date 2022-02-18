//
// Copyright 2022 Christoph Sprenger
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

#ifndef AGE_GB_LCD_COMMON_HPP
#define AGE_GB_LCD_COMMON_HPP

//!
//! \file
//!

#include "../../common/age_gb_clock.hpp"

#include <age_types.hpp>

namespace age
{
    constexpr int16_t gb_clock_cycles_per_lcd_line  = 456;
    constexpr int16_t gb_lcd_line_count             = 154;
    constexpr int     gb_clock_cycles_per_lcd_frame = gb_lcd_line_count * gb_clock_cycles_per_lcd_line;

    constexpr int16_t gb_screen_width  = 160;
    constexpr int16_t gb_screen_height = 144;

    constexpr uint8_t gb_lcdc_enable      = 0x80;
    constexpr uint8_t gb_lcdc_win_map     = 0x40;
    constexpr uint8_t gb_lcdc_win_enable  = 0x20;
    constexpr uint8_t gb_lcdc_bg_win_data = 0x10;
    constexpr uint8_t gb_lcdc_bg_map      = 0x08;
    constexpr uint8_t gb_lcdc_obj_size    = 0x04;
    constexpr uint8_t gb_lcdc_obj_enable  = 0x02;
    constexpr uint8_t gb_lcdc_bg_enable   = 0x01;

    constexpr bool is_window_enabled(uint8_t lcdc)
    {
        return (lcdc & gb_lcdc_win_enable) != 0;
    }

    struct gb_current_line
    {
        int m_line;
        int m_line_clks;

        [[nodiscard]] uint64_t as_64bits() const
        {
            uint64_t result = 0;
            memcpy(&result, &m_line, sizeof(uint64_t));
            return result;
        }
    };

    constexpr gb_current_line gb_no_line = {.m_line = -1, .m_line_clks = gb_no_clock_cycle};

} // namespace age

#endif // AGE_GB_LCD_COMMON_HPP
