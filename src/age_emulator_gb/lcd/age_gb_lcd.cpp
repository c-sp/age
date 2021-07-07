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
      m_line(device, clock),
      m_lcd_irqs(device, clock, m_line, events, interrupts),
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
    if (!m_line.lcd_is_on())
    {
        return true;
    }

    auto line = calculate_line();
    AGE_ASSERT(line.m_line < gb_lcd_line_count)

    bool accessible = false;
    if (line.m_line_clks == gb_clock_cycles_per_lcd_line - 1)
    {
        accessible = false;
    }
    else if (m_line.is_first_frame() && !line.m_line)
    {
        accessible = (line.m_line_clks < 82) || (line.m_line_clks >= (80 + 172 + (m_render.m_scx & 7) + 2));
    }
    else
    {
        accessible = (line.m_line >= gb_screen_height) || (line.m_line_clks >= (80 + 172 + (m_render.m_scx & 7)));
    }

    log_reg() << "OAM accessible: " << accessible
              << " (" << line.m_line_clks << " clks into line " << line.m_line << ")";
    return accessible;
}



bool age::gb_lcd::is_video_ram_accessible()
{
    // LCD off
    if (!m_line.lcd_is_on())
    {
        return true;
    }

    auto line = calculate_line();

    // mode 3 end (+1 T4 cycle for double speed)
    // (Gambatte tests: vramw_m3end/*)
    bool accessible = (line.m_line >= gb_screen_height)
                      || (line.m_line_clks < 80)
                      || (line.m_line_clks >= (80 + 172 + (m_render.m_scx & 7)));

    log_reg() << "VRAM accessible: " << accessible
              << " (" << line.m_line_clks << " clks into line " << line.m_line << ")";
    return accessible;
}



void age::gb_lcd::after_speed_change()
{
    if (m_line.lcd_is_on())
    {
        update_state();
        m_line.after_speed_change();
    }
}

void age::gb_lcd::update_state()
{
    // LCD on?
    if (!m_line.lcd_is_on())
    {
        return;
    }

    // Continue unfinished frame.
    // During a line's mode 0 the emulated program may already prepare
    // data for the next line.
    // We thus render each line before it enters mode 0.
    auto line = m_line.current_line();

    m_render.render(line.m_line + ((line.m_line_clks >= 80) ? 1 : 0));

    // start new frame?
    if (line.m_line >= gb_lcd_line_count)
    {
        m_line.fast_forward_frames();
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
    m_line.set_back_clock(clock_cycle_offset);
    m_lcd_irqs.set_back_clock(clock_cycle_offset);
}
