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



void age::gb_lcd_interrupts::trigger_interrupts()
{
    check_vblank_irq();
    check_lyc_irq();
}

void age::gb_lcd_interrupts::set_back_clock(int clock_cycle_offset)
{
    AGE_GB_SET_BACK_CLOCK(m_clk_next_vblank_irq, clock_cycle_offset);
    AGE_GB_SET_BACK_CLOCK(m_clk_next_lyc_irq, clock_cycle_offset);
}



void age::gb_lcd_interrupts::lcd_on()
{
    int clk_frame_start = m_scanline.clk_frame_start();
    AGE_ASSERT(clk_frame_start != gb_no_clock_cycle);

    m_clk_next_vblank_irq = clk_frame_start
            + gb_screen_height * gb_clock_cycles_per_scanline;
    AGE_ASSERT(m_clk_next_vblank_irq > m_clock.get_clock_cycle());

    //! \todo schedule next lyc irq

    schedule_irq_event();
}

void age::gb_lcd_interrupts::lcd_off()
{
    m_clk_next_vblank_irq = gb_no_clock_cycle;
    m_clk_next_lyc_irq = gb_no_clock_cycle;

    m_events.remove_event(gb_event::lcd_interrupt);
}

void age::gb_lcd_interrupts::lyc_update()
{
    //! \todo schedule next lyc irq
}



void age::gb_lcd_interrupts::check_vblank_irq()
{
    AGE_ASSERT(m_clk_next_vblank_irq != gb_no_clock_cycle);

    int clk_current = m_clock.get_clock_cycle();
    if (m_clk_next_vblank_irq > clk_current)
    {
        return;
    }

    m_interrupts.trigger_interrupt(gb_interrupt::vblank);

    int clk_diff = clk_current - m_clk_next_vblank_irq;
    int vblanks = 1 + clk_diff / gb_clock_cycles_per_frame;
    m_clk_next_vblank_irq += vblanks * gb_clock_cycles_per_frame;

    AGE_GB_CLOG_LCD_IRQ("next v-blank interrupt in "
                        << (m_clk_next_vblank_irq - clk_current)
                        << " clock cycles (" << m_clk_next_vblank_irq << ")");
    schedule_irq_event();
}

void age::gb_lcd_interrupts::check_lyc_irq()
{
    if (m_clk_next_lyc_irq == gb_no_clock_cycle)
    {
        return;
    }

    int clk_current = m_clock.get_clock_cycle();
    if (m_clk_next_lyc_irq > clk_current)
    {
        return;
    }

    m_interrupts.trigger_interrupt(gb_interrupt::lcd);

    //! \todo schedule next lyc irq
}

void age::gb_lcd_interrupts::schedule_irq_event()
{
    int clk_current = m_clock.get_clock_cycle();

    AGE_ASSERT(m_clk_next_vblank_irq != gb_no_clock_cycle);
    AGE_ASSERT(m_clk_next_vblank_irq > clk_current);
    int clk_next_event = m_clk_next_vblank_irq;

    if (m_clk_next_lyc_irq != gb_no_clock_cycle)
    {
        AGE_ASSERT(m_clk_next_lyc_irq > clk_current);
        clk_next_event = std::min<int>(clk_next_event, m_clk_next_lyc_irq);
    }

    // schedule interrupt
    AGE_ASSERT(clk_next_event > clk_current);
    m_events.schedule_event(gb_event::lcd_interrupt, clk_next_event - clk_current);
}
