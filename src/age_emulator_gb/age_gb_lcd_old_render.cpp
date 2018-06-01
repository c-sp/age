//
// Copyright 2018 Christoph Sprenger
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

#include "age_gb_lcd.hpp"
/*




//---------------------------------------------------------
//
//   Render single bg tile.
//
//---------------------------------------------------------
//
// bg, window & sprite priorities:
// sprite pixel displayed, if prio < 0x40
//
// - no flags:    priority in [0x00;0x03]
// - sprite flag: priority in [0x40;0x43]
// - bg flag:     priority >= 0x80
//

void age::gb_lcd::render_bg_tile()
{
    AGE_ASSERT((m_stat & 0x03) == 0x03);
    AGE_ASSERT(m_current_tile < m_last_tile);

    // tile data
    uint tileIndexAdd = ((m_lcdc & gb_lcdc_bg_win_data) > 0) ? 0 : 128;
    uint tileData = ((m_lcdc & gb_lcdc_bg_win_data) > 0) ? 0x0000 : 0x0800; // video ram, not gb memory
    uint tileMap = ((m_lcdc & gb_lcdc_bg_map) > 0) ? 0x1C00 : 0x1800;

    // tile line offset
    tileMap += ((m_scy + get_ly()) & 0xF8ul) << 2; //  y / 8 * 32

    // read tile index & attributes
    uint tileMapOffset = tileMap + (((m_tile_scx >> 3) + m_current_tile) & 0x1F); // tilemap offset for this tile
    uint tileDataIndex = (tileIndexAdd + m_video_ram[tileMapOffset]) & 0xFF; // tile index
    uint8 tileAttribs = m_video_ram[0x2000 + tileMapOffset]; // tile attributes
    tileDataIndex <<= 4;

    AGE_ASSERT(m_cgb || (tileAttribs == 0));
    AGE_ASSERT(tileDataIndex < 256 * 16);

    // offset within a tile
    uint tileLineOffset = ((m_scy + get_ly()) & 7) << 1;
    if ((tileAttribs & 0x40) > 0)
    {
        tileLineOffset ^= 0x0E; // attribute: vertical flip
    }

    // render tile line
    render_bg_tile_line(&m_scanline[8 + m_current_tile * 8], tileData + tileLineOffset + tileDataIndex, tileAttribs);
}





//---------------------------------------------------------
//
//   Render lcd scanline.
//
//---------------------------------------------------------

void age::gb_lcd::render_line()
{
    AGE_ASSERT((m_stat & 0x03) == 0x03);



    // --- render window line

    if (((m_lcdc & gb_lcdc_win_enable) > 0) && (m_wx < 167) && (m_wy_render <= get_ly()))
    {
        // tile data
        uint tile_index_add = ((m_lcdc & gb_lcdc_bg_win_data) > 0)? 0 : 128;
        uint tile_data = ((m_lcdc & gb_lcdc_bg_win_data) > 0)? 0x0000 : 0x0800; // video ram, not gb memory!
        uint tile_map = ((m_lcdc & gb_lcdc_win_map) > 0)? 0x1C00 : 0x1800; // video ram, not gb memory!
        // tile line offset
        tile_map += (m_window_line_counter & 0xF8) << 2; //  / 8 * 32;
        // offset within a tile
        uint tile_line_offset = (m_window_line_counter & 7) << 1;

        // render window line
        uint offset = 8 + (m_line_scx & 7) - 7 + m_wx;
        for (uint tile = 0, max = ((167 - m_wx) >> 3) + 1; tile < max; ++tile)
        {
            // read tile index & attributes
            uint tile_offset = tile_map + (tile & 0x1F); // tilemap offset for this tile
            uint tile_data_index = (tile_index_add + m_video_ram[tile_offset]) & 0xFF; // tile index
            uint8 tile_attribs = m_video_ram[0x2000 + tile_offset]; // tile attributes
            AGE_ASSERT(m_cgb || (tile_attribs == 0));
            AGE_ASSERT(tile_data_index < 256);
            tile_data_index <<= 4;
            // calculate tile data offset
            uint tile_data_offset = tile_line_offset;
            if ((tile_attribs & 0x40) > 0)
            {
                tile_data_offset ^= 0x0E;   // attribute: vertical flip
            }
            // render tile line
            AGE_ASSERT(offset < m_scanline.size());
            render_bg_tile_line(&m_scanline[offset], tile_data + tile_line_offset + tile_data_index, tile_attribs);
            offset += 8;
        }
        ++m_window_line_counter;
    }



    // --- render sprite line

    {
        uint line_offset = (m_line_scx & 7) + 8;
        // priority mask
        uint8 prio_mask = (m_cgb && ((m_lcdc & gb_lcdc_bg_enable) == 0))? 0x00 : 0xFF;
        // render cached sprites
        for (uint sprite_index = 0; sprite_index < m_sprite_cache.size(); ++sprite_index)
        {
            gb_sprite *sprite = &m_sprite_cache[sprite_index];
            // sprite not visible
            if ((sprite->get_x() >= 168) || (sprite->get_x() == 0))
            {
                continue;
            }
            // sprite's x coordinate
            uint offset = line_offset + sprite->get_x() - 8;
            // tile indices
            uint index1 = (sprite->get_tile_byte1() & 0xF0) + ((sprite->get_tile_byte2() & 0xF0) >> 4);
            uint index2 = ((sprite->get_tile_byte1() & 0x0F) << 4) + (sprite->get_tile_byte2() & 0x0F);
            index1 <<= 2;
            index2 <<= 2;

            // render sprite line
            for (uint index = 0; index < 2; ++index, index1 = index2)
            {
                for (uint pixel = 0; pixel < 4; ++pixel, ++offset)
                {
                    AGE_ASSERT(offset < m_scanline.size());
                    uint8 cp = m_scanline[offset].m_priority | sprite->get_priority();
                    if ((cp & prio_mask) <= 0x40)
                    {
                        uint8 color = m_tile_cache[index1 + pixel];
                        if (color > 0)
                        {
                            uint8 colorIndex = sprite->get_palette() + color;
                            AGE_ASSERT(colorIndex < gb_num_palette_colors);
                            m_scanline[offset].m_color = m_colors[colorIndex];
                        }
                    }
                } // for
            } // for
        } // for
    }



    // --- copy line into frame buffer

    {
        uint line_offset = 8 + (m_line_scx & 7);
        // offset within line cache
        AGE_ASSERT((line_offset >= 8) && (line_offset < 16));
        uint vr_offset = 160 * ((gb_screen_height - 1) - get_ly()) - line_offset;
        // fill lcd line
        for (uint max = line_offset + 160; line_offset < max; ++line_offset)
        {
            (*m_current_back_frame)[vr_offset + line_offset] = m_scanline[line_offset].m_color;
        }
    }
}





//---------------------------------------------------------
//
//   Render bg/wnd tile line.
//
//---------------------------------------------------------

void age::gb_lcd::render_bg_tile_line(scanline_pixel *pixel, uint tile_data_offset, uint8 attributes)
{
    tile_data_offset += (attributes & 0x08ul) * 0x400; // attribute: vram bank

    // read tile data
    uint8 tile_byte1 = m_video_ram[tile_data_offset];
    uint8 tile_byte2 = m_video_ram[tile_data_offset + 1];
    if ((attributes & 0x20) > 0) // attribute: horizontal flip
    {
        tile_byte1 = m_xflip_cache[tile_byte1];
        tile_byte2 = m_xflip_cache[tile_byte2];
    }
    uint index1 = (tile_byte1 & 0xF0) + ((tile_byte2 & 0xF0) >> 4);
    uint index2 = ((tile_byte1 & 0x0F) << 4) + (tile_byte2 & 0x0F);
    index1 <<= 2;
    index2 <<= 2;

    // palette & color priority
    uint8 palette = (attributes & 0x07) << 2;
    uint8 priority = attributes & 0x80;

    // cache colors & priorities
    for (uint index = 0; index < 2; ++index, index1 = index2)
    {
        for (uint pixel_index = 0; pixel_index < 4; ++pixel_index)
        {
            uint8 color_index = pixel->m_priority = m_tile_cache[index1 + pixel_index];
            pixel->m_priority += priority;
            color_index += palette;
            AGE_ASSERT(color_index < gb_num_palette_colors);
            pixel->m_color = m_colors[color_index];
            ++pixel;
        }
    }
}
*/
