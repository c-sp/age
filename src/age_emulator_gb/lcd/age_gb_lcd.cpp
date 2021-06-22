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

#include "age_gb_lcd.hpp"



age::gb_lcd::gb_lcd(const gb_device&      device,
                    const gb_clock&       clock,
                    const uint8_t*        video_ram,
                    const uint8_t*        rom_header,
                    gb_events&            events,
                    gb_interrupt_trigger& interrupts,
                    screen_buffer&        screen_buffer,
                    gb_colors_hint        colors_hint)
    : m_device(device),
      m_clock(clock),
      m_scanline(device, clock),
      m_lcd_irqs(device, clock, m_scanline, events, interrupts),
      m_palettes(device, rom_header, colors_hint),
      m_sprites(m_device.is_cgb()),
      m_render(device, m_palettes, m_sprites, video_ram, screen_buffer)
{
}



age::uint8_t age::gb_lcd::read_oam(int offset)
{
    return m_sprites.read_oam(offset);
}

void age::gb_lcd::write_oam(int offset, uint8_t value)
{
    update_state();
    m_sprites.write_oam(offset, value);
}

bool age::gb_lcd::is_oam_accessible()
{
    // LCD off
    if (!m_scanline.lcd_is_on())
    {
        return true;
    }

    int scanline      = -1;
    int scanline_clks = -1;
    m_scanline.current_scanline(scanline, scanline_clks);

    return (scanline >= gb_screen_height)
           || (scanline_clks >= (80 + 172 + (m_render.m_scx & 7)));
}



bool age::gb_lcd::is_video_ram_accessible()
{
    // LCD off
    if (!m_scanline.lcd_is_on())
    {
        return true;
    }

    int scanline      = -1;
    int scanline_clks = -1;
    m_scanline.current_scanline(scanline, scanline_clks);

    // mode 3 end (+1 T4 cycle for double speed)
    // (Gambatte tests: vramw_m3end/*)
    return (scanline >= gb_screen_height)
           || (scanline_clks < 80)
           || (scanline_clks >= (80 + 172 + (m_render.m_scx & 7) + m_clock.is_double_speed()));
}



void age::gb_lcd::update_state()
{
    // LCD on?
    if (!m_scanline.lcd_is_on())
    {
        return;
    }

    // Continue unfinished frame.
    // During a scanline's mode 0 the emulated program may already prepare
    // data for the next scanline.
    // We thus render each scanline before it enters mode 0.
    int scanline      = -1;
    int scanline_clks = -1;
    m_scanline.current_scanline(scanline, scanline_clks);

    m_render.render(scanline + (scanline_clks >= 80));

    // start new frame?
    if (scanline >= gb_scanline_count)
    {
        m_scanline.fast_forward_frames();
        m_render.new_frame();
        update_state();
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
