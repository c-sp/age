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

#include "age_gb_lcd_renderer.hpp"

#include <algorithm> // std::fill
#include <cassert>



age::gb_lcd_renderer::gb_lcd_renderer(const gb_device&         device,
                                      const gb_lcd_palettes&   palettes,
                                      gb_lcd_sprites&          sprites,
                                      std::span<uint8_t const> video_ram,
                                      screen_buffer&           screen_buffer)
    : gb_lcd_renderer_common(device, sprites),
      m_window(device.is_dmg_device()),
      m_fifo_renderer(device, *this, palettes, sprites, video_ram, m_window, screen_buffer),
      m_line_renderer(device, *this, palettes, sprites, video_ram, m_window, screen_buffer),
      m_screen_buffer(screen_buffer),
      m_palettes(palettes)
{
}



bool age::gb_lcd_renderer::stat_mode0() const
{
    return m_fifo_renderer.stat_mode0();
}

void age::gb_lcd_renderer::set_clks_tile_data_change(gb_current_line at_line)
{
    m_fifo_renderer.set_clks_tile_data_change(at_line);
}

void age::gb_lcd_renderer::set_clks_bgp_change(gb_current_line at_line)
{
    m_fifo_renderer.set_clks_bgp_change(at_line);
}

void age::gb_lcd_renderer::check_for_wy_match(gb_current_line at_line, uint8_t wy)
{
    m_window.check_for_wy_match(get_lcdc(), wy, at_line);
}

void age::gb_lcd_renderer::new_frame(bool frame_is_blank)
{
    if (frame_is_blank)
    {
        auto  blank       = m_device.is_dmg_device() ? m_palettes.get_color_zero_dmg() : pixel(0xFFFFFF);
        auto& back_buffer = m_screen_buffer.get_back_buffer();
        std::fill(begin(back_buffer), end(back_buffer), blank);
    }
    m_screen_buffer.switch_buffers();

    m_window.new_frame();
    m_rendered_lines = 0;
    m_fifo_renderer.reset();
    assert(!m_fifo_renderer.in_progress());
}



void age::gb_lcd_renderer::render(gb_current_line until, bool is_first_frame)
{
    // finish fifo-rendered line
    if (m_fifo_renderer.in_progress())
    {
        if (!m_fifo_renderer.continue_line(until))
        {
            return; // line not yet finished
        }
        ++m_rendered_lines;
    }

#ifdef AGE_FORCE_FIFO_RENDERER

    int sanitized = std::min<int>(gb_screen_height, until.m_line);

    for (; m_rendered_lines < sanitized; ++m_rendered_lines)
    {
        gb_current_line line{.m_line = m_rendered_lines, .m_line_clks = gb_clock_cycles_per_lcd_line};
        m_fifo_renderer.begin_new_line(line, is_first_frame);
    }

#else

    // render complete lines, if possible
    // (note that 289 T4 cycles is the longest mode 3 duration)
    int complete_lines = until.m_line + ((until.m_line_clks >= 80 + 289) ? 1 : 0);
    int sanitized      = std::min<int>(gb_screen_height, complete_lines);

    for (; m_rendered_lines < sanitized; ++m_rendered_lines)
    {
        m_line_renderer.render_line(m_rendered_lines);
    }

#endif

    // fifo-render current line, if necessary
    if (m_rendered_lines >= gb_screen_height)
    {
        return;
    }
    assert(m_rendered_lines >= until.m_line);
    if (m_rendered_lines == until.m_line)
    {
        m_fifo_renderer.begin_new_line(until, is_first_frame);
    }
}
