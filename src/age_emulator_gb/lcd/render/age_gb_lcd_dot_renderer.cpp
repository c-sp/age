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
    return m_fetcher_clks.m_line >= 0;
}



void age::gb_lcd_dot_renderer::reset()
{
    m_fetcher_clks = gb_no_line;
}

void age::gb_lcd_dot_renderer::begin_new_line(gb_current_line line)
{
    AGE_ASSERT(!in_progress())

    m_bg_fifo.clear();
    m_fifo_clks    = {.m_line = line.m_line, .m_line_clks = gb_no_clock_cycle}; // initialized after the first tile has been fetched
    m_current_line = &m_screen_buffer.get_back_buffer()[line.m_line * gb_screen_width];
    m_next_dot_idx = 0;

    m_fetcher_clks          = {.m_line = line.m_line, .m_line_clks = 0};
    m_fetcher_step_hash     = 0;
    m_next_bg_tile_to_fetch = 0;
    // no need to reset the following members
    // m_fetched_tile_id         = 0;
    // m_fetched_tile_attributes = 0;
    // m_fetched_bitplane        = {};

    bool line_finished = continue_line(line);
    AGE_ASSERT(!line_finished)
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



void age::gb_lcd_dot_renderer::fetch_bg_tile_id()
{
    int bg_y          = m_common.m_scy + m_fetcher_clks.m_line;
    int tile_line_ofs = ((m_common.m_scx >> 3) + m_next_bg_tile_to_fetch) & 0b11111;
    int tile_vram_ofs = m_common.m_bg_tile_map_offset + ((bg_y & 0b11111000) << 2) + tile_line_ofs;

    m_fetched_tile_id         = m_video_ram[tile_vram_ofs] ^ m_common.m_tile_xor; // bank 0
    m_fetched_tile_attributes = m_video_ram[tile_vram_ofs + 0x2000];              // bank 1 (always zero for DMG)
}



void age::gb_lcd_dot_renderer::fetch_bg_bitplane(int bitplane_offset)
{
    AGE_ASSERT((bitplane_offset == 0) || (bitplane_offset == 1));

    int tile_line = (m_common.m_scy + m_fetcher_clks.m_line) & 0b111;
    // y-flip
    if (m_fetched_tile_attributes & gb_tile_attrib_flip_y)
    {
        tile_line = 7 - tile_line;
    }

    // read tile data
    int tile_data_ofs = m_fetched_tile_id << 4; // 16 bytes per tile
    tile_data_ofs += m_common.m_tile_data_offset;
    tile_data_ofs += (m_fetched_tile_attributes & gb_tile_attrib_vram_bank) << 10;
    tile_data_ofs += tile_line << 1; // 2 bytes per line

    auto bitplane = m_video_ram[tile_data_ofs + bitplane_offset];

    // x-flip
    // (we invert the x-flip for easier rendering:
    // this way bit 0 is used for the leftmost pixel)
    m_fetched_bitplane[bitplane_offset] = (m_fetched_tile_attributes & gb_tile_attrib_flip_x)
                                              ? bitplane
                                              : m_common.m_xflip_cache[bitplane];
}



void age::gb_lcd_dot_renderer::push_bg_bitplanes()
{
    int bitplane0 = m_fetched_bitplane[0];
    int bitplane1 = m_fetched_bitplane[1];

    uint8_t palette_ofs = (m_fetched_tile_attributes & gb_tile_attrib_palette) << 2;

    bitplane1 <<= 1;
    for (int i = 0; i < 8; ++i)
    {
        m_bg_fifo.push_back((bitplane0 & 0b01) + (bitplane1 & 0b10) + palette_ofs);
        bitplane0 >>= 1;
        bitplane1 >>= 1;
    }

    ++m_next_bg_tile_to_fetch;
}



void age::gb_lcd_dot_renderer::render_dots(int current_line_clks)
{
    if (m_fifo_clks.m_line_clks == gb_no_clock_cycle)
    {
        return;
    }
    AGE_ASSERT((m_fifo_clks.m_line_clks >= 0) && (m_fifo_clks.m_line_clks < gb_clock_cycles_per_lcd_line));

    for (int i = m_fifo_clks.m_line_clks; (i < current_line_clks) && !m_bg_fifo.empty(); ++i)
    {
        auto color_idx = m_bg_fifo[0];
        m_bg_fifo.pop_front();
        m_current_line[m_next_dot_idx] = m_palettes.get_color(color_idx);

        ++m_next_dot_idx;

        if (m_next_dot_idx == gb_screen_width)
        {
            m_fifo_clks.m_line_clks    = gb_no_clock_cycle; // no more dots on this line
            m_fetcher_clks.m_line_clks = gb_clock_cycles_per_lcd_line;
            m_fetcher_step_hash        = fnv_hash("line_finished");
            return;
        }
    }

    m_fifo_clks.m_line_clks = current_line_clks;
}



// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BEGIN_DEFERRED_EXECUTION \
    switch (m_fetcher_step_hash) \
    {                            \
        case 0: {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFER_NEXT(next_step_name, t4_cycles)                                \
    /* calculate the next fetcher step's clock cycle */                      \
    m_fetcher_clks.m_line_clks += (t4_cycles);                               \
    AGE_ASSERT(m_fetcher_clks.m_line_clks <= gb_clock_cycles_per_lcd_line)   \
    /* check if we can immediately execute the next fetcher step */          \
    if (m_fetcher_clks.m_line_clks > current_line_clks)                      \
    {                                                                        \
        m_fetcher_step_hash = fnv_hash(#next_step_name);                     \
        render_dots(current_line_clks); /* may adjust m_fetcher_step_hash */ \
        return false;                   /* line not finished yet */          \
    }                                                                        \
    } /* close previous case and fall through to the next step */            \
    label_##next_step_name : case fnv_hash(#next_step_name) :                \
    {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFER_GOTO(goto_step_name, t4_cycles)                                \
    /* calculate the fetcher step's clock cycle */                           \
    m_fetcher_clks.m_line_clks += (t4_cycles);                               \
    AGE_ASSERT(m_fetcher_clks.m_line_clks <= gb_clock_cycles_per_lcd_line)   \
    /* check if we can immediately goto the specified fetcher step */        \
    if (m_fetcher_clks.m_line_clks > current_line_clks)                      \
    {                                                                        \
        m_fetcher_step_hash = fnv_hash(#goto_step_name);                     \
        render_dots(current_line_clks); /* may adjust m_fetcher_step_hash */ \
        return false;                   /* line not finished yet */          \
    }                                                                        \
    goto label_##goto_step_name;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define END_DEFERRED_EXECUTION                                                           \
    AGE_ASSERT(m_fetcher_clks.m_line_clks < gb_clock_cycles_per_lcd_line)                \
    DEFER_NEXT(line_finished, gb_clock_cycles_per_lcd_line - m_fetcher_clks.m_line_clks) \
    render_dots(current_line_clks);                                                      \
    break;                                                                               \
    } /* close case */                                                                   \
    default: {                                                                           \
        AGE_ASSERT(false)                                                                \
    }                                                                                    \
        } /* close switch */



#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-label" /* suppress "unused label" warning */

bool age::gb_lcd_dot_renderer::continue_line(gb_current_line until)
{
    AGE_ASSERT(in_progress())
    AGE_ASSERT(until.m_line >= m_fetcher_clks.m_line)

    int current_line_clks = (until.m_line > m_fetcher_clks.m_line)
                                ? gb_clock_cycles_per_lcd_line
                                : until.m_line_clks;

    if (m_fetcher_clks.m_line_clks > current_line_clks)
    {
        render_dots(current_line_clks);
        return false;
    }

    BEGIN_DEFERRED_EXECUTION
    //! \todo mode 2 emulation

    DEFER_NEXT(first_fetch_bg_tile_id, 85)
    fetch_bg_tile_id();
    DEFER_NEXT(first_fetch_bg_bitplane0, 2);
    fetch_bg_bitplane(0);
    DEFER_NEXT(first_fetch_bg_bitplane1, 2);
    fetch_bg_bitplane(1);
    DEFER_NEXT(first_push_bg_bitplanes, 2);
    push_bg_bitplanes();
    m_fifo_clks.m_line_clks = m_common.m_scx & 0b111;
    for (int i = 0; i < m_fifo_clks.m_line_clks; ++i)
    {
        m_bg_fifo.pop_front();
    }

    DEFER_NEXT(fetch_bg_tile_id, 2)
    fetch_bg_tile_id();
    DEFER_NEXT(fetch_bg_bitplane0, 2);
    fetch_bg_bitplane(0);
    DEFER_NEXT(fetch_bg_bitplane1, 2);
    fetch_bg_bitplane(1);
    DEFER_NEXT(push_bg_bitplanes, 2);
    push_bg_bitplanes();
    if (m_next_bg_tile_to_fetch < 21)
    {
        DEFER_GOTO(fetch_bg_tile_id, 2);
    }

    END_DEFERRED_EXECUTION
    m_fetcher_clks = gb_no_line;
    return true;
}

#pragma clang diagnostic pop
