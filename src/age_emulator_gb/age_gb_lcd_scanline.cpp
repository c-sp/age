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

#include <age_debug.hpp>

#include "age_gb_lcd.hpp"



age::gb_lcd_scanline::gb_lcd_scanline(const gb_device& device,
                                      const gb_clock&  clock)
    : m_clock(clock)
{
    m_clk_frame_start = m_clock.get_clock_cycle();
    if (device.is_cgb())
    {
        m_clk_frame_start += 4396 - gb_clock_cycles_per_frame;
    }
    else
    {
        m_clk_frame_start += 60 - gb_clock_cycles_per_frame;
    }
    m_clk_frame_start += gb_lcd_m_cycle_align;
    m_clk_scanline_start = m_clk_frame_start;

    AGE_GB_CLOG_LCD_PORTS("aligned frame to clock cycle " << m_clk_frame_start)
}

void age::gb_lcd_scanline::set_back_clock(int clock_cycle_offset)
{
    AGE_GB_SET_BACK_CLOCK(m_clk_frame_start, clock_cycle_offset)
    AGE_GB_SET_BACK_CLOCK(m_clk_scanline_start, clock_cycle_offset)
}



//---------------------------------------------------------
//
//   LCD operation (on/off)
//
//---------------------------------------------------------

bool age::gb_lcd_scanline::lcd_is_on() const
{
    return m_clk_frame_start != gb_no_clock_cycle;
}

void age::gb_lcd_scanline::lcd_on()
{
    // LCD already on
    if (lcd_is_on())
    {
        return;
    }

    // switch on LCD:
    // start new frame with shortened first scanline
    m_clk_frame_start    = m_clock.get_clock_cycle() + gb_lcd_m_cycle_align;
    m_clk_scanline_start = m_clk_frame_start;
    m_scanline           = 0;
    m_first_frame        = true;
    AGE_GB_CLOG_LCD_PORTS("    * aligning first frame to clock cycle "
                          << m_clk_frame_start)
}

void age::gb_lcd_scanline::lcd_off()
{
    // LCD already off
    if (!lcd_is_on())
    {
        return;
    }

    m_clk_frame_start    = gb_no_clock_cycle;
    m_clk_scanline_start = gb_no_clock_cycle;
    m_scanline           = 0;
    m_first_frame        = false;
}



//---------------------------------------------------------
//
//   frame
//
//---------------------------------------------------------

int age::gb_lcd_scanline::clk_frame_start() const
{
    return m_clk_frame_start;
}

bool age::gb_lcd_scanline::is_first_frame() const
{
    return m_first_frame;
}

void age::gb_lcd_scanline::fast_forward_frames()
{
    AGE_ASSERT(lcd_is_on())

    // check for new frame
    int clk_current = m_clock.get_clock_cycle();
    int clks        = clk_current - m_clk_frame_start;

    if (clks >= gb_clock_cycles_per_frame)
    {
        int frames = clks / gb_clock_cycles_per_frame;
        m_clk_frame_start += frames * gb_clock_cycles_per_frame;

        m_clk_scanline_start = m_clk_frame_start;
        m_scanline           = 0;

        m_first_frame = false;
        AGE_GB_CLOG_LCD_PORTS("aligned frame to clock cycle " << m_clk_frame_start)
    }
}



//---------------------------------------------------------
//
//   scanline
//
//---------------------------------------------------------

void age::gb_lcd_scanline::current_scanline(int& scanline, int& scanline_clks) const
{
    AGE_ASSERT(lcd_is_on())

    int clk_current = m_clock.get_clock_cycle();
    int clks_diff   = clk_current - m_clk_scanline_start;

    // no need to recalculate the scanline
    // (no expensive division)
    if (clks_diff < gb_clock_cycles_per_scanline)
    {
        scanline      = m_scanline;
        scanline_clks = clks_diff;
        return;
    }

    // recalculate scanline
    int clks_frame = clk_current - m_clk_frame_start;

    // We hope for the compiler to optimize the "/" and "%" operations,
    // e.g. to make use of the remainder being a by-product of the
    // division.
    // On x86 this should be a single DIV instruction.
    scanline      = clks_frame / gb_clock_cycles_per_scanline;
    scanline_clks = clks_frame % gb_clock_cycles_per_scanline;

    m_clk_scanline_start = m_clk_frame_start + scanline * gb_clock_cycles_per_scanline;
    m_scanline           = scanline;
}
