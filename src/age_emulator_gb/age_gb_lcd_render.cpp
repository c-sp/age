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



age::gb_lcd_renderer::gb_lcd_renderer(const gb_device &device,
                                      const gb_memory &memory,
                                      screen_buffer &screen_buffer,
                                      bool dmg_green)
    : m_device(device),
      m_memory(memory),
      m_screen_buffer(screen_buffer),
      m_dmg_green(dmg_green)
{
}



age::uint8_t* age::gb_lcd_renderer::get_oam()
{
    return &m_oam[0];
}

void age::gb_lcd_renderer::set_lcdc(int lcdc)
{
    m_bg_tile_map_offset = (lcdc & gb_lcdc_bg_map) ? 0x1C00 : 0x1800;
    m_win_tile_map_offset = (lcdc & gb_lcdc_win_map) ? 0x1C00 : 0x1800;
    m_tile_idx_offset = (lcdc & gb_lcdc_bg_win_data) ? 0 : -128;
    m_tile_data_offset = (lcdc & gb_lcdc_bg_win_data) ? 0x0000 : 0x0800;
}



void age::gb_lcd_renderer::create_dmg_palette(unsigned palette_idx, uint8_t colors)
{
    if (m_device.is_cgb())
    {
        return;
    }
    AGE_ASSERT(palette_idx < m_colors.size() - 4);

    // greenish Gameboy colors
    // (taken from some googled gameboy photo)
    if (m_dmg_green)
    {
        for (unsigned idx = palette_idx, max = palette_idx + 4; idx < max; ++idx)
        {
            switch (colors & 0x03)
            {
                case 0x00: m_colors[idx] = pixel(152, 192, 15); break;
                case 0x01: m_colors[idx] = pixel(112, 152, 15); break;
                case 0x02: m_colors[idx] = pixel(48, 96, 15); break;
                case 0x03: m_colors[idx] = pixel(15, 56, 15); break;
            }
            colors >>= 2;
        }
        return;
    }

    // grayish Gameboy colors, used by test runner
    for (unsigned idx = palette_idx, max = palette_idx + 4; idx < max; ++idx)
    {
        switch (colors & 0x03)
        {
            case 0x00: m_colors[idx] = pixel(255, 255, 255); break;
            case 0x01: m_colors[idx] = pixel(170, 170, 170); break;
            case 0x02: m_colors[idx] = pixel(85, 85, 85); break;
            case 0x03: m_colors[idx] = pixel(0, 0, 0); break;
        }
        colors >>= 2;
    }
}



void age::gb_lcd_renderer::update_color(unsigned color_idx, int gb_color)
{
    AGE_ASSERT(m_device.is_cgb());
    AGE_ASSERT(color_idx < m_colors.size());

    // gb-color:   red:    bits 0-4
    //             green:  bits 5-9
    //             blue:   bits 10-14
    int gb_r = gb_color & 0x1F;
    int gb_g = (gb_color >> 5) & 0x1F;
    int gb_b = (gb_color >> 10) & 0x1F;

    // formula copied from gambatte (video.cpp)
    int r = (gb_r * 13 + gb_g * 2 + gb_b) >> 1;
    int g = (gb_g * 3 + gb_b) << 1;
    int b = (gb_r * 3 + gb_g * 2 + gb_b * 11) >> 1;

    m_colors[color_idx] = pixel(r, g, b);
}



void age::gb_lcd_renderer::render(int until_scanline)
{
    AGE_ASSERT(until_scanline >= m_rendered_scanlines);
    AGE_ASSERT(m_rendered_scanlines <= gb_screen_height);

    int santized = std::min<int>(gb_screen_height, until_scanline);
    int to_render = santized - m_rendered_scanlines;

    // new scanlines to render?
    if (to_render <= 0)
    {
        return;
    }

    //! \todo render scanlines
    m_rendered_scanlines += to_render;
}



void age::gb_lcd_renderer::new_frame()
{
    m_screen_buffer.switch_buffers();
    m_rendered_scanlines = 0;
}
