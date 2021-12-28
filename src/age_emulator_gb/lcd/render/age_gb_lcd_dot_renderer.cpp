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

#include <age_debug.hpp>

namespace
{
    constexpr int x_pos_first_px = 8;
    constexpr int x_pos_last_px  = age::gb_screen_width + x_pos_first_px - 1;

} // namespace

//! \todo handle LCDC bit 0



age::gb_lcd_dot_renderer::gb_lcd_dot_renderer(const gb_device&            device,
                                              const gb_lcd_render_common& common,
                                              const gb_lcd_palettes&      palettes,
                                              const gb_lcd_sprites&       sprites,
                                              const uint8_t*              video_ram,
                                              gb_window_check&            window,
                                              screen_buffer&              screen_buffer)
    : m_device(device),
      m_common(common),
      m_palettes(palettes),
      m_sprites(sprites),
      m_video_ram(video_ram),
      m_window(window),
      m_screen_buffer(screen_buffer)
{
}



bool age::gb_lcd_dot_renderer::in_progress() const
{
    return m_line.m_line >= 0;
}

bool age::gb_lcd_dot_renderer::stat_mode0() const
{
    return !in_progress() || m_mode3_finished;
}

void age::gb_lcd_dot_renderer::set_clks_tile_data_change(gb_current_line at_line)
{
    m_clks_tile_data_change = at_line;
}

void age::gb_lcd_dot_renderer::reset()
{
    m_line = gb_no_line;
}

void age::gb_lcd_dot_renderer::begin_new_line(gb_current_line line, bool is_first_frame)
{
    AGE_ASSERT(!in_progress())
    bool is_line_zero = is_first_frame && (line.m_line == 0);

    m_bg_win_fifo.clear();

    m_line                 = {.m_line = line.m_line, .m_line_clks = 0};
    m_line_stage           = line_stage::mode2;
    m_line_buffer          = &m_screen_buffer.get_back_buffer()[line.m_line * gb_screen_width];
    m_clks_begin_align_scx = is_line_zero ? 86 : 84;
    m_x_pos                = 0;
    m_x_pos_win_start      = int_max;
    m_mode3_finished       = false;
    // no need to reset all members
    // m_aligned_scx = 0;

    m_next_fetcher_step = fetcher_step::fetch_bg_win_tile_id;
    m_next_fetcher_clks = is_line_zero ? 87 : 85;
    // no need to reset all members
    // m_fetched_bg_win_tile_id         = 0;
    // m_fetched_bg_win_tile_attributes = 0;
    // m_fetched_bg_win_bitplane        = {};
    m_clks_tile_data_change = gb_no_line;

    bool line_finished = continue_line(line);
    AGE_ASSERT(!line_finished)
    AGE_UNUSED(line_finished); // unused in release builds
}

bool age::gb_lcd_dot_renderer::continue_line(gb_current_line until)
{
    AGE_ASSERT(in_progress())
    AGE_ASSERT(until.m_line >= m_line.m_line)

    int current_line_clks = (until.m_line > m_line.m_line)
                                ? gb_clock_cycles_per_lcd_line
                                : until.m_line_clks;

    while (m_next_fetcher_clks < current_line_clks)
    {
        update_line_stage(m_next_fetcher_clks); // may set m_next_fetcher_step = fetcher_step::finish_line
        if (m_next_fetcher_clks > current_line_clks)
        {
            break;
        }
        switch (m_next_fetcher_step)
        {
            case fetcher_step::fetch_bg_win_tile_id:
                fetch_bg_win_tile_id();
                schedule_next_fetcher_step(2, fetcher_step::fetch_bg_win_bitplane0);
                break;

            case fetcher_step::fetch_bg_win_bitplane0:
                fetch_bg_win_bitplane(0);
                schedule_next_fetcher_step(2, fetcher_step::fetch_bg_win_bitplane1);
                break;

            case fetcher_step::fetch_bg_win_bitplane1:
                fetch_bg_win_bitplane(1);
                push_bg_win_bitplanes();
                schedule_next_fetcher_step(4, fetcher_step::fetch_bg_win_tile_id);
                break;

            case fetcher_step::finish_line:
                reset();
                return true;
        }
    }

    AGE_ASSERT(m_next_fetcher_clks >= current_line_clks)
    update_line_stage(current_line_clks); //! \todo this may reset the fetcher => make it part of the loop
    AGE_ASSERT(m_line.m_line_clks < gb_clock_cycles_per_lcd_line)
    AGE_ASSERT(m_line.m_line_clks >= current_line_clks)
    return false;
}



void age::gb_lcd_dot_renderer::update_line_stage(int until_line_clks)
{
    AGE_ASSERT(until_line_clks <= gb_clock_cycles_per_lcd_line)
    // m_line.m_line_clks may be > until_line_clks
    // (e.g. when starting window rendering)
    AGE_ASSERT(m_line.m_line_clks <= gb_clock_cycles_per_lcd_line)

    while (m_line.m_line_clks < until_line_clks)
    {
        switch (m_line_stage)
        {
            case line_stage::mode2:
                line_stage_mode2(until_line_clks);
                break;

            case line_stage::mode3_align_scx:
                line_stage_mode3_align_scx(until_line_clks);
                break;

            case line_stage::mode3_render:
                line_stage_mode3_render(until_line_clks);
                break;

            case line_stage::rendering_finished:
                m_line.m_line_clks = until_line_clks;
                break;
        }
        // terminate loop early if the fetcher has been reset
        // (to initialize window or sprite rendering)
        if (m_next_fetcher_clks < m_line.m_line_clks)
        {
            break;
        }
    }

    AGE_ASSERT(m_line.m_line_clks >= until_line_clks)
}

void age::gb_lcd_dot_renderer::line_stage_mode2(int until_line_clks)
{
    AGE_ASSERT(m_line.m_line_clks < m_clks_begin_align_scx)
    m_line.m_line_clks = until_line_clks;
    if (m_line.m_line_clks >= m_clks_begin_align_scx)
    {
        m_line_stage       = line_stage::mode3_align_scx;
        m_line.m_line_clks = m_clks_begin_align_scx;
        // AGE_LOG("line " << m_line.m_line << " (" << m_line.m_line_clks << "):"
        //                 << " entering line_stage::mode3_align_scx"
        //                 << ", scx=" << (int) m_common.m_scx)
    }
}

void age::gb_lcd_dot_renderer::line_stage_mode3_align_scx(int until_line_clks)
{
    AGE_ASSERT(m_line.m_line_clks >= m_clks_begin_align_scx)
    uint8_t scx = m_common.m_scx & 0b111;

    for (int i = m_line.m_line_clks; i < until_line_clks; ++i)
    {
        int match = (m_line.m_line_clks - m_clks_begin_align_scx) & 0b111;
        ++m_line.m_line_clks;

        // scx match => next stage
        if (match == scx)
        {
            // we cannot discard the first X pixel now as the
            // pixel FIFO might still be empty
            // => save the number of pixel to discard for later
            //    (SCX might have been changed already when we
            //    discard the first X pixel)
            m_aligned_scx = scx;
            AGE_ASSERT(m_bg_win_fifo.size() <= 8)
            m_line_stage = line_stage::mode3_render;
            // AGE_LOG("line " << m_line.m_line << " (" << m_line.m_line_clks << "):"
            //                 << " entering line_stage::mode3_wait_for_initial_fifo_contents"
            //                 << ", scx=" << (int) m_common.m_scx
            //                 << ", bg-fifo-size=" << m_bg_win_fifo.size())
            break;
        }

        // still no match at scx 7 -> clear BG fifo and begin anew
        if (match == 0b111)
        {
            m_bg_win_fifo.clear();
        }
    }
}

void age::gb_lcd_dot_renderer::line_stage_mode3_render(int until_line_clks)
{
    AGE_ASSERT(m_line.m_line_clks > m_clks_begin_align_scx)
    AGE_ASSERT(m_x_pos <= x_pos_last_px)
    for (int i = m_line.m_line_clks; i < until_line_clks; ++i)
    {
        // set next pixel
        if (m_x_pos >= x_pos_first_px)
        {
            // align the pixel FIFO right before setting the first pixel
            if (m_x_pos == x_pos_first_px)
            {
                initial_fifo_alignment();
            }

            AGE_ASSERT(!m_bg_win_fifo.empty())
            auto color_idx = m_bg_win_fifo[0];
            m_bg_win_fifo.pop_front();
            m_line_buffer[m_x_pos - x_pos_first_px] = m_palettes.get_color(color_idx);
        }

        // next dot (keep x-pos & line-clks in sync)
        ++m_x_pos;
        ++m_line.m_line_clks;

        // last pixel plotted
        if (m_x_pos > x_pos_last_px)
        {
            m_line_stage        = line_stage::rendering_finished;
            m_next_fetcher_clks = gb_clock_cycles_per_lcd_line - 1;
            m_next_fetcher_step = fetcher_step::finish_line;
            m_mode3_finished    = true;
            // don't miss any WY match in case WX is out of the visible range
            m_window.check_for_wy_match(m_common.get_lcdc(), m_common.m_wy, m_line.m_line);
            // AGE_LOG("line " << m_line.m_line << " (" << m_line.m_line_clks << "):"
            //                 << " entering line_stage::rendering_finished")
            break;
        }

        // start window rendering on the upcoming pixel?
        if (check_start_window())
        {
            break;
        }

        // if the upcoming pixel is the last one,
        // and we can plot it on the next cycle (no window and no sprite interferes),
        // indicate that mode 3 is finished
        m_mode3_finished = m_x_pos == x_pos_last_px;
    }
}

void age::gb_lcd_dot_renderer::initial_fifo_alignment()
{
    // AGE_LOG("line " << m_line.m_line << " (" << m_line.m_line_clks << "):"
    //                 << " entering line_stage::mode3_render"
    //                 << ", bg-fifo-size=" << m_bg_win_fifo.size()
    //                 << ", discarding " << m_aligned_scx << " FIFO pixel")
    AGE_ASSERT(m_bg_win_fifo.size() >= 8)
    int skip = (m_x_pos_win_start < int_max) && m_window.is_enabled_and_wy_matched(m_common.get_lcdc())
                   ? 7 - m_common.m_wx
                   : m_aligned_scx;
    for (int p = 0; p < skip; ++p)
    {
        m_bg_win_fifo.pop_front();
    }
    m_line_stage = line_stage::mode3_render;
}

bool age::gb_lcd_dot_renderer::check_start_window()
{
    AGE_ASSERT(m_x_pos <= x_pos_last_px)
    if (m_x_pos == (m_common.m_wx + 1))
    {
        if (m_device.is_dmg_device() && (m_x_pos >= x_pos_last_px))
        {
            return false;
        }

        m_window.check_for_wy_match(m_common.get_lcdc(), m_common.m_wy, m_line.m_line);
        if ((m_x_pos_win_start == int_max) && m_window.is_enabled_and_wy_matched(m_common.get_lcdc()))
        {
            // AGE_LOG("line " << m_line.m_line << " (" << m_line.m_line_clks << "):"
            //                 << " initialize window rendering"
            //                 << ", x-pos = " << m_x_pos
            //                 << ", wx = " << (int) m_common.m_wx
            //                 << ", scx = " << (int) m_common.m_scx)
            m_bg_win_fifo.clear(); // is replaced with first window tile
            m_window.next_window_line();
            m_x_pos_win_start   = m_x_pos;
            m_next_fetcher_step = fetcher_step::fetch_bg_win_tile_id;
            m_next_fetcher_clks = m_line.m_line_clks;
            //! \todo this line is a workaround for x-pos==167, any better way?
            m_line.m_line_clks += (m_x_pos == x_pos_last_px) ? 5 : 6;

            // glitch on (WX == 0) && ((SCX & 7) != 0)
            if ((m_x_pos == 1) && (m_aligned_scx >= 1))
            {
                ++m_line.m_line_clks;
            }

            return true;
        }
    }
    return false;
}



void age::gb_lcd_dot_renderer::schedule_next_fetcher_step(int clks_offset, fetcher_step step)
{
    AGE_ASSERT(clks_offset > 0)
    m_next_fetcher_clks += clks_offset;
    m_next_fetcher_step = step;
    AGE_ASSERT(m_next_fetcher_clks < gb_clock_cycles_per_lcd_line)
}

void age::gb_lcd_dot_renderer::fetch_bg_win_tile_id()
{
    int tile_vram_ofs = 0;

    if ((m_x_pos_win_start <= m_x_pos) && m_window.is_enabled_and_wy_matched(m_common.get_lcdc()))
    {
        int tile_line_ofs = (m_x_pos - m_x_pos_win_start) >> 3;
        tile_vram_ofs     = m_common.m_bg_tile_map_offset + ((m_window.current_window_line() & 0b11111000) << 2) + tile_line_ofs;
    }
    else
    {
        int bg_y          = m_common.m_scy + m_line.m_line;
        int px_ofs        = ((m_x_pos < x_pos_first_px) || m_device.is_cgb_device()) ? 0 : 1;
        int tile_line_ofs = ((m_common.m_scx + m_x_pos + px_ofs) >> 3) & 0b11111;
        tile_vram_ofs     = m_common.m_bg_tile_map_offset + ((bg_y & 0b11111000) << 2) + tile_line_ofs;
    }

    m_fetched_bg_win_tile_id         = m_video_ram[tile_vram_ofs];          // bank 0
    m_fetched_bg_win_tile_attributes = m_video_ram[tile_vram_ofs + 0x2000]; // bank 1 (always zero for DMG)
}

void age::gb_lcd_dot_renderer::fetch_bg_win_bitplane(int bitplane_offset)
{
    AGE_ASSERT((bitplane_offset == 0) || (bitplane_offset == 1))

    // CGB tile data glitch
    if (m_clks_tile_data_change.m_line_clks == m_next_fetcher_clks)
    {
        AGE_ASSERT(m_device.is_cgb_device())
        if (!(m_common.get_lcdc() & gb_lcdc_bg_win_data))
        {
            m_fetched_bg_win_bitplane[bitplane_offset] = m_common.m_xflip_cache[m_fetched_bg_win_tile_id];
            return;
        }
    }

    int tile_line = ((m_x_pos_win_start <= m_x_pos) && m_window.is_enabled_and_wy_matched(m_common.get_lcdc()))
                        ? m_window.current_window_line() & 0b111
                        : (m_common.m_scy + m_line.m_line) & 0b111;
    // y-flip
    if (m_fetched_bg_win_tile_attributes & gb_tile_attrib_flip_y)
    {
        tile_line = 7 - tile_line;
    }

    // read tile data
    int tile_data_ofs = (m_fetched_bg_win_tile_id ^ m_common.m_tile_xor) << 4; // 16 bytes per tile
    tile_data_ofs += m_common.m_tile_data_offset;
    tile_data_ofs += (m_fetched_bg_win_tile_attributes & gb_tile_attrib_vram_bank) << 10;
    tile_data_ofs += tile_line * 2; // 2 bytes per line

    auto bitplane = m_video_ram[tile_data_ofs + bitplane_offset];

    // AGE_LOG("line " << m_line.m_line << " (" << m_line.m_line_clks << "):"
    //                 << " read bitplane " << bitplane_offset << " = " << (int) bitplane)

    // x-flip
    // (we invert the x-flip for easier rendering:
    // this way bit 0 is used for the leftmost pixel)
    m_fetched_bg_win_bitplane[bitplane_offset] = (m_fetched_bg_win_tile_attributes & gb_tile_attrib_flip_x)
                                                     ? bitplane
                                                     : m_common.m_xflip_cache[bitplane];
}

void age::gb_lcd_dot_renderer::push_bg_win_bitplanes()
{
    unsigned bitplane0 = m_fetched_bg_win_bitplane[0];
    unsigned bitplane1 = m_fetched_bg_win_bitplane[1];

    uint8_t palette_ofs = m_fetched_bg_win_tile_attributes & gb_tile_attrib_palette;
    palette_ofs <<= 2;

    bitplane1 <<= 1;
    for (int i = 0; i < 8; ++i)
    {
        m_bg_win_fifo.push_back((bitplane0 & 0b01U) + (bitplane1 & 0b10U) + palette_ofs);
        bitplane0 >>= 1;
        bitplane1 >>= 1;
    }
}
