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



age::gb_lcd::gb_lcd(const gb_device &device,
                    const gb_clock &clock,
                    const gb_memory &memory,
                    gb_events &events,
                    gb_interrupt_trigger &interrupts,
                    screen_buffer &screen_buffer,
                    bool dmg_green)
    : m_device(device),
      m_clock(clock),
      m_events(events),
      m_interrupts(interrupts),
      m_scanline(clock),
      m_renderer(device, memory, screen_buffer, dmg_green)
{
    m_renderer.create_dmg_palette(gb_palette_bgp, m_bgp);
    m_renderer.create_dmg_palette(gb_palette_obp0, m_obp0);
    m_renderer.create_dmg_palette(gb_palette_obp1, m_obp1);

    schedule_vblank_irq();
}

age::uint8_t* age::gb_lcd::get_oam()
{
    return m_renderer.get_oam();
}



void age::gb_lcd::update_state()
{
    // LCD on?
    int scanline = m_scanline.current_scanline();
    if (scanline == gb_scanline_lcd_off)
    {
        return;
    }

    // continue unfinished frame
    m_renderer.render(scanline);

    // start new frame?
    if (scanline > gb_scanline_count)
    {
        AGE_GB_CLOG_LCD("switch frame buffers");
        m_scanline.fast_forward_frames();
        m_renderer.new_frame();
        m_renderer.render(m_scanline.current_scanline());
    }
}

void age::gb_lcd::trigger_interrupt()
{
    AGE_ASSERT(m_next_vblank_irq != gb_no_clock_cycle);

    // pending v-blank interrupt?
    if (m_next_vblank_irq <= m_clock.get_clock_cycle())
    {
        m_interrupts.trigger_interrupt(gb_interrupt::vblank);
        schedule_vblank_irq();
    }
}

void age::gb_lcd::set_back_clock(int clock_cycle_offset)
{
    m_scanline.set_back_clock(clock_cycle_offset);
    AGE_GB_SET_BACK_CLOCK(m_next_vblank_irq, clock_cycle_offset);
}



void age::gb_lcd::update_color(unsigned color_idx)
{
    AGE_ASSERT(m_device.is_cgb());
    AGE_ASSERT(color_idx < m_cpd.size() / 2);

    int low_byte = m_cpd[color_idx * 2];
    int high_byte = m_cpd[color_idx * 2 + 1];
    int gb_color = (high_byte << 8) + low_byte;

    m_renderer.update_color(color_idx, gb_color);
}



void age::gb_lcd::schedule_vblank_irq()
{
    int clk_current = m_clock.get_clock_cycle();

    // first v-blank interrupt after the lcd was switched on
    if (m_next_vblank_irq == gb_no_clock_cycle)
    {
        int clk_frame_start = m_scanline.clk_frame_start();
        AGE_ASSERT(clk_frame_start != gb_no_clock_cycle);

        m_next_vblank_irq = clk_frame_start
                + gb_screen_height * gb_clock_cycles_per_scanline;
    }

    // follow up v-blank interrupt
    else
    {
        int clk_diff = clk_current - m_next_vblank_irq;
        int vblanks = 1 + clk_diff / gb_clock_cycles_per_frame;
        m_next_vblank_irq += vblanks * gb_clock_cycles_per_frame;
    }

    // schedule interrupt
    AGE_ASSERT(m_next_vblank_irq > clk_current);
    m_events.schedule_event(gb_event::lcd_interrupt, m_next_vblank_irq - clk_current);

    AGE_GB_CLOG_LCD("next v-blank interrupt in "
                    << (m_next_vblank_irq - m_clock.get_clock_cycle())
                    << " cycles (" << m_next_vblank_irq << ")");
}
