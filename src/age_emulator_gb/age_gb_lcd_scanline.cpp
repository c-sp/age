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

int age::gb_lcd_scanline::current_ly() const
{
    int scanline = current_scanline();
    if (scanline == gb_scanline_lcd_off)
    {
        return 0;
    }
    return scanline & 0xFF;
}

int age::gb_lcd_scanline::current_scanline() const
{
    if (m_clk_frame_start == gb_no_clock_cycle)
    {
        return gb_scanline_lcd_off;
    }

    int clks = m_clock.get_clock_cycle() - m_clk_frame_start;
    return clks / gb_clock_cycles_per_scanline;
}

age::uint8_t age::gb_lcd_scanline::stat_mode() const
{
    if (m_clk_frame_start == gb_no_clock_cycle)
    {
        return 0;
    }

    int clks_frame = m_clock.get_clock_cycle() - m_clk_frame_start;
    int scanline = clks_frame / gb_clock_cycles_per_scanline;
    int clks_scanline = clks_frame % gb_clock_cycles_per_scanline;

    if (scanline >= gb_screen_height)
    {
        return 1;
    }
    if (clks_scanline < 80)
    {
        // LY 0 mode 2 is not flagged right after the LCD has been
        // switched on (no sprites on this scanline then?).
        //
        // Gambatte tests:
        //      enable_display/nextstat_1_dmg08_cgb04c_out84
        //      enable_display/nextstat_2_dmg08_cgb04c_out87
        return (m_first_frame && (scanline == 0)) ? 0 : 2;
    }
    //! \todo fix for pixel fifo
    return (clks_scanline < 172) ? 3 : 0;
}



void age::gb_lcd_scanline::lcd_on()
{
    // LCD already on
    if (m_clk_frame_start != gb_no_clock_cycle)
    {
        return;
    }
    // switch on LCD:
    // start new frame
    m_clk_frame_start = m_clock.get_clock_cycle();
    m_first_frame = true;
}

void age::gb_lcd_scanline::lcd_off()
{
    m_clk_frame_start = gb_no_clock_cycle;
}



void age::gb_lcd_scanline::fast_forward_frames()
{
    // LCD off
    if (m_clk_frame_start == gb_no_clock_cycle)
    {
        return;
    }

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
