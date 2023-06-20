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

#include <algorithm>
#include <cassert>

namespace
{
    constexpr int x_pos_last_px = age::gb_screen_width + age::gb_x_pos_first_px - 1;

} // namespace



age::gb_lcd_fifo_renderer::gb_lcd_fifo_renderer(const gb_device&              device,
                                                const gb_lcd_renderer_common& common,
                                                const gb_lcd_palettes&        palettes,
                                                const gb_lcd_sprites&         sprites,
                                                std::span<uint8_t const>      video_ram,
                                                gb_window_check&              window,
                                                screen_buffer&                screen_buffer)
    : m_device(device),
      m_common(common),
      m_palettes(palettes),
      m_sprites(sprites),
      m_window(window),
      m_screen_buffer(screen_buffer),
      m_fetcher(device, common, video_ram, sprites, window)
{
}



bool age::gb_lcd_fifo_renderer::in_progress() const
{
    return (m_line.m_line >= 0) && (m_line.m_line_clks < gb_clock_cycles_per_lcd_line);
}

bool age::gb_lcd_fifo_renderer::stat_mode0() const
{
    return !in_progress() || (m_line_stage == line_stage::rendering_finished);
}

void age::gb_lcd_fifo_renderer::set_clks_tile_data_change([[maybe_unused]] gb_current_line at_line)
{
    if (in_progress())
    {
        assert(m_device.is_cgb_device());
        assert(m_line.m_line == at_line.m_line);
        assert(m_line.m_line_clks >= at_line.m_line_clks);
        m_fetcher.set_clks_tile_data_change(m_line.m_line_clks);
    }
}

void age::gb_lcd_fifo_renderer::set_clks_bgp_change(gb_current_line at_line)
{
    assert(m_device.is_dmg_device());
    m_clks_bgp_change = at_line;
}

void age::gb_lcd_fifo_renderer::reset()
{
    m_line = gb_no_line;
}

void age::gb_lcd_fifo_renderer::begin_new_line(gb_current_line line, bool is_first_frame)
{
    assert(!in_progress());
    bool is_line_zero = is_first_frame && (line.m_line == 0);

    //! \todo sprites should be searched step by step in mode 2
    m_sorted_sprites = m_sprites.get_line_sprites(line.m_line, true);
    // first sprite last in vector (so that we can easily pop_back() to the next sprite)
    std::reverse(m_sorted_sprites.begin(), m_sorted_sprites.end());
    m_next_sprite_x = m_sorted_sprites.empty() ? -1 : m_sorted_sprites.back().m_x;

    m_line                 = {.m_line = line.m_line, .m_line_clks = 0};
    m_line_stage           = line_stage::mode2;
    m_line_buffer          = &m_screen_buffer.get_back_buffer()[static_cast<size_t>(line.m_line) * gb_screen_width];
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
    assert(in_progress());
    assert(until.m_line >= m_line.m_line);

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

    assert(current_line_clks <= gb_clock_cycles_per_lcd_line);
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

    assert(m_fetcher.next_step_clks() >= current_line_clks);
    assert(m_line.m_line_clks <= gb_clock_cycles_per_lcd_line);
    assert(m_line.m_line_clks >= current_line_clks);
    return !in_progress();
}



void age::gb_lcd_fifo_renderer::update_line_stage(int until_line_clks)
{
    assert(until_line_clks <= m_fetcher.next_step_clks());
    assert(until_line_clks <= gb_clock_cycles_per_lcd_line);
    // m_line.m_line_clks may be > until_line_clks
    // (e.g. when starting window rendering)
    assert(m_line.m_line_clks <= gb_clock_cycles_per_lcd_line);

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
    assert(until_line_clks <= m_fetcher.next_step_clks());
    assert(m_line.m_line_clks <= m_fetcher.next_step_clks());
}

void age::gb_lcd_fifo_renderer::line_stage_mode2(int until_line_clks)
{
    assert(m_line.m_line_clks < m_clks_begin_align_scx);
    m_line.m_line_clks = until_line_clks;
    if (m_line.m_line_clks >= m_clks_begin_align_scx)
    {
        m_line_stage       = line_stage::mode3_align_scx;
        m_line.m_line_clks = m_clks_begin_align_scx;
    }
}

void age::gb_lcd_fifo_renderer::line_stage_mode3_align_scx(int until_line_clks)
{
    assert(m_line.m_line_clks >= m_clks_begin_align_scx);
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
            m_line_stage  = line_stage::mode3_render;
            break;
        }

        m_fetcher.pop_bg_dot();
    }
}

void age::gb_lcd_fifo_renderer::line_stage_mode3_render(int until_line_clks)
{
    assert(m_fetcher.next_step_clks() >= until_line_clks);
    assert(m_fetcher.next_step_clks() >= m_line.m_line_clks);
    assert(m_line.m_line_clks > m_clks_begin_align_scx);
    assert(m_x_pos <= x_pos_last_px);
    for (int i = m_line.m_line_clks; i < until_line_clks; ++i)
    {
        // fetch next sprite?
        if (fetch_next_sprite())
        {
            break;
        }

        // start window rendering on the next pixel?
        // (do this before increasing m_x_pos, otherwise we would initialize the window
        // in the last loop iteration too early as m_x_pos already points to the next pixel
        // handled after the last loop iteration)
        if (init_window())
        {
            // break the loop after this iteration
            until_line_clks = m_line.m_line_clks;
        }

        // check for DMG window reactivation glitch
        if (dmg_wx_glitch())
        {
            // fetcher restarted & zero-pixel inserted,
            // break the loop after this cycle
            until_line_clks = m_line.m_line_clks;
        }
        else
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
            break;
        }
    }
    assert(m_fetcher.next_step_clks() >= m_line.m_line_clks);
}

void age::gb_lcd_fifo_renderer::line_stage_mode3_init_window(int until_line_clks)
{
    assert(m_line.m_line_clks < m_clks_end_window_init);

    // window termination can occur only on the update's first cycle
    // as LCD registers are constant during continue_line()
    if (m_x_pos_win_start == int_max)
    {
        if (m_device.is_cgb_device() || ((m_clks_end_window_init - m_line.m_line_clks) >= 6))
        {
            m_line_stage = line_stage::mode3_render;
            m_line.m_line_clks += m_device.is_cgb_device() ? 1 : 0;
            // note that we keep m_alignment_x as the FIFO might run empty otherwise
            m_fetcher.restart_bg_fetch(m_line.m_line_clks);
            return;
        }
    }

    m_line.m_line_clks = until_line_clks;
    if (m_line.m_line_clks >= m_clks_end_window_init)
    {
        m_x_pos_win_start  = m_x_pos - 8;
        m_line_stage       = line_stage::mode3_render;
        m_line.m_line_clks = m_clks_end_window_init;
        m_fetcher.restart_bg_fetch(m_line.m_line_clks);
    }
}

void age::gb_lcd_fifo_renderer::line_stage_mode3_wait_for_sprite(int until_line_clks)
{
    m_line.m_line_clks = until_line_clks;
    if (m_line.m_line_clks >= m_fetcher.clks_last_sprite_finished())
    {
        m_line_stage       = line_stage::mode3_render;
        m_line.m_line_clks = m_fetcher.clks_last_sprite_finished() + 1;
    }
}



void age::gb_lcd_fifo_renderer::plot_pixel()
{
    auto bg_color = m_fetcher.pop_bg_dot();
    auto sp_dot   = m_fetcher.pop_sp_dot();

    if (m_x_pos >= gb_x_pos_first_px)
    {
        uint8_t bg_prio_flag = (m_fetcher.get_bg_attributes() | sp_dot.m_attributes) & gb_tile_attrib_priority;
        uint8_t prio         = ((bg_color & 0b11) | bg_prio_flag) & m_common.m_priority_mask;

        pixel color;
        if ((prio <= 0x80) && (sp_dot.m_color & 0b11))
        {
            color = m_palettes.get_color(sp_dot.m_color);
        }
        else
        {
            // BGP glitch not on the first pixel, see age-test-roms/m3-bg-bgp
            bool bgp_glitch = (m_clks_bgp_change.m_line_clks == m_line.m_line_clks + 1)
                              && (m_x_pos > gb_x_pos_first_px);

            color = bgp_glitch ? m_palettes.get_color_bgp_glitch(bg_color)
                               : m_palettes.get_color(bg_color);
        }

        m_line_buffer[m_x_pos - gb_x_pos_first_px] = color;
    }
}

bool age::gb_lcd_fifo_renderer::dmg_wx_glitch()
{
    // check for WX zero pixel glitch
    // (see mealybug tearoom tests: m3_wx_4_change, m3_wx_5_change, m3_wx_6_change)
    if (!m_device.is_dmg_device()
        || (m_x_pos_win_start >= m_x_pos - 8) // not for the actual window activation
        || (m_x_pos != m_common.m_wx + 1)
        || (m_fetcher.clks_last_bg_name_fetch() != m_line.m_line_clks))
    {
        return false;
    }

    m_line_buffer[m_x_pos - gb_x_pos_first_px] = m_palettes.get_color_zero_dmg();
    m_fetcher.restart_bg_fetch(m_line.m_line_clks + 1);
    return true;
}

bool age::gb_lcd_fifo_renderer::init_window()
{
    assert(m_x_pos <= x_pos_last_px);
    if (m_x_pos != m_common.m_wx)
    {
        return false;
    }
    if (m_device.is_dmg_device() && (m_x_pos >= x_pos_last_px - 1))
    {
        //! \todo (dmg) this enables the window on the next line!
        return false;
    }

    m_window.check_for_wy_match(m_common.get_lcdc(), m_common.m_wy, m_line.m_line);
    if ((m_x_pos_win_start != int_max) || !m_window.is_enabled_and_wy_matched(m_common.get_lcdc()))
    {
        return false;
    }

    // glitch on (WX == 0) && ((SCX & 7) != 0)
    int clks_init_window = ((m_common.m_wx == 0) && (m_alignment_x >= 1)) ? 8 : 7;

    m_window.next_window_line();
    m_x_pos_win_start      = m_x_pos;                       // for first tile fetch
    m_alignment_x          = 7 - m_common.m_wx;
    m_line_stage           = line_stage::mode3_init_window; // break loop after this pixel
    m_clks_end_window_init = m_line.m_line_clks + clks_init_window;
    m_fetcher.restart_bg_fetch(m_line.m_line_clks + 2);

    return true;
}

bool age::gb_lcd_fifo_renderer::fetch_next_sprite()
{
    if (m_next_sprite_x != m_x_pos)
    {
        return false;
    }

    if (m_device.is_dmg_device() && !are_sprites_enabled(m_common.get_lcdc()))
    {
        while (m_next_sprite_x == m_x_pos)
        {
            m_sorted_sprites.pop_back();
            m_next_sprite_x = m_sorted_sprites.empty() ? -1 : m_sorted_sprites.back().m_x;
        }
        return false;
    }

    int spx0_delay = (m_x_pos == 0) ? std::min(m_alignment_x & 0b111, 5) : 0;

    assert(!m_sorted_sprites.empty());

    // trigger sprite fetch
    m_line_stage = line_stage::mode3_wait_for_sprite;
    auto sprite  = m_sorted_sprites.back();
    m_fetcher.trigger_sprite_fetch(sprite.m_sprite_id, sprite.m_x, m_line.m_line_clks, spx0_delay);

    // watch out for the next sprite
    m_sorted_sprites.pop_back();
    m_next_sprite_x = m_sorted_sprites.empty() ? -1 : m_sorted_sprites.back().m_x;

    return true;
}
