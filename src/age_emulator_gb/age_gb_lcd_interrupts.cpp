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

#include <algorithm>

#include <age_debug.hpp>

#include "age_gb_lcd.hpp"



age::gb_lcd_interrupts::gb_lcd_interrupts(const gb_clock &clock,
                                          const gb_lcd_scanline &scanline,
                                          gb_events &events,
                                          gb_interrupt_trigger &interrupts)
    : m_clock(clock),
      m_scanline(scanline),
      m_events(events),
      m_interrupts(interrupts)
{
    lcd_on(); // schedule inital interrupt event
}



age::uint8_t age::gb_lcd_interrupts::read_stat() const {
    return m_stat | m_scanline.stat_flags();
}

void age::gb_lcd_interrupts::write_stat(uint8_t value)
{
    m_stat = 0x80 | (value & ~(gb_stat_modes | gb_stat_ly_match));
    schedule_lyc_irq();
}



void age::gb_lcd_interrupts::trigger_interrupt_vblank()
{
    AGE_ASSERT(m_clk_next_vblank_irq != gb_no_clock_cycle);

    int clk_current = m_clock.get_clock_cycle();
    if (m_clk_next_vblank_irq > clk_current)
    {
        return;
    }

    m_clk_next_vblank_irq += add_total_frames(m_clk_next_vblank_irq, clk_current);
    int clk_diff = m_clk_next_vblank_irq - clk_current;

    AGE_GB_CLOG_LCD_IRQ("next v-blank interrupt in " << clk_diff
                        << " clock cycles (" << m_clk_next_vblank_irq << ")");

    m_events.schedule_event(gb_event::lcd_interrupt_vblank, clk_diff);
    m_interrupts.trigger_interrupt(gb_interrupt::vblank);
    if (m_stat & gb_stat_irq_mode1)
    {
        m_interrupts.trigger_interrupt(gb_interrupt::lcd);
    }
}

void age::gb_lcd_interrupts::trigger_interrupt_lyc()
{
    AGE_ASSERT(m_clk_next_lyc_irq != gb_no_clock_cycle);

    int clk_current = m_clock.get_clock_cycle();
    if (m_clk_next_lyc_irq > clk_current)
    {
        return;
    }

    m_clk_next_lyc_irq += add_total_frames(m_clk_next_lyc_irq, clk_current);
    int clk_diff = m_clk_next_lyc_irq - clk_current;

    AGE_GB_CLOG_LCD_IRQ("next lyc interrupt in " << clk_diff
                        << " clock cycles (" << m_clk_next_lyc_irq << ")");

    m_events.schedule_event(gb_event::lcd_interrupt_lyc, clk_diff);
    m_interrupts.trigger_interrupt(gb_interrupt::lcd);
}

void age::gb_lcd_interrupts::set_back_clock(int clock_cycle_offset)
{
    AGE_GB_SET_BACK_CLOCK(m_clk_next_vblank_irq, clock_cycle_offset);
    AGE_GB_SET_BACK_CLOCK(m_clk_next_lyc_irq, clock_cycle_offset);
}



void age::gb_lcd_interrupts::lcd_on()
{
    int clk_current = m_clock.get_clock_cycle();
    int clk_frame_start = m_scanline.clk_frame_start();
    AGE_ASSERT(clk_frame_start != gb_no_clock_cycle);

    m_clk_next_vblank_irq = clk_frame_start
            + gb_screen_height * gb_clock_cycles_per_scanline;

    AGE_ASSERT(m_clk_next_vblank_irq > clk_current);
    m_events.schedule_event(gb_event::lcd_interrupt_vblank, m_clk_next_vblank_irq - clk_current);

    schedule_lyc_irq();
}

void age::gb_lcd_interrupts::lcd_off()
{
    m_clk_next_vblank_irq = gb_no_clock_cycle;
    m_clk_next_lyc_irq = gb_no_clock_cycle;

    m_events.remove_event(gb_event::lcd_interrupt_vblank);
    m_events.remove_event(gb_event::lcd_interrupt_lyc);
}

void age::gb_lcd_interrupts::lyc_update()
{
    if (m_scanline.current_scanline() == gb_scanline_lcd_off)
    {
        return;
    }
    schedule_lyc_irq();
}



int age::gb_lcd_interrupts::add_total_frames(int clk_last, int clk_current)
{
    AGE_ASSERT(clk_last <= clk_current);

    int clk_diff = clk_current - clk_last;
    int frames = 1 + clk_diff / gb_clock_cycles_per_frame;

    return frames * gb_clock_cycles_per_frame;
}

void age::gb_lcd_interrupts::schedule_lyc_irq()
{
    if (!(m_stat & gb_stat_irq_ly_match) || (m_scanline.m_lyc >= gb_scanline_count))
    {
        m_clk_next_lyc_irq = gb_no_clock_cycle;
        m_events.remove_event(gb_event::lcd_interrupt_lyc);
        return;
    }

    // schedule next LYC irq
    m_clk_next_lyc_irq = m_scanline.clk_frame_start()
            + m_scanline.m_lyc * gb_clock_cycles_per_scanline;

    int clk_current = m_clock.get_clock_cycle();
    if (m_clk_next_lyc_irq <= clk_current)
    {
        m_clk_next_lyc_irq += gb_clock_cycles_per_frame;
    }
    AGE_ASSERT(m_clk_next_lyc_irq > clk_current);
    m_events.schedule_event(gb_event::lcd_interrupt_lyc, m_clk_next_lyc_irq - clk_current);

    // immediate LYC irq
    if (m_scanline.m_lyc == m_scanline.current_scanline())
    {
        m_interrupts.trigger_interrupt(gb_interrupt::lcd);
    }
}
