//
// © 2020 Christoph Sprenger <https://github.com/c-sp>
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

#ifndef AGE_GB_LCD_RENDERER_HPP
#define AGE_GB_LCD_RENDERER_HPP

//!
//! \file
//!

#include "../../common/age_gb_device.hpp"
#include "../palettes/age_gb_lcd_palettes.hpp"
#include "age_gb_lcd_fifo_renderer.hpp"
#include "age_gb_lcd_line_renderer.hpp"
#include "age_gb_lcd_renderer_common.hpp"
#include "age_gb_lcd_window_check.hpp"

#include <age_types.hpp>
#include <gfx/age_screen_buffer.hpp>

#include <span>



namespace age
{

    class gb_lcd_renderer : private gb_lcd_renderer_common
    {
        AGE_DISABLE_COPY(gb_lcd_renderer);
        AGE_DISABLE_MOVE(gb_lcd_renderer);

    public:
        gb_lcd_renderer(const gb_device&         device,
                        const gb_lcd_palettes&   palettes,
                        gb_lcd_sprites&          sprites,
                        std::span<uint8_t const> video_ram,
                        screen_buffer&           screen_buffer);

        ~gb_lcd_renderer() = default;

        [[nodiscard]] bool stat_mode0() const;

        void set_clks_tile_data_change(gb_current_line at_line);
        void set_clks_bgp_change(gb_current_line at_line);
        void check_for_wy_match(gb_current_line at_line, uint8_t wy);
        void new_frame(bool frame_is_blank);
        void render(gb_current_line until, bool is_first_frame);

        using gb_lcd_renderer_common::get_lcdc;
        using gb_lcd_renderer_common::set_lcdc;

        using gb_lcd_renderer_common::m_scx;
        using gb_lcd_renderer_common::m_scy;
        using gb_lcd_renderer_common::m_wx;
        using gb_lcd_renderer_common::m_wy;

    private:
        gb_window_check        m_window;
        gb_lcd_fifo_renderer   m_fifo_renderer;
        gb_lcd_line_renderer   m_line_renderer;
        screen_buffer&         m_screen_buffer;
        const gb_lcd_palettes& m_palettes;

        int m_rendered_lines = 0;
    };

} // namespace age



#endif // AGE_GB_LCD_RENDERER_HPP
