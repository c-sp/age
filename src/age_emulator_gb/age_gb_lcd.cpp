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
      m_lcd_irqs(device, clock, m_scanline, events, interrupts),
      m_palettes(device, dmg_green),
      m_sprites(m_device.is_cgb()),
      m_render(device, m_palettes, m_sprites, video_ram, screen_buffer)
{
}



age::uint8_t age::gb_lcd::read_oam(int offset)
{
    if (!is_oam_accessible())
    {
        AGE_GB_CLOG_LCD_OAM("read OAM @" << AGE_LOG_HEX8(offset)
                            << ": 0xFF (not accessible now)");
        return 0xFF;
    }
    auto result = m_sprites.read_oam(offset);
    AGE_GB_CLOG_LCD_OAM("read OAM @" << AGE_LOG_HEX8(offset)
                        << ": " << AGE_LOG_HEX8(result));
    return result;
}

void age::gb_lcd::write_oam(int offset, uint8_t value)
{
    if (!is_oam_accessible())
    {
        AGE_GB_CLOG_LCD_OAM("write to OAM @" << AGE_LOG_HEX8(offset)
                            << " ignored, not accessible now");
        return;
    }
    if (m_scanline.lcd_is_on())
    {
        update_state();
    }
    AGE_GB_CLOG_LCD_OAM("write to OAM @" << AGE_LOG_HEX8(offset)
                        << ": " << AGE_LOG_HEX8(value));
    m_sprites.write_oam(offset, value);
}

bool age::gb_lcd::is_oam_accessible()
{
    // LCD off
    if (!m_scanline.lcd_is_on())
    {
        return true;
    }

    int scanline, scanline_clks;
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

    int scanline, scanline_clks;
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
    int scanline, scanline_clks;
    m_scanline.current_scanline(scanline, scanline_clks);

    m_render.render(scanline + (scanline_clks >= 80));

    // start new frame?
    if (scanline >= gb_scanline_count)
    {
        AGE_GB_CLOG_LCD_RENDER("switch frame buffers on scanline " << scanline);
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
