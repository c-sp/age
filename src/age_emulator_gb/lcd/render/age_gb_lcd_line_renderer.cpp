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

#include "age_gb_lcd_line_renderer.hpp"

#include <age_debug.hpp>

#include <algorithm> // std::fill



age::gb_lcd_line_renderer::gb_lcd_line_renderer(const gb_device&              device,
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
      m_video_ram(video_ram),
      m_window(window),
      m_screen_buffer(screen_buffer)
{
}



void age::gb_lcd_line_renderer::render_line(int line)
{
    // We use the pixel alpha channel temporary for priority
    // information.
    // When the line is finished, during copying to the
    // screen buffer the alpha channel is restored to 0xFF.
    //
    // Priority bits:
    //  0-2     BG color index
    //  3-6     -
    //  7       BG priority flag (OAM | CGB tile attribute)
    //
    //  => render sprite pixel only for priority <= 0x80

    // first visible pixel
    int px0 = 8 + (m_common.m_scx & 0b111);

    // BG & window not visible
    if (!m_device.cgb_mode() && !(m_common.get_lcdc() & gb_lcdc_bg_enable))
    {
        pixel fill_color = m_palettes.get_palette(gb_palette_bgp)[0];
        fill_color.m_a   = 0; // sprites are prioritized
        std::fill(begin(m_line), end(m_line), fill_color);
    }

    // render BG & window
    else
    {
        m_window.check_for_wy_match(m_common.get_lcdc(), m_common.m_wy, line);
        bool winx_visible  = ((m_common.get_lcdc() & gb_lcdc_win_enable) != 0) && (m_common.m_wx < 167);
        bool render_window = m_window.is_enabled_and_wy_matched(m_common.get_lcdc()) && winx_visible;

        // render BG
        int    bg_y          = m_common.m_scy + line;
        int    tile_vram_ofs = m_common.m_bg_tile_map_offset + ((bg_y & 0b11111000) << 2);
        int    tile_line     = bg_y & 0b111;
        pixel* px            = &m_line[8];

        int tiles = render_window
                        ? 1 + std::min<int>(20, std::max<int>(0, m_common.m_wx - 7) >> 3)
                        : 21;

        for (int tx = m_common.m_scx >> 3, max = tx + tiles; tx < max; ++tx)
        {
            px = render_bg_tile(px, tile_line, tile_vram_ofs + (tx & 0b11111));
        }

        // render window
        if (render_window)
        {
            int wline     = m_window.next_window_line();
            tile_vram_ofs = m_common.m_win_tile_map_offset + ((wline & 0b11111000) << 2);
            tile_line     = wline & 0b111;
            px            = &m_line[px0 + m_common.m_wx - 7];

            for (int tx = 0, max = ((gb_screen_width + 7 - m_common.m_wx) >> 3) + 1; tx < max; ++tx)
            {
                px = render_bg_tile(px, tile_line, tile_vram_ofs + tx);
            }
        }
    }

    // render sprites
    if (m_common.get_lcdc() & gb_lcdc_obj_enable)
    {
        auto sprites = m_sprites.get_line_sprites(line, !m_device.cgb_mode());
        std::for_each(rbegin(sprites),
                      rend(sprites),
                      [this, &px0, &line](const gb_sprite& sprite) {
                          if (sprite.m_x && (sprite.m_x < gb_screen_width + 8))
                          {
                              AGE_ASSERT((px0 + sprite.m_x) < gb_screen_width + 24)
                              render_sprite_tile(
                                  &m_line[px0 + sprite.m_x - 8],
                                  line - (sprite.m_y - 16),
                                  sprite);
                          }
                      });
    }

    // copy line
    auto* dst = &m_screen_buffer.get_back_buffer()[static_cast<size_t>(line) * gb_screen_width];
    auto* src = &m_line[px0];

    auto alpha_bits = pixel{0, 0, 0, 255}.get_32bits();
    for (int i = 0; i < gb_screen_width; ++i)
    {
        // replace priority information with alpha value
        dst->set_32bits(src->get_32bits() | alpha_bits);
        ++src;
        ++dst;
    }
}



age::pixel* age::gb_lcd_line_renderer::render_bg_tile(pixel* dst,
                                                      int    tile_line,
                                                      int    tile_vram_ofs)
{
    AGE_ASSERT((tile_vram_ofs >= 0x1800) && (tile_vram_ofs < 0x2000))
    AGE_ASSERT((tile_line >= 0) && (tile_line < 8))

    int tile_nr = m_video_ram[tile_vram_ofs] ^ m_common.m_tile_xor; // bank 0

    // tile attributes (for DMG always zero)
    int attributes = m_video_ram[tile_vram_ofs + 0x2000]; // bank 1

    // y-flip
    if (attributes & gb_tile_attrib_flip_y)
    {
        tile_line = 7 - tile_line;
    }

    // read tile data
    int tile_data_ofs = tile_nr << 4; // 16 bytes per tile
    tile_data_ofs += m_common.m_tile_data_offset;
    tile_data_ofs += (attributes & gb_tile_attrib_vram_bank) << 10;
    tile_data_ofs += tile_line << 1; // 2 bytes per line

    int tile_byte1 = m_video_ram[tile_data_ofs];
    int tile_byte2 = m_video_ram[tile_data_ofs + 1];

    // x-flip
    // (we invert the x-flip for easier rendering:
    // this way bit 0 is used for the leftmost pixel)
    if (!(attributes & gb_tile_attrib_flip_x))
    {
        tile_byte1 = m_common.m_xflip_cache[tile_byte1];
        tile_byte2 = m_common.m_xflip_cache[tile_byte2];
    }

    // bg palette
    const pixel* palette = m_palettes.get_palette(attributes & gb_tile_attrib_cgb_palette);

    // bg priority
    uint8_t priority = attributes & gb_tile_attrib_priority;

    // render tile line
    // (least significant bit in byte1)
    tile_byte2 <<= 1;

    for (int i = 0; i < 8; ++i)
    {
        int   color_idx = (tile_byte1 & 0b01) + (tile_byte2 & 0b10);
        pixel color     = palette[color_idx];
        color.m_a       = color_idx + priority;
        *dst            = color;

        ++dst;
        tile_byte1 >>= 1;
        tile_byte2 >>= 1;
    }
    return dst;
}



void age::gb_lcd_line_renderer::render_sprite_tile(pixel*           dst,
                                                   int              tile_line,
                                                   const gb_sprite& sprite)
{
    AGE_ASSERT((tile_line >= 0) && (tile_line < 16))

    // y-flip
    uint8_t oam_attr = sprite.m_attributes;
    if (oam_attr & gb_tile_attrib_flip_y)
    {
        tile_line = m_sprites.get_sprite_size() - 1 - tile_line;
    }

    // read tile data
    int tile_data_ofs = sprite.m_tile_nr & m_sprites.get_tile_nr_mask();
    tile_data_ofs <<= 4; // 16 bytes per tile
    tile_data_ofs += (oam_attr & gb_tile_attrib_vram_bank) << 10;
    tile_data_ofs += tile_line << 1; // 2 bytes per line

    int tile_byte1 = m_video_ram[tile_data_ofs];
    int tile_byte2 = m_video_ram[tile_data_ofs + 1];

    // x-flip
    // (we invert the x-flip for easier rendering:
    // this way bit 0 is used for the leftmost pixel)
    if (!(oam_attr & gb_tile_attrib_flip_x))
    {
        tile_byte1 = m_common.m_xflip_cache[tile_byte1];
        tile_byte2 = m_common.m_xflip_cache[tile_byte2];
    }

    // palette
    const pixel* palette = m_palettes.get_palette(sprite.m_palette_idx);

    // bg priority
    uint8_t priority = oam_attr & gb_tile_attrib_priority;

    // render tile sprite
    // (the least significant bit in byte1)
    tile_byte2 <<= 1;

    for (int i = 0; i < 8; ++i)
    {
        pixel px = *dst;

        // sprite pixel visible?
        int color_idx = (tile_byte1 & 0b01) + (tile_byte2 & 0b10);

        int px_priority = (px.m_a | priority) & m_common.m_priority_mask;
        if ((px_priority <= 0x80) && color_idx)
        {
            pixel color = palette[color_idx];
            color.m_a   = px.m_a;
            *dst        = color;
        }

        // next pixel
        ++dst;
        tile_byte1 >>= 1;
        tile_byte2 >>= 1;
    }
}
