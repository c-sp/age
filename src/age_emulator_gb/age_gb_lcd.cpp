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
                    screen_buffer &screen_buffer,
                    bool dmg_green)
    : m_device(device),
      m_clock(clock),
      m_scanline(clock),
      m_renderer(device, memory, screen_buffer, dmg_green)
{
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
        m_scanline.fast_forward_frames();
        m_renderer.new_frame();
        m_renderer.render(m_scanline.current_scanline());
    }
}

void age::gb_lcd::set_back_clock(int clock_cycle_offset)
{
    m_scanline.set_back_clock(clock_cycle_offset);
}



void age::gb_lcd::update_color(unsigned int color_idx)
{
    AGE_ASSERT(m_device.is_cgb());
    AGE_ASSERT(color_idx < m_cpd.size() / 2);

    uint8_t low_byte = m_cpd[color_idx * 2];
    uint8_t high_byte = m_cpd[color_idx * 2 + 1];

    // we rely on integer promotion to 32 bit int
    // for the following calculation
    int gb_color = (high_byte << 8) + low_byte;

    m_renderer.update_color(color_idx, gb_color);
}
