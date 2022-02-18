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

#include <age_debug.hpp>



age::gb_lcd_renderer::gb_lcd_renderer(const gb_device&       device,
                                      const gb_lcd_palettes& palettes,
                                      gb_lcd_sprites&        sprites,
                                      const uint8_t*         video_ram,
                                      screen_buffer&         screen_buffer)
    : gb_lcd_renderer_common(device, sprites),
      m_window(device.is_dmg_device()),
      m_fifo_renderer(device, *this, palettes, sprites, video_ram, m_window, screen_buffer),
      m_line_renderer(device, *this, palettes, sprites, video_ram, m_window, screen_buffer),
      m_screen_buffer(screen_buffer)
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

void age::gb_lcd_renderer::window_switched_off(gb_current_line at_line)
{
    m_window.window_switched_off(at_line);
}

void age::gb_lcd_renderer::new_frame()
{
    m_fifo_renderer.reset(); //! \todo maybe we can remove this after fixing LCD off/on

    AGE_ASSERT(!m_fifo_renderer.in_progress())
    //! \todo AGE_ASSERT(m_rendered_lines == gb_screen_height) (does not work for LCD off/on)

    m_window.new_frame();
    m_screen_buffer.switch_buffers();
    m_rendered_lines = 0;
}



#ifdef AGE_COMPILE_FIFO_RENDERER

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
    AGE_ASSERT(m_rendered_lines >= until.m_line)
    if (m_rendered_lines == until.m_line)
    {
        m_fifo_renderer.begin_new_line(until, is_first_frame);
    }
}

#else

void age::gb_lcd_renderer::render(gb_current_line until, bool is_first_frame)
{
    // AGE_COMPILE_FIFO_RENDERER not set
    AGE_UNUSED(is_first_frame);
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
