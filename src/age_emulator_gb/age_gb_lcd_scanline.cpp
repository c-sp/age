//
// Copyright 2019 Christoph Sprenger
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

#include <age_debug.hpp>

#include "age_gb_lcd.hpp"



age::gb_lcd_scanline::gb_lcd_scanline(const gb_clock &clock)
    : m_clock(clock)
{
    m_clk_frame_start = m_clock.get_clock_cycle();
}



int age::gb_lcd_scanline::clk_frame_start() const
{
    return m_clk_frame_start;
}

int age::gb_lcd_scanline::current_scanline() const
{
    if (m_clk_frame_start == gb_no_clock_cycle)
    {
        return gb_scanline_lcd_off;
    }
    int scanline, scanline_clks;
    calculate_scanline(scanline, scanline_clks);
    return scanline;
}



age::uint8_t age::gb_lcd_scanline::stat_flags() const
{
    if (m_clk_frame_start == gb_no_clock_cycle)
    {
        return m_retained_ly_match;
    }

    int scanline, scanline_clks;
    calculate_scanline(scanline, scanline_clks);

    return stat_ly_match(scanline, scanline_clks)
            | stat_mode(scanline, scanline_clks);
}

age::uint8_t age::gb_lcd_scanline::current_ly() const
{
    if (m_clk_frame_start == gb_no_clock_cycle)
    {
        return 0;
    }

    int scanline, scanline_clks;
    calculate_scanline(scanline, scanline_clks);
    AGE_ASSERT(scanline < gb_scanline_count);

    // LY = 153 for one machine cycle (two for CGB double speed)
    if ((scanline >= 153) && (scanline_clks >= 2 + m_clock.is_double_speed()))
    {
        return 0;
    }

    // Ly is incremented 2 clock cycles earlier than the respective scanline
    int ly = scanline + (scanline_clks >= gb_clock_cycles_per_scanline - 2);
    return ly & 0xFF;
}



void age::gb_lcd_scanline::lcd_on()
{
    // LCD already on
    if (m_clk_frame_start != gb_no_clock_cycle)
    {
        return;
    }

    // switch on LCD:
    // start new frame with shortened first scanline
    m_clk_frame_start = m_clock.get_clock_cycle() - 2;
    m_first_frame = true;
    AGE_GB_CLOG_LCD_PORTS("    * aligning first frame to clock cycle "
                          << m_clk_frame_start);

    // Clear STAT LY match flag as we now calculate it
    // for each STAT read
    // (bit was retained when switching off the LCD).
    m_retained_ly_match = 0;
}

void age::gb_lcd_scanline::lcd_off()
{
    // The STAT LY match flag is retained when switching off the LCD.
    // Mooneye GB tests:
    //      acceptance/ppu/stat_lyc_onoff
    m_retained_ly_match = stat_flags() & gb_stat_ly_match;
    AGE_GB_CLOG_LCD_PORTS("    * retained LY match "
                          << AGE_LOG_HEX8(m_retained_ly_match)
                          << " for LYC " << AGE_LOG_HEX8(m_lyc)
                          << " & scanline " << current_scanline());

    m_clk_frame_start = gb_no_clock_cycle;
    m_first_frame = false;
}

void age::gb_lcd_scanline::fast_forward_frames()
{
    AGE_ASSERT(m_clk_frame_start != gb_no_clock_cycle);

    // check for new frame
    int clk_current = m_clock.get_clock_cycle();
    int clks = clk_current - m_clk_frame_start;

    if (clks >= gb_clock_cycles_per_frame)
    {
        int frames = clks / gb_clock_cycles_per_frame;
        m_clk_frame_start += frames * gb_clock_cycles_per_frame;
        m_first_frame = false;
    }
}

void age::gb_lcd_scanline::set_back_clock(int clock_cycle_offset)
{
    AGE_GB_SET_BACK_CLOCK(m_clk_frame_start, clock_cycle_offset);
}



age::uint8_t age::gb_lcd_scanline::stat_mode(int scanline, int scanline_clks) const
{
    AGE_ASSERT(scanline != gb_scanline_lcd_off);
    AGE_ASSERT(scanline < gb_scanline_count);

    if (scanline >= gb_screen_height)
    {
        // mode 0 is signalled for v-blank's last machine cycle
        // (not for double speed though)
        if (m_clock.is_double_speed() || (scanline < 153))
        {
            return 1;
        }
        return (scanline_clks >= gb_clock_cycles_per_scanline - 2) ? 0 : 1;
    }
    // 80 cycles mode 0 (instead of mode 2) for the first
    // scanline after switching on the LCD.
    // Note that the first scanline after switching on the LCD
    // is 2 4-Mhz-clock cycles shorter than usual.
    if (m_first_frame && !scanline && (scanline_clks < 82))
    {
        return 0;
    }
    if (scanline_clks < 80)
    {
        return 2;
    }
    //! \todo fix for pixel fifo
    return (scanline_clks < 172) ? 3 : 0;
}

age::uint8_t age::gb_lcd_scanline::stat_ly_match(int scanline, int scanline_clks) const
{
    int match_scanline = scanline;

    // special timing for scanline 153
    if (scanline == 153)
    {
        //! \todo need more timing details
        if (scanline_clks > 4)
        {
            match_scanline = 0;
        }
    }

    // "regular" timing
    else
    {
        // when not running at double speed LY-match is cleared early
        int clks_next_scanline = gb_clock_cycles_per_scanline - scanline_clks;
        if (clks_next_scanline <= (m_clock.is_double_speed() ? 0 : 2))
        {
            match_scanline = -1;
        }
    }

    return m_lyc == match_scanline ? gb_stat_ly_match : 0;
}

void age::gb_lcd_scanline::calculate_scanline(int &scanline, int &scanline_clks) const
{
    AGE_ASSERT(m_clk_frame_start != gb_no_clock_cycle);
    int clks = m_clock.get_clock_cycle() - m_clk_frame_start;

    // We hope for the compiler to optimize the "/" and "%" operations,
    // e.g. to make use of the remainder being a by-product of the
    // division.
    // On x86 this should be a single DIV instruction.
    scanline = clks / gb_clock_cycles_per_scanline;
    scanline_clks = clks % gb_clock_cycles_per_scanline;
}
