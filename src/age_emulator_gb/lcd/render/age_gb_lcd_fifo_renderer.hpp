//
// Copyright 2022 Christoph Sprenger
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

#ifndef AGE_GB_LCD_FIFO_RENDERER_HPP
#define AGE_GB_LCD_FIFO_RENDERER_HPP

//!
//! \file
//!

#include "../../common/age_gb_device.hpp"
#include "../palettes/age_gb_lcd_palettes.hpp"
#include "age_gb_lcd_fifo_fetcher.hpp"
#include "age_gb_lcd_renderer_common.hpp"
#include "age_gb_lcd_sprites.hpp"
#include "age_gb_lcd_window_check.hpp"

#include <age_types.hpp>
#include <gfx/age_pixel.hpp>
#include <gfx/age_screen_buffer.hpp>

#include <deque>
#include <span>



namespace age
{

    class gb_lcd_fifo_renderer
    {
        AGE_DISABLE_COPY(gb_lcd_fifo_renderer);
        AGE_DISABLE_MOVE(gb_lcd_fifo_renderer);

    public:
        gb_lcd_fifo_renderer(const gb_device&              device,
                             const gb_lcd_renderer_common& common,
                             const gb_lcd_palettes&        palettes,
                             const gb_lcd_sprites&         sprites,
                             std::span<uint8_t const>      video_ram,
                             gb_window_check&              window,
                             screen_buffer&                screen_buffer);

        ~gb_lcd_fifo_renderer() = default;

        [[nodiscard]] bool in_progress() const;
        [[nodiscard]] bool stat_mode0() const;

        void set_clks_tile_data_change(gb_current_line at_line);
        void set_clks_bgp_change(gb_current_line at_line);
        void reset();
        void begin_new_line(gb_current_line line, bool is_first_frame);
        bool continue_line(gb_current_line until);

    private:
        enum class line_stage
        {
            mode2,
            mode3_align_scx,
            mode3_render,
            mode3_init_window,
            mode3_wait_for_sprite,
            rendering_finished,
        };
        void update_line_stage(int until_line_clks);
        void line_stage_mode2(int until_line_clks);
        void line_stage_mode3_align_scx(int until_line_clks);
        void line_stage_mode3_render(int until_line_clks);
        void line_stage_mode3_init_window(int until_line_clks);
        void line_stage_mode3_wait_for_sprite(int until_line_clks);
        void plot_pixel();
        bool dmg_wx_glitch();
        bool init_window();
        bool fetch_next_sprite();

        const gb_device&              m_device;
        const gb_lcd_renderer_common& m_common;
        const gb_lcd_palettes&        m_palettes;
        const gb_lcd_sprites&         m_sprites;
        gb_window_check&              m_window;
        screen_buffer&                m_screen_buffer;

        std::vector<gb_sprite> m_sorted_sprites{};
        gb_lcd_fifo_fetcher    m_fetcher;

        gb_current_line m_line                 = gb_no_line;
        line_stage      m_line_stage           = line_stage::mode2;
        pixel*          m_line_buffer          = nullptr;
        int             m_clks_begin_align_scx = 0;
        int             m_clks_end_window_init = 0;
        int             m_x_pos                = 0;
        int             m_x_pos_win_start      = 0;
        int             m_alignment_x          = 0;
        gb_current_line m_clks_bgp_change      = gb_no_line;
        int             m_next_sprite_x        = -1;
    };

} // namespace age



#endif // AGE_GB_LCD_FIFO_RENDERER_HPP
