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
      m_sprites(m_device.cgb_mode()),
      m_render(device, m_palettes, m_sprites, video_ram, screen_buffer)
{
}



bool age::gb_lcd::is_video_ram_accessible()
{
    // LCD off
    if (!m_line.lcd_is_on())
    {
        return true;
    }

    auto line = calculate_line();

    bool accessible = false;
    if (m_line.is_first_frame() && !line.m_line)
    {
        // DMG-C:                < 83 cycles
        // CGB-B/E single speed: < 84 cycles
        // CGB-B/E double speed: < 82 cycles
        int m3_edge = m_device.is_dmg_device()    ? 83
                      : m_clock.is_double_speed() ? 82
                                                  : 84;
        accessible  = (line.m_line_clks < m3_edge)
                     || (line.m_line_clks >= (82 + 172 + (m_render.m_scx & 7)));
    }
    else
    {
        int m3_edge = m_device.is_cgb_device() ? 80 : 78;
        accessible  = (line.m_line >= gb_screen_height)
                     || (line.m_line_clks < m3_edge)
                     || (line.m_line_clks >= (80 + 172 + (m_render.m_scx & 7)));
    }

    m_clock.log(gb_log_category::lc_lcd_vram) << "VRAM accessible: " << accessible << log_line_clks(m_line);
    return accessible;
}



void age::gb_lcd::after_speed_change()
{
    if (m_line.lcd_is_on() && m_clock.is_double_speed())
    {
        update_state();
        m_line.align_after_speed_change(1);
        m_lcd_irqs.align_after_speed_change(1);
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



void age::gb_lcd::update_state()
{
    update_state(0);
}

void age::gb_lcd::check_for_finished_frame()
{
    if (!m_line.lcd_is_on())
    {
        return;
    }
    auto line = m_line.current_line();
    if (line.m_line >= gb_lcd_line_count)
    {
        update_frame();
    }
}

void age::gb_lcd::update_state(int line_clock_offset)
{
    if (!m_line.lcd_is_on())
    {
        return;
    }
    bool new_frame = update_frame(line_clock_offset);
    if (new_frame)
    {
        new_frame = update_frame(line_clock_offset);
        AGE_ASSERT(!new_frame)
    }
}

bool age::gb_lcd::update_frame(int line_clock_offset)
{
    AGE_ASSERT(m_line.lcd_is_on())

    // continue rendering the current unfinished frame
    auto line           = m_line.current_line();
    auto is_first_frame = m_line.is_first_frame();
    line.m_line_clks    = std::min(std::max(line.m_line_clks + line_clock_offset, 0), gb_clock_cycles_per_lcd_line - 1);
    m_render.render(line, is_first_frame);

    if (line.m_line < gb_lcd_line_count)
    {
        return false;
    }

    // begin a new frame, if the current frame is finished
    // (we do NOT start rendering the new frame here,
    // call update_frame() a second time to do this)
    m_line.fast_forward_frames();
    m_render.new_frame(is_first_frame);
    return true;
}
