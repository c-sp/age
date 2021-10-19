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
    return m_next_fetcher_clks.m_line >= 0;
}



void age::gb_lcd_dot_renderer::reset()
{
    m_next_fetcher_clks = gb_no_line;
}

void age::gb_lcd_dot_renderer::begin_new_line(gb_current_line line)
{
    AGE_ASSERT(!in_progress());
    m_next_fetcher_clks = {.m_line = line.m_line, .m_line_clks = 0};
    m_next_fetcher_hash = 0;

    bool line_finished = continue_line(line);
    AGE_ASSERT(!line_finished);
    AGE_UNUSED(line_finished); // for release builds
}



namespace
{
    //!
    //! see https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
    //!
    constexpr age::uint32_t fnv_hash(const char* str)
    {
        age::uint32_t hash = 2166136261U;
        while (*str)
        {
            hash = 16777619U * (hash ^ static_cast<age::uint32_t>(*str));
            ++str;
        }
        return hash;
    }

} // namespace



// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BEGIN_DEFERRED_EXECUTION \
    switch (m_next_fetcher_hash) \
    {                            \
        case 0: {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFER_NEXT(next_step_name, t4_cycles)                                    \
    m_next_fetcher_clks.m_line_clks += (t4_cycles);                              \
    AGE_ASSERT(m_next_fetcher_clks.m_line_clks <= gb_clock_cycles_per_lcd_line); \
    if (m_next_fetcher_clks.m_line_clks > line_clks)                             \
    {                                                                            \
        m_next_fetcher_hash = fnv_hash(#next_step_name);                         \
        return false; /* line not finished yet */                                \
    }                                                                            \
    } /* close previous case */                                                  \
    label_##next_step_name : case fnv_hash(#next_step_name) :                    \
    {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define END_DEFERRED_EXECUTION        \
    m_next_fetcher_clks = gb_no_line; \
    return true;                      \
    } /* close case */                \
    } /* close switch */

namespace
{
    void fill_buffer(age::pixel* buffer, int pixels, age::pixel color)
    {
        for (auto* max = buffer + pixels; buffer < max; ++buffer)
        {
            buffer->set_32bits(color.get_32bits());
        }
    }

} // namespace



bool age::gb_lcd_dot_renderer::continue_line(gb_current_line until)
{
    AGE_ASSERT(in_progress());
    AGE_ASSERT(until.m_line >= m_next_fetcher_clks.m_line);

    int line_clks = (until.m_line > m_next_fetcher_clks.m_line)
                        ? gb_clock_cycles_per_lcd_line
                        : until.m_line_clks;

    //! \todo implement dot rendering, the following is just dummy code

    BEGIN_DEFERRED_EXECUTION

    DEFER_NEXT(render_black, 80) // skip mode 2
    fill_buffer(&m_screen_buffer.get_back_buffer()[until.m_line * gb_screen_width], 40, pixel{0, 0, 0});

    DEFER_NEXT(render_red, 40)
    fill_buffer(&m_screen_buffer.get_back_buffer()[until.m_line * gb_screen_width + 40], 40, pixel{255, 0, 0});

    DEFER_NEXT(render_green, 40)
    fill_buffer(&m_screen_buffer.get_back_buffer()[until.m_line * gb_screen_width + 80], 40, pixel{0, 255, 0});

    DEFER_NEXT(render_blue, 40)
    fill_buffer(&m_screen_buffer.get_back_buffer()[until.m_line * gb_screen_width + 120], 40, pixel{0, 0, 255});

    DEFER_NEXT(finished, 256)

    END_DEFERRED_EXECUTION
}
