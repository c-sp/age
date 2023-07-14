//
// © 2022 Christoph Sprenger <https://github.com/c-sp>
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

#ifndef AGE_GB_LCD_LINE_RENDERER_HPP
#define AGE_GB_LCD_LINE_RENDERER_HPP

//!
//! \file
//!

#include "../../common/age_gb_device.hpp"
#include "../palettes/age_gb_lcd_palettes.hpp"
#include "age_gb_lcd_renderer_common.hpp"
#include "age_gb_lcd_sprites.hpp"
#include "age_gb_lcd_window_check.hpp"

#include <age_types.hpp>
#include <gfx/age_pixel.hpp>
#include <gfx/age_screen_buffer.hpp>

#include <span>



namespace age
{

    class gb_lcd_line_renderer
    {
        AGE_DISABLE_COPY(gb_lcd_line_renderer);
        AGE_DISABLE_MOVE(gb_lcd_line_renderer);

    public:
        gb_lcd_line_renderer(const gb_device&              device,
                             const gb_lcd_renderer_common& common,
                             const gb_lcd_palettes&        palettes,
                             const gb_lcd_sprites&         sprites,
                             std::span<uint8_t const>      video_ram,
                             gb_window_check&              window,
                             screen_buffer&                screen_buffer);

        ~gb_lcd_line_renderer() = default;

        void render_line(int line);

    private:
        void render_bg_tile(std::span<pixel, 8> dst, int tile_line, int tile_vram_ofs);
        void render_sprite_tile(std::span<pixel, 8> dst, int tile_line, const gb_sprite& sprite);

        const gb_device&              m_device;
        const gb_lcd_renderer_common& m_common;
        const gb_lcd_palettes&        m_palettes;
        const gb_lcd_sprites&         m_sprites;
        std::span<uint8_t const>      m_video_ram;
        gb_window_check&              m_window;
        screen_buffer&                m_screen_buffer;

        // 160px + 3 tiles (8px + scx + last window/sprite tile)
        pixel_vector m_line{gb_screen_width + 24, pixel(0, 0, 0)};
    };

} // namespace age



#endif // AGE_GB_LCD_LINE_RENDERER_HPP
