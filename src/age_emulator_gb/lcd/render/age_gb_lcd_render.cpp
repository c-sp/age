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

#include "age_gb_lcd_render.hpp"

#include <age_debug.hpp>



namespace
{
    age::uint8_array<256> calculate_xflip_lut()
    {
        age::uint8_array<256> xflip;

        for (unsigned byte = 0; byte < 256; ++byte)
        {
            age::uint8_t flip_byte = 0;

            for (unsigned bit = 0x01, flip_bit = 0x80;
                 bit < 0x100;
                 bit += bit, flip_bit >>= 1)
            {
                if ((byte & bit) > 0)
                {
                    flip_byte |= flip_bit;
                }
            }

            xflip[byte] = flip_byte;
        }

        return xflip;
    }

} // namespace



age::gb_lcd_render_common::gb_lcd_render_common(const gb_device& device, gb_lcd_sprites& sprites)
    : m_device(device),
      m_sprites(sprites),
      m_xflip_cache(calculate_xflip_lut())
{
    set_lcdc(0x91);
}

void age::gb_lcd_render_common::set_lcdc(uint8_t lcdc)
{
    m_lcdc = lcdc;

    m_sprites.set_sprite_size((lcdc & gb_lcdc_obj_size) ? 16 : 8);

    m_bg_tile_map_offset  = (lcdc & gb_lcdc_bg_map) ? 0x1C00 : 0x1800;
    m_win_tile_map_offset = (lcdc & gb_lcdc_win_map) ? 0x1C00 : 0x1800;

    m_tile_data_offset = (lcdc & gb_lcdc_bg_win_data) ? 0x0000 : 0x0800;
    m_tile_xor         = (lcdc & gb_lcdc_bg_win_data) ? 0 : 0x80;

    if (m_device.cgb_mode())
    {
        // CGB: if LCDC bit 0 is 0, sprites are always displayed above
        // BG & window regardless of any priority flags
        m_priority_mask = (lcdc & gb_lcdc_bg_enable) ? 0xFF : 0x00;
    }
}

void age::gb_lcd_render_common::reset_window()
{
    m_wline = -1;
}

bool age::gb_lcd_render_common::window_visible(int line) const
{
    bool win_x_visible = ((m_lcdc & gb_lcdc_win_enable) != 0) && (m_wx < 167);

    // window is already being rendered => ignore (potentially modified) WY
    if (m_wline >= 0)
    {
        return win_x_visible;
    }

    // start rendering window on this line?
    if (win_x_visible && (m_wy <= line))
    {
        m_wline = 0;
        return true;
    }
    return false;
}

int age::gb_lcd_render_common::get_next_window_line() const
{
    int wline = m_wline;
    if (wline < 0)
    {
        return -1;
    }
    ++m_wline;
    return wline;
}



age::gb_lcd_render::gb_lcd_render(const gb_device&       device,
                                  const gb_lcd_palettes& palettes,
                                  gb_lcd_sprites&        sprites,
                                  const uint8_t*         video_ram,
                                  screen_buffer&         screen_buffer)
    : gb_lcd_render_common(device, sprites),
      m_dot_renderer(*this, device, palettes, sprites, video_ram, screen_buffer),
      m_line_renderer(*this, device, palettes, sprites, video_ram, screen_buffer),
      m_screen_buffer(screen_buffer)
{
}



void age::gb_lcd_render::new_frame()
{
    m_dot_renderer.reset(); //! \todo maybe we can remove this after fixing LCD off/on

    AGE_ASSERT(!m_dot_renderer.in_progress())
    //! \todo AGE_ASSERT(m_rendered_lines == gb_screen_height) (does not work for LCD off/on)

    reset_window();
    m_screen_buffer.switch_buffers();
    m_rendered_lines = 0;
}



#ifdef AGE_COMPILE_DOT_RENDERER

void age::gb_lcd_render::render(gb_current_line until)
{
    // finish dot-rendered line
    if (m_dot_renderer.in_progress())
    {
        if (!m_dot_renderer.continue_line(until))
        {
            return;
        }
        ++m_rendered_lines;
    }

    // render complete lines, if possible
    // (note that 289 T4 cycles is the longest mode 3 duration)
    int complete_lines = until.m_line + ((until.m_line_clks >= 80 + 289) ? 1 : 0);
    int sanitized = std::min<int>(gb_screen_height, complete_lines);

    for (; m_rendered_lines < sanitized; ++m_rendered_lines)
    {
        m_line_renderer.render_line(m_rendered_lines);
    }

    // dot-render current line, if necessary
    if (m_rendered_lines >= gb_screen_height)
    {
        return;
    }
    AGE_ASSERT(m_rendered_lines >= until.m_line)
    if (m_rendered_lines == until.m_line)
    {
        m_dot_renderer.begin_new_line(until);
    }
}

#else

void age::gb_lcd_render::render(gb_current_line until)
{
    // AGE_COMPILE_DOT_RENDERER not set
    int until_line = until.m_line + ((until.m_line_clks >= 80) ? 1 : 0);

    AGE_ASSERT(until_line >= m_rendered_lines)
    AGE_ASSERT(m_rendered_lines <= gb_screen_height)

    int sanitized = std::min<int>(gb_screen_height, until_line);
    int to_render = sanitized - m_rendered_lines;

    // new lines to render?
    if (to_render <= 0)
    {
        return;
    }

    // render lines
    int line = m_rendered_lines;
    m_rendered_lines += to_render;

    for (; line < m_rendered_lines; ++line)
    {
        m_line_renderer.render_line(line);
    }
}

#endif