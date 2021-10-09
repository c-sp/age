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

#ifndef AGE_GB_LCD_RENDER_HPP
#define AGE_GB_LCD_RENDER_HPP

//!
//! \file
//!

#include <age_types.hpp>
#include <gfx/age_pixel.hpp>
#include <gfx/age_screen_buffer.hpp>

#include "../../common/age_gb_device.hpp"
#include "../palettes/age_gb_lcd_palettes.hpp"
#include "age_gb_lcd_sprites.hpp"



namespace age
{
    constexpr int16_t gb_clock_cycles_per_lcd_line  = 456;
    constexpr int16_t gb_lcd_line_count             = 154;
    constexpr int     gb_clock_cycles_per_lcd_frame = gb_lcd_line_count * gb_clock_cycles_per_lcd_line;

    constexpr int16_t gb_screen_width  = 160;
    constexpr int16_t gb_screen_height = 144;

    constexpr uint8_t gb_lcdc_enable      = 0x80;
    constexpr uint8_t gb_lcdc_win_map     = 0x40;
    constexpr uint8_t gb_lcdc_win_enable  = 0x20;
    constexpr uint8_t gb_lcdc_bg_win_data = 0x10;
    constexpr uint8_t gb_lcdc_bg_map      = 0x08;
    constexpr uint8_t gb_lcdc_obj_size    = 0x04;
    constexpr uint8_t gb_lcdc_obj_enable  = 0x02;
    constexpr uint8_t gb_lcdc_bg_enable   = 0x01;



    class gb_lcd_render_common
    {
        AGE_DISABLE_COPY(gb_lcd_render_common);
        AGE_DISABLE_MOVE(gb_lcd_render_common);

    public:
        gb_lcd_render_common(const gb_device& device, gb_lcd_sprites& sprites);
        ~gb_lcd_render_common() = default;

        [[nodiscard]] uint8_t get_lcdc() const
        {
            return m_lcdc;
        }

        void set_lcdc(uint8_t lcdc);
        void reset_window();

        [[nodiscard]] bool window_visible(int line) const;
        [[nodiscard]] int  get_next_window_line() const;

    private:
        const gb_device& m_device;
        gb_lcd_sprites&  m_sprites;

        uint8_t     m_lcdc  = 0;
        mutable int m_wline = -1;

    public:
        const uint8_array<256> m_xflip_cache;

        int     m_bg_tile_map_offset  = 0;
        int     m_win_tile_map_offset = 0;
        int     m_tile_data_offset    = 0;
        uint8_t m_tile_xor            = 0;
        uint8_t m_priority_mask       = 0xFF;

        uint8_t m_scy = 0;
        uint8_t m_scx = 0;
        uint8_t m_wy  = 0;
        uint8_t m_wx  = 0;
    };



    class gb_lcd_line_renderer
    {
        AGE_DISABLE_COPY(gb_lcd_line_renderer);
        AGE_DISABLE_MOVE(gb_lcd_line_renderer);

    public:
        gb_lcd_line_renderer(const gb_lcd_render_common& common,
                             const gb_device&            device,
                             const gb_lcd_palettes&      palettes,
                             const gb_lcd_sprites&       sprites,
                             const uint8_t*              video_ram,
                             screen_buffer&              screen_buffer);

        ~gb_lcd_line_renderer() = default;

        void render_line(int line);

    private:
        pixel* render_bg_tile(pixel* dst, int tile_line, int tile_vram_ofs);
        void   render_sprite_tile(pixel* dst, int tile_line, const gb_sprite& sprite);

        const gb_lcd_render_common& m_common;
        const gb_device&            m_device;
        const gb_lcd_palettes&      m_palettes;
        const gb_lcd_sprites&       m_sprites;
        const uint8_t*              m_video_ram;
        screen_buffer&              m_screen_buffer;

        // 160px + 3 tiles (8px + scx + last window/sprite tile)
        pixel_vector m_line{gb_screen_width + 24, pixel(0, 0, 0)};
    };



    struct gb_current_line
    {
        int m_line;
        int m_line_clks;
    };

    constexpr gb_current_line gb_no_line = {.m_line = -1, .m_line_clks = -1};

    class gb_lcd_dot_renderer
    {
        AGE_DISABLE_COPY(gb_lcd_dot_renderer);
        AGE_DISABLE_MOVE(gb_lcd_dot_renderer);

    public:
        gb_lcd_dot_renderer(const gb_lcd_render_common& common,
                            const gb_device&            device,
                            const gb_lcd_palettes&      palettes,
                            const gb_lcd_sprites&       sprites,
                            const uint8_t*              video_ram,
                            screen_buffer&              screen_buffer);

        ~gb_lcd_dot_renderer() = default;

        [[nodiscard]] bool in_progress() const;

        void reset();
        void begin_new_line(gb_current_line line);
        bool continue_line(gb_current_line until);

    private:
        const gb_lcd_render_common& m_common;
        const gb_device&            m_device;
        const gb_lcd_palettes&      m_palettes;
        const gb_lcd_sprites&       m_sprites;
        const uint8_t*              m_video_ram;
        screen_buffer&              m_screen_buffer;

        gb_current_line m_rendering_line = gb_no_line;
    };



    class gb_lcd_render : private gb_lcd_render_common
    {
        AGE_DISABLE_COPY(gb_lcd_render);
        AGE_DISABLE_MOVE(gb_lcd_render);

    public:
        gb_lcd_render(const gb_device&       device,
                      const gb_lcd_palettes& palettes,
                      gb_lcd_sprites&        sprites,
                      const uint8_t*         video_ram,
                      screen_buffer&         screen_buffer);

        ~gb_lcd_render() = default;

        void new_frame();
        void render(gb_current_line until);

        using gb_lcd_render_common::get_lcdc;
        using gb_lcd_render_common::set_lcdc;

        using gb_lcd_render_common::m_scx;
        using gb_lcd_render_common::m_scy;
        using gb_lcd_render_common::m_wx;
        using gb_lcd_render_common::m_wy;

    private:
        gb_lcd_dot_renderer  m_dot_renderer;
        gb_lcd_line_renderer m_line_renderer;
        screen_buffer&       m_screen_buffer;

        int m_rendered_lines = 0;
    };

} // namespace age



#endif // AGE_GB_LCD_RENDER_HPP
