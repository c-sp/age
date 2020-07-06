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
                    const uint8_t *video_ram,
                    gb_events &events,
                    gb_interrupt_trigger &interrupts,
                    screen_buffer &screen_buffer,
                    bool dmg_green)
    : m_device(device),
      m_clock(clock),
      m_scanline(device, clock),
      m_lcd_irqs(clock, m_scanline, events, interrupts),
      m_palettes(device, dmg_green),
      m_sprites(m_device.is_cgb()),
      m_render(device, m_palettes, m_sprites, video_ram, screen_buffer)
{
}



age::uint8_t age::gb_lcd::read_oam(int offset) const
{
    return m_sprites.read_oam(offset);
}

void age::gb_lcd::write_oam(int offset, uint8_t value)
{
    m_sprites.write_oam(offset, value);
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
    m_render.render(scanline);

    // start new frame?
    if (scanline >= gb_scanline_count)
    {
        AGE_GB_CLOG_LCD_RENDER("switch frame buffers");
        m_scanline.fast_forward_frames();
        m_render.new_frame();
        m_render.render(m_scanline.current_scanline());
    }
}

void age::gb_lcd::trigger_irq_vblank()
{
    m_lcd_irqs.trigger_irq_vblank();
}

void age::gb_lcd::trigger_irq_lyc()
{
    m_lcd_irqs.trigger_irq_lyc();
}

void age::gb_lcd::trigger_irq_mode2()
{
    m_lcd_irqs.trigger_irq_mode2();
}

void age::gb_lcd::trigger_irq_mode0()
{
    m_lcd_irqs.trigger_irq_mode0(m_render.m_scx);
}

void age::gb_lcd::set_back_clock(int clock_cycle_offset)
{
    m_scanline.set_back_clock(clock_cycle_offset);
    m_lcd_irqs.set_back_clock(clock_cycle_offset);
}
