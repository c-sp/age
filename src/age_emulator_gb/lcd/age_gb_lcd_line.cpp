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



age::gb_lcd_line::gb_lcd_line(const gb_device& device,
                              const gb_clock&  clock)
    : m_clock(clock),
      m_clk_frame_start(m_clock.get_clock_cycle())
{
    if (device.is_cgb())
    {
        m_clk_frame_start += 4396 - gb_clock_cycles_per_lcd_frame;
    }
    else
    {
        m_clk_frame_start += 60 - gb_clock_cycles_per_lcd_frame;
    }
    m_clk_frame_start += gb_lcd_m_cycle_align;
    m_clk_line_start = m_clk_frame_start;

    AGE_GB_CLOG_LCD_PORTS("aligned frame to clock cycle " << m_clk_frame_start)
}



void age::gb_lcd_line::after_speed_change()
{
    if ((m_clk_frame_start != gb_no_clock_cycle) && is_odd_alignment()) {
        ++m_clk_frame_start;
        ++m_clk_line_start;
    }
}

void age::gb_lcd_line::set_back_clock(int clock_cycle_offset)
{
    gb_set_back_clock_cycle(m_clk_frame_start, clock_cycle_offset);
    gb_set_back_clock_cycle(m_clk_line_start, clock_cycle_offset);
}



//---------------------------------------------------------
//
//   LCD operation (on/off)
//
//---------------------------------------------------------

bool age::gb_lcd_line::lcd_is_on() const
{
    return m_clk_frame_start != gb_no_clock_cycle;
}

void age::gb_lcd_line::lcd_on()
{
    // LCD already on
    if (lcd_is_on())
    {
        return;
    }

    // switch on LCD:
    // start new frame with shortened first line
    m_clk_frame_start = m_clock.get_clock_cycle() + gb_lcd_m_cycle_align + m_clock.is_double_speed();
    m_clk_line_start  = m_clk_frame_start;
    m_line            = 0;
    m_first_frame     = true;
    AGE_GB_CLOG_LCD_PORTS("    * aligning first frame to clock cycle " << m_clk_frame_start)
}

void age::gb_lcd_line::lcd_off()
{
    // LCD already off
    if (!lcd_is_on())
    {
        return;
    }

    m_clk_frame_start = gb_no_clock_cycle;
    m_clk_line_start  = gb_no_clock_cycle;
    m_line            = 0;
    m_first_frame     = false;
}



//---------------------------------------------------------
//
//   frame
//
//---------------------------------------------------------

int age::gb_lcd_line::clk_frame_start() const
{
    return m_clk_frame_start;
}

bool age::gb_lcd_line::is_first_frame() const
{
    return m_first_frame;
}

bool age::gb_lcd_line::is_odd_alignment() const
{
    return (m_clk_frame_start % 2) != 0;
}

void age::gb_lcd_line::fast_forward_frames()
{
    AGE_ASSERT(lcd_is_on())

    // check for new frame
    int clk_current = m_clock.get_clock_cycle();
    int clks        = clk_current - m_clk_frame_start;

    if (clks >= gb_clock_cycles_per_lcd_frame)
    {
        int frames = clks / gb_clock_cycles_per_lcd_frame;
        m_clk_frame_start += frames * gb_clock_cycles_per_lcd_frame;

        m_clk_line_start = m_clk_frame_start;
        m_line           = 0;
        m_first_frame    = false;
        AGE_GB_CLOG_LCD_PORTS("aligned frame to clock cycle " << m_clk_frame_start)
    }
}



//---------------------------------------------------------
//
//   line
//
//---------------------------------------------------------

age::gb_current_line age::gb_lcd_line::current_line() const
{
    AGE_ASSERT(lcd_is_on())

    int clk_current = m_clock.get_clock_cycle();
    int clks_diff   = clk_current - m_clk_line_start;

    // no need to recalculate the line
    // (no expensive division)
    if (clks_diff < gb_clock_cycles_per_lcd_line)
    {
        return {.m_line = m_line, .m_line_clks = clks_diff};
    }

    // recalculate line
    int clks_frame = clk_current - m_clk_frame_start;

    // We hope for the compiler to optimize the "/" and "%" operations,
    // e.g. to make use of the remainder being a by-product of the
    // division.
    // On x86 this should be a single DIV instruction.
    m_line        = clks_frame / gb_clock_cycles_per_lcd_line;
    int line_clks = clks_frame % gb_clock_cycles_per_lcd_line;

    m_clk_line_start = m_clk_frame_start + m_line * gb_clock_cycles_per_lcd_line;

    return {.m_line = m_line, .m_line_clks = line_clks};
}
