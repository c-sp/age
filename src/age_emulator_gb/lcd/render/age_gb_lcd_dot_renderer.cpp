//
// Copyright 2021 Christoph Sprenger
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



age::gb_lcd_dot_renderer::gb_lcd_dot_renderer(const gb_lcd_render_common& common,
                                              const gb_device&            device,
                                              const gb_lcd_palettes&      palettes,
                                              const gb_lcd_sprites&       sprites,
                                              const uint8_t*              video_ram,
                                              screen_buffer&              screen_buffer)
    : m_common(common),
      m_device(device),
      m_palettes(palettes),
      m_sprites(sprites),
      m_video_ram(video_ram),
      m_screen_buffer(screen_buffer)
{
}



bool age::gb_lcd_dot_renderer::in_progress() const
{
    return m_rendering_line.m_line >= 0;
}



void age::gb_lcd_dot_renderer::reset()
{
    m_rendering_line = gb_no_line;
}

void age::gb_lcd_dot_renderer::begin_new_line(gb_current_line line)
{
    AGE_ASSERT(!in_progress());
    m_rendering_line = {.m_line = line.m_line, .m_line_clks = 0};

    bool line_finished = continue_line(line);
    AGE_ASSERT(!line_finished);
    AGE_UNUSED(line_finished); // for release builds
}



bool age::gb_lcd_dot_renderer::continue_line(gb_current_line until)
{
    AGE_ASSERT(in_progress());
    AGE_ASSERT(until.m_line >= m_rendering_line.m_line);

    int line_clks = (until.m_line > m_rendering_line.m_line) ? gb_clock_cycles_per_lcd_line : until.m_line_clks;

    //! \todo implement dot rendering, the following is just dummy code

    if (line_clks >= gb_clock_cycles_per_lcd_line)
    {
        auto* dst   = &m_screen_buffer.get_back_buffer()[0] + m_rendering_line.m_line * gb_screen_width;
        auto  black = pixel{0, 0, 0, 255}.get_32bits();

        for (int i = 0; i < gb_screen_width; ++i)
        {
            dst->set_32bits(black);
            ++dst;
        }

        m_rendering_line = gb_no_line;
        return true;
    }

    m_rendering_line = until;
    return false;
}
