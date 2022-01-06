//
// Copyright 2021 Christoph Sprenger
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

#ifndef AGE_GB_LCD_WINDOW_CHECK_HPP
#define AGE_GB_LCD_WINDOW_CHECK_HPP

//!
//! \file
//!

#include <age_debug.hpp>
#include <age_types.hpp>

namespace age
{
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



    class gb_window_check
    {
    public:
        explicit gb_window_check(bool dmg)
            : m_dmg(dmg)
        {
        }

        void new_frame()
        {
            m_last_window_off = gb_no_line;
            m_frame_wy_match  = false;
            m_current_wline   = -1;
        }

        void check_for_wy_match(uint8_t lcdc, int wy, int at_line)
        {
            AGE_ASSERT((lcdc & gb_lcdc_enable) != 0)
            // WY matching requires the window to be enabled
            // (see Gambatte window/late_enable_afterVblank tests)
            m_frame_wy_match |= is_window_enabled(lcdc) && (wy == at_line);
        }

        void check_for_wy_match(uint8_t lcdc, int wy, gb_current_line at_line)
        {
            AGE_ASSERT((lcdc & gb_lcdc_enable) != 0)
            // WY matching requires the window to be enabled
            // (see Gambatte window/late_enable_afterVblank tests)
            if (!is_window_enabled(lcdc) || m_frame_wy_match)
            {
                return;
            }

            // special timing for line 0:
            // not matched in the first 2 cycles
            if ((at_line.m_line == 0) && (at_line.m_line_clks < 3))
            {
                return;
            }

            // match the specified line
            int  max_wy_clks = 450 + (m_dmg ? 1 : 0);
            bool wy_match    = (at_line.m_line_clks <= max_wy_clks) && (wy == at_line.m_line);

            // CGB: match the next line on this line's last cycle
            if (!m_dmg && (at_line.m_line_clks >= 455))
            {
                wy_match |= (wy == (at_line.m_line + 1));
            }

            // AGE_LOG("WY " << wy << " match on line " << at_line.m_line << " @" << at_line.m_line_clks
            //               << ": " << wy_match << " (" << m_frame_wy_match << ")");
            m_frame_wy_match |= wy_match;
        }

        [[nodiscard]] bool is_enabled_and_wy_matched(uint8_t lcdc) const
        {
            return m_frame_wy_match && is_window_enabled(lcdc);
        }

        int next_window_line()
        {
            m_current_wline++;
            return m_current_wline;
        }

        [[nodiscard]] int current_window_line() const
        {
            AGE_ASSERT(m_current_wline >= 0)
            return m_current_wline;
        }

        void window_switched_off(gb_current_line at_line)
        {
            m_last_window_off = at_line;
        }

        [[nodiscard]] bool window_switched_off_on(gb_current_line line)
        {
            return m_last_window_off.as_64bits() == line.as_64bits();
        }

    private:
        gb_current_line m_last_window_off = gb_no_line;
        const bool      m_dmg;
        bool            m_frame_wy_match = false;
        int             m_current_wline  = -1;
    };

} // namespace age

#endif // AGE_GB_LCD_WINDOW_CHECK_HPP
