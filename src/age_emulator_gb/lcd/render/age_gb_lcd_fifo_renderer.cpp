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

#include "age_gb_lcd_fifo_renderer.hpp"

#include <age_debug.hpp>

#include <algorithm>

namespace
{
    constexpr int x_pos_last_px = age::gb_screen_width + age::gb_x_pos_first_px - 1;

    age::uint8_t pop_sp_pixel(age::gb_sp_fifo& sp_fifo)
    {
        if (sp_fifo.empty())
        {
            return 0;
        }
        auto color = sp_fifo[0].m_color;
        sp_fifo.pop_front();
        return color;
    }

} // namespace

//! \todo handle LCDC bit 0

//! \todo maybe replace this with gb_logger
#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif


age::gb_lcd_fifo_renderer::gb_lcd_fifo_renderer(const gb_device&              device,
                                                const gb_lcd_renderer_common& common,
                                                const gb_lcd_palettes&        palettes,
                                                const gb_lcd_sprites&         sprites,
                                                const uint8_t*                video_ram,
                                                gb_window_check&              window,
                                                screen_buffer&                screen_buffer)
    : m_device(device),
      m_common(common),
      m_palettes(palettes),
      m_sprites(sprites),
      m_window(window),
      m_screen_buffer(screen_buffer),
      m_fetcher(device, common, video_ram, sprites, window, m_bg_fifo, m_sp_fifo)
{
}



bool age::gb_lcd_fifo_renderer::in_progress() const
{
    return (m_line.m_line >= 0) && (m_line.m_line_clks < gb_clock_cycles_per_lcd_line);
}

bool age::gb_lcd_fifo_renderer::stat_mode0() const
{
    return !in_progress() || m_fetcher.during_mode0();
}

void age::gb_lcd_fifo_renderer::set_clks_tile_data_change(gb_current_line at_line)
{
    if (in_progress())
    {
        m_fetcher.set_clks_tile_data_change(at_line);
    }
}

void age::gb_lcd_fifo_renderer::set_clks_bgp_change(gb_current_line at_line)
{
    AGE_ASSERT(m_device.is_dmg_device())
    m_clks_bgp_change = at_line;
}

void age::gb_lcd_fifo_renderer::reset()
{
    m_line = gb_no_line;
}

void age::gb_lcd_fifo_renderer::begin_new_line(gb_current_line line, bool is_first_frame)
{
    AGE_ASSERT(!in_progress())
    bool is_line_zero = is_first_frame && (line.m_line == 0);

    //! \todo sprites should be searched step by step in mode 2
    m_sorted_sprites = m_sprites.get_line_sprites(line.m_line, true);
    // first sprite last in vector (so that we can easily pop_back() to the next sprite)
    std::reverse(m_sorted_sprites.begin(), m_sorted_sprites.end());
    m_next_sprite_x = m_sorted_sprites.empty() ? -1 : m_sorted_sprites.back().m_x;

    m_bg_fifo.clear();

    m_line                 = {.m_line = line.m_line, .m_line_clks = 0};
    m_line_stage           = line_stage::mode2;
    m_line_buffer          = &m_screen_buffer.get_back_buffer()[line.m_line * gb_screen_width];
    m_clks_begin_align_scx = is_line_zero ? 86 : 84;
    m_clks_end_window_init = gb_no_clock_cycle;
    m_x_pos                = 0;
    m_x_pos_win_start      = int_max;
    m_clks_bgp_change      = gb_no_line;
    // no need to reset all members
    // m_alignment_scx = 0;

    m_fetcher.init_for_line(line.m_line, is_line_zero);

    continue_line(line);
}

bool age::gb_lcd_fifo_renderer::continue_line(gb_current_line until)
{
    AGE_ASSERT(in_progress())
    AGE_ASSERT(until.m_line >= m_line.m_line)

    // deactivate window?
    // (LCD registers are constant during continue_line(), so we can check this upfront)
    if (!is_window_enabled(m_common.get_lcdc()))
    {
        m_x_pos_win_start = int_max;
    }

    // finish line?
    int current_line_clks = (until.m_line > m_line.m_line)
                                ? gb_clock_cycles_per_lcd_line
                                : until.m_line_clks;

    LOG("line " << m_line.m_line << " (" << m_line.m_line_clks << "):"
                << " update until clks < " << current_line_clks
                << ", window enabled = " << is_window_enabled(m_common.get_lcdc()))

    AGE_ASSERT(current_line_clks <= gb_clock_cycles_per_lcd_line)
    while (std::min(m_fetcher.next_step_clks(), m_line.m_line_clks) < current_line_clks)
    {
        // update_line_stage() may modify the next fetcher cycle.
        // We have to continue the loop until the fetcher has caught up.
        // As m_line_clks might already be equal to current_line_clks,
        // the std::min() in the loop condition is required.

        // Update until right before the next fetcher step.
        // The fetcher always comes first (before plotting the pixel on that cycle).
        update_line_stage(std::min(m_fetcher.next_step_clks(), current_line_clks));

        if (m_fetcher.next_step_clks() < current_line_clks)
        {
            m_fetcher.execute_next_step(m_x_pos, m_x_pos_win_start);
        }
    }

    AGE_ASSERT(m_fetcher.next_step_clks() >= current_line_clks)
    AGE_ASSERT(m_line.m_line_clks <= gb_clock_cycles_per_lcd_line)
    AGE_ASSERT(m_line.m_line_clks >= current_line_clks)
    return !in_progress();
}



void age::gb_lcd_fifo_renderer::update_line_stage(int until_line_clks)
{
    AGE_ASSERT(until_line_clks <= m_fetcher.next_step_clks())
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

            case line_stage::mode3_init_window:
                line_stage_mode3_init_window(until_line_clks);
                break;

            case line_stage::mode3_wait_for_sprite:
                line_stage_mode3_wait_for_sprite(until_line_clks);
                break;

            case line_stage::rendering_finished:
                m_line.m_line_clks = until_line_clks;
                break;
        }
        // terminate loop early if the fetcher has been reset
        // (to initialize window or sprite rendering)
        until_line_clks = std::min(m_fetcher.next_step_clks(), until_line_clks);
    }

    // the next fetcher step might have been changed during the upper loop
    AGE_ASSERT(until_line_clks <= m_fetcher.next_step_clks())
    AGE_ASSERT(m_line.m_line_clks <= m_fetcher.next_step_clks())
}

void age::gb_lcd_fifo_renderer::line_stage_mode2(int until_line_clks)
{
    AGE_ASSERT(m_line.m_line_clks < m_clks_begin_align_scx)
    m_line.m_line_clks = until_line_clks;
    if (m_line.m_line_clks >= m_clks_begin_align_scx)
    {
        m_line_stage       = line_stage::mode3_align_scx;
        m_line.m_line_clks = m_clks_begin_align_scx;
        LOG("line " << m_line.m_line << " (" << m_line.m_line_clks << "):"
                    << " entering line_stage::mode3_align_scx"
                    << ", scx=" << (int) m_common.m_scx)
    }
}

void age::gb_lcd_fifo_renderer::line_stage_mode3_align_scx(int until_line_clks)
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
            m_alignment_x = scx;
            AGE_ASSERT(m_bg_fifo.size() <= 8)
            m_line_stage = line_stage::mode3_render;
            LOG("line " << m_line.m_line << " (" << m_line.m_line_clks << "):"
                        << " entering line_stage::mode3_render"
                        << ", scx=" << (int) m_common.m_scx
                        << ", bg-fifo-size=" << m_bg_fifo.size())
            break;
        }

        // still no match at scx 7 -> clear BG fifo and begin anew
        if (match == 0b111)
        {
            m_bg_fifo.clear();
        }
    }
}

void age::gb_lcd_fifo_renderer::line_stage_mode3_render(int until_line_clks)
{
    AGE_ASSERT(m_fetcher.next_step_clks() >= until_line_clks)
    AGE_ASSERT(m_fetcher.next_step_clks() >= m_line.m_line_clks)
    AGE_ASSERT(m_line.m_line_clks > m_clks_begin_align_scx)
    AGE_ASSERT(m_x_pos <= x_pos_last_px)
    for (int i = m_line.m_line_clks; i < until_line_clks; ++i)
    {
        // fetch next sprite?
        if (m_next_sprite_x == m_x_pos)
        {
            AGE_ASSERT(!m_sorted_sprites.empty())
            LOG("line " << m_line.m_line << " (" << m_line.m_line_clks << "):"
                        << " prepare fetching sprite #" << (int) m_sorted_sprites[0].m_sprite_id
                        << ", x-pos = " << m_x_pos
                        << ", scx = " << (int) m_common.m_scx
                        << ", next-fetcher-step " << m_fetcher.next_step()
                        << ", next-fetcher-clks " << m_fetcher.next_step_clks())

            // trigger sprite fetch
            m_line_stage   = line_stage::mode3_wait_for_sprite;
            int spx0_delay = (m_x_pos == 0) ? std::min(m_alignment_x & 0b111, 5) : 0;
            m_fetcher.trigger_sprite_fetch(m_sorted_sprites.back().m_sprite_id, m_line.m_line_clks, spx0_delay);

            // next sprite
            m_sorted_sprites.pop_back();
            m_next_sprite_x = m_sorted_sprites.empty() ? -1 : m_sorted_sprites.back().m_x;
            break;
        }

        // start window rendering on this pixel?
        // (do this before increasing m_x_pos, otherwise we would initialize the window
        // in the last loop iteration too early as m_x_pos already points to the next pixel
        // handled after the last loop iteration)
        if (init_window())
        {
            // glitch on (WX == 0) && ((SCX & 7) != 0)
            int clks_init_window = ((m_common.m_wx == 0) && (m_alignment_x >= 1)) ? 8 : 7;

            m_window.next_window_line();
            m_x_pos_win_start      = m_x_pos - 6;
            m_alignment_x          = 7 - m_common.m_wx;
            m_line_stage           = line_stage::mode3_init_window; // loop terminated after this pixel
            m_clks_end_window_init = m_line.m_line_clks + clks_init_window;
            m_fetcher.restart_bg_win_fetch(m_line.m_line_clks + 2);

            LOG("line " << m_line.m_line << " (" << m_line.m_line_clks << "):"
                        << " initialize window for next pixel"
                        << ", x-pos = " << m_x_pos
                        << ", wx = " << (int) m_common.m_wx
                        << ", scx = " << (int) m_common.m_scx)
        }

        // plot current pixel
        if (m_x_pos >= gb_x_pos_first_px)
        {
            plot_pixel();
        }

        // next pixel (keep x-pos & line-clks in sync)
        ++m_x_pos;
        ++m_line.m_line_clks;

        // last pixel plotted
        if (m_x_pos > x_pos_last_px)
        {
            m_line_stage = line_stage::rendering_finished;
            m_fetcher.finish_rendering();
            // don't miss any WY match in case WX is out of the visible range
            // as init_window() verifies WY only if WX matches
            m_window.check_for_wy_match(m_common.get_lcdc(), m_common.m_wy, m_line.m_line);
            LOG("line " << m_line.m_line << " (" << m_line.m_line_clks << "):"
                        << " entering line_stage::rendering_finished")
            break;
        }

        // terminate loop on window init
        if (m_line_stage == line_stage::mode3_init_window)
        {
            m_bg_fifo.clear(); // any pending tile will be replaced by the window's first tile
            break;
        }
    }
    AGE_ASSERT(m_fetcher.next_step_clks() >= m_line.m_line_clks)
}

void age::gb_lcd_fifo_renderer::line_stage_mode3_init_window(int until_line_clks)
{
    AGE_ASSERT(m_line.m_line_clks < m_clks_end_window_init)

    // window termination can occur only on the update's first cycle
    // as LCD registers are constant during continue_line()
    if (m_x_pos_win_start == int_max)
    {
        if (m_device.is_cgb_device() || ((m_clks_end_window_init - m_line.m_line_clks) >= 6))
        {
            LOG("line " << m_line.m_line << " (" << m_line.m_line_clks << "): terminate window init"
                        << " (would have been finished on line cycle " << m_clks_end_window_init << ")")

            m_line_stage = line_stage::mode3_render;
            m_line.m_line_clks += m_device.is_cgb_device() ? 1 : 0;
            m_bg_fifo.clear();
            // note that we keep m_alignment_x as the FIFO might run empty otherwise
            //! \todo restart previously fetched tile, is this correct?
            m_fetcher.restart_bg_win_fetch(m_line.m_line_clks, true);
            return;
        }
    }

    m_line.m_line_clks = until_line_clks;
    if (m_line.m_line_clks >= m_clks_end_window_init)
    {
        m_line_stage       = line_stage::mode3_render;
        m_line.m_line_clks = m_clks_end_window_init;
        m_fetcher.restart_bg_win_fetch(m_line.m_line_clks + 1);
        LOG("line " << m_line.m_line << " (" << m_line.m_line_clks << "): finish window init"
                    << ", fifo size = " << m_bg_fifo.size())
    }
}

void age::gb_lcd_fifo_renderer::line_stage_mode3_wait_for_sprite(int until_line_clks)
{
    m_line.m_line_clks = until_line_clks;
    if (m_line.m_line_clks >= m_fetcher.last_sprite_finished_clks())
    {
        m_line_stage       = line_stage::mode3_render;
        m_line.m_line_clks = m_fetcher.last_sprite_finished_clks() + 1;
        LOG("line " << m_line.m_line << " (" << m_line.m_line_clks << "): finished sprite fetching, continue rendering")
    }
}



void age::gb_lcd_fifo_renderer::plot_pixel()
{
    // align the pixel FIFO right before setting the first pixel
    // (the first N pixel are discarded based on SCX or WX)
    bool bgp_glitch = false;
    if (m_x_pos == gb_x_pos_first_px)
    {
        LOG("line " << m_line.m_line << " (" << m_line.m_line_clks << "): first pixel"
                    << ", bg-fifo-size=" << m_bg_fifo.size()
                    << ", discarding " << m_alignment_x << " FIFO pixel")
        AGE_ASSERT(m_bg_fifo.size() >= 8)
        for (int p = 0; p < m_alignment_x; ++p)
        {
            m_bg_fifo.pop_front();
        }
    }
    else
    {
        // not on the first pixel, see age-test-roms/m3-bg-bgp
        bgp_glitch = m_clks_bgp_change.m_line_clks == m_line.m_line_clks + 1;
    }

    // check for WX zero pixel glitch
    // (see mealybug tearoom tests: m3_wx_4_change, m3_wx_5_change, m3_wx_6_change)
    bool wx_glitch = m_device.is_dmg_device()
                     && (m_x_pos_win_start < m_x_pos - 8) // not for the actual window activation
                     && (m_x_pos == m_common.m_wx + 1)
                     && (m_fetcher.next_step_clks() == m_line.m_line_clks + 1)
                     && m_fetcher.next_step_fetches_tile_id();
    if (wx_glitch)
    {
        m_line_buffer[m_x_pos - gb_x_pos_first_px] = m_palettes.get_color_zero_dmg();
    }
    else
    {
        AGE_ASSERT(!m_bg_fifo.empty())
        auto sp_color = pop_sp_pixel(m_sp_fifo);
        auto color    = (sp_color & 0b11)
                            ? m_palettes.get_color(sp_color)
                        : bgp_glitch
                            ? m_palettes.get_color_bgp_glitch(m_bg_fifo[0])
                            : m_palettes.get_color(m_bg_fifo[0]);
        m_bg_fifo.pop_front();
        m_line_buffer[m_x_pos - gb_x_pos_first_px] = color;
    }
}

bool age::gb_lcd_fifo_renderer::init_window()
{
    AGE_ASSERT(m_x_pos <= x_pos_last_px)
    if (m_x_pos == m_common.m_wx)
    {
        //! \todo CGB glitch, does not work yet
        if (m_device.is_cgb_device() && m_window.window_switched_off_on(m_line))
        {
            auto num_px = m_bg_fifo.size();
            m_bg_fifo.clear();
            for (size_t i = 0; i < num_px; ++i)
            {
                m_bg_fifo.push_back(0);
            }
            return false;
        }

        if (m_device.is_dmg_device() && (m_x_pos >= x_pos_last_px - 1))
        {
            //! \todo (dmg) this enables the window on the next line!
            return false;
        }

        m_window.check_for_wy_match(m_common.get_lcdc(), m_common.m_wy, m_line.m_line);
        if ((m_x_pos_win_start == int_max) && m_window.is_enabled_and_wy_matched(m_common.get_lcdc()))
        {
            return true;
        }
    }
    return false;
}
