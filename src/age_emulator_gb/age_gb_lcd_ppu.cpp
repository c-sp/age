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

#include <algorithm>

#include <age_debug.hpp>

#include "age_gb_lcd.hpp"

#if 0
#define LOG(x) { if ((m_core.get_oscillation_cycle() < 13000) && ((uint)get_ly() < 2)) \
{ AGE_LOG("cycle " << m_core.get_oscillation_cycle() << ", x_current " << m_x_current << ", " << m_tile_offset << ", ly " << (uint)get_ly() << ": " << x); }}
#else
#define LOG(x)
#endif

#if 0
#define LOG_FETCH(x) LOG(x)
#else
#define LOG_FETCH(x)
#endif

#if 0
#define LOG_PLOT(x) LOG(x)
#else
#define LOG_PLOT(x)
#endif



// memory dumps,
// based on *.bin files used by gambatte tests and gambatte source code (initstate.cpp)

constexpr const age::uint8_array<0xA0> dmg_oam_dump =
{{
     0xBB, 0xD8, 0xC4, 0x04, 0xCD, 0xAC, 0xA1, 0xC7, 0x7D, 0x85, 0x15, 0xF0, 0xAD, 0x19, 0x11, 0x6A,
     0xBA, 0xC7, 0x76, 0xF8, 0x5C, 0xA0, 0x67, 0x0A, 0x7B, 0x75, 0x56, 0x3B, 0x65, 0x5C, 0x4D, 0xA3,
     0x00, 0x05, 0xD7, 0xC9, 0x1B, 0xCA, 0x11, 0x6D, 0x38, 0xE7, 0x13, 0x2A, 0xB1, 0x10, 0x72, 0x4D,
     0xA7, 0x47, 0x13, 0x89, 0x7C, 0x62, 0x5F, 0x90, 0x64, 0x2E, 0xD3, 0xEF, 0xAB, 0x01, 0x15, 0x85,
     0xE8, 0x2A, 0x6E, 0x4A, 0x1F, 0xBE, 0x49, 0xB1, 0xE6, 0x0F, 0x93, 0xE2, 0xB6, 0x87, 0x5D, 0x35,
     0xD8, 0xD4, 0x4A, 0x45, 0xCA, 0xB3, 0x33, 0x74, 0x18, 0xC1, 0x16, 0xFB, 0x8F, 0xA4, 0x8E, 0x70,
     0xCD, 0xB4, 0x4A, 0xDC, 0xE6, 0x34, 0x32, 0x41, 0xF9, 0x84, 0x6A, 0x99, 0xEC, 0x92, 0xF1, 0x8B,
     0x5D, 0xA5, 0x09, 0xCF, 0x3A, 0x93, 0xBC, 0xE0, 0x15, 0x19, 0xE4, 0xB6, 0x9A, 0x04, 0x3B, 0xC1,
     0x96, 0xB7, 0x56, 0x85, 0x6A, 0xAA, 0x1E, 0x2A, 0x80, 0xEE, 0xE7, 0x46, 0x76, 0x8B, 0x0D, 0xBA,
     0x24, 0x40, 0x42, 0x05, 0x0E, 0x04, 0x20, 0xA6, 0x5E, 0xC1, 0x97, 0x7E, 0x44, 0x05, 0x01, 0xA9
 }};





//---------------------------------------------------------
//
//   constructor
//
//---------------------------------------------------------

age::gb_lcd_ppu::gb_lcd_ppu(gb_core &core, const gb_memory &memory, bool dmg_green)
    : gb_lyc_interrupter(core),
      m_cgb(core.is_cgb()),
      m_dmg_green(dmg_green),
      m_core(core),
      m_memory(memory)
{
    // init caches
    create_tile_cache(m_tile_cache);
    calculate_xflip(m_xflip_cache);

    // init object attribute memory
    if (m_cgb)
    {
        std::fill(begin(m_oam), end(m_oam), 0);
    }
    else
    {
        std::copy(begin(dmg_oam_dump), end(dmg_oam_dump), begin(m_oam));
    }
}





//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

age::uint8 age::gb_lcd_ppu::read_lcdc() const { return m_lcdc; }
age::uint8 age::gb_lcd_ppu::read_scx() const { return m_scx; }
age::uint8 age::gb_lcd_ppu::read_scy() const { return m_scy; }
age::uint8 age::gb_lcd_ppu::read_wx() const { return m_wx; }
age::uint8 age::gb_lcd_ppu::read_wy() const { return m_wy; }

void age::gb_lcd_ppu::write_lcdc(uint8 value)
{
    LOG((uint)value);
    m_lcdc = value;
    m_obj_enabled = (m_lcdc & gb_lcdc_obj_enable) > 0;
    m_obj_size_16 = (m_lcdc & gb_lcdc_obj_size) > 0;
    m_win_enabled = (m_lcdc & gb_lcdc_win_enable) > 0;
    m_b_tile_name_offset = ((m_lcdc & gb_lcdc_bg_map) > 0) ? 0x1C00 : 0x1800;
    m_bw_tile_name_add = ((m_lcdc & gb_lcdc_bg_win_data) > 0) ? 0 : 0x80;
    m_w_tile_name_offset = ((m_lcdc & gb_lcdc_win_map) > 0) ? 0x1C00 : 0x1800;
    m_bw_tile_data_offset = ((m_lcdc & gb_lcdc_bg_win_data) > 0) ? 0x0000 : 0x0800; // video ram offset
}

void age::gb_lcd_ppu::write_scx(uint8 value)
{
    LOG((uint)value);
    m_scx = value;
}

void age::gb_lcd_ppu::write_scy(uint8 value)
{
    LOG((uint)value);
    m_scy = value;
}

void age::gb_lcd_ppu::write_wx(uint8 value)
{
    LOG((uint)value);
    m_wx = value;
}

void age::gb_lcd_ppu::write_wy(uint8 value)
{
    LOG((uint)value);
    m_wy = value;
}



age::uint age::gb_lcd_ppu::get_mode0_interrupt_cycle_offset() const
{
    // if the cycle offset to the upcoming mode 0 interrupt is
    // irrelevant (basically if it's more than a handful of
    // cycles away), we just return some really big number
    uint result = (get_scanline() >= gb_screen_height) ? uint_max : m_x_m0_int - m_x_current;
    return result;
}

age::uint8* age::gb_lcd_ppu::get_oam()
{
    return &m_oam[0];
}



void age::gb_lcd_ppu::create_classic_palette(uint index, uint8 colors)
{
    if (!m_cgb)
    {
        for (uint max = index + 4; index < max; ++index)
        {
            // greenish Gameboy colors (taken from some googled gameboy photo) for regular build,
            if (m_dmg_green)
            {
                switch (colors & 0x03)
                {
                    case 0x00: m_colors[index] = pixel(152, 192, 15); break;
                    case 0x01: m_colors[index] = pixel(112, 152, 15); break;
                    case 0x02: m_colors[index] = pixel(48, 96, 15); break;
                    case 0x03: m_colors[index] = pixel(15, 56, 15); break;
                }
            }
            else
            {
                switch (colors & 0x03)
                {
                    case 0x00: m_colors[index] = pixel(255, 255, 255); break;
                    case 0x01: m_colors[index] = pixel(170, 170, 170); break;
                    case 0x02: m_colors[index] = pixel(85, 85, 85); break;
                    case 0x03: m_colors[index] = pixel(0, 0, 0); break;
                }
            }
            colors >>= 2;
        }
    }
}

void age::gb_lcd_ppu::update_color(size_t index, uint8_t high_byte, uint8_t low_byte)
{
    AGE_ASSERT(m_cgb && (index < 64));

    // make sure we have at least 32 bits available,
    // don't rely on integral promotion to int (high_byte << 8)
    int32_t gb_color = high_byte;
    gb_color = (gb_color << 8) + low_byte;

    // gb-color:   red:    bits 0-4
    //             green:  bits 5-9
    //             blue:   bits 10-14
    int32_t gb_r = gb_color & 0x1F;
    int32_t gb_g = (gb_color >> 5) & 0x1F;
    int32_t gb_b = (gb_color >> 10) & 0x1F;

    // formula copied from gambatte (video.cpp)
    int32_t r = (gb_r * 13 + gb_g * 2 + gb_b) >> 1;
    int32_t g = (gb_g * 3 + gb_b) << 1;
    int32_t b = (gb_r * 3 + gb_g * 2 + gb_b * 11) >> 1;

    m_colors[index] = pixel(r, g, b);
}

void age::gb_lcd_ppu::white_screen(pixel_vector &screen)
{
    pixel p = (m_cgb || !m_dmg_green) ? pixel(255, 255, 255) : pixel(152, 192, 15);
    std::fill(begin(screen), end(screen), p);
}

void age::gb_lcd_ppu::write_late_wy()
{
    LOG("");
    m_late_wy = m_wy;
}

void age::gb_lcd_ppu::search_sprites()
{
    m_sprites.clear();
    m_sprite_at_167 = false;

    // sprites visible?
    if (m_obj_enabled)
    {
        // sprite size
        uint8 sprite_size = m_obj_size_16 ? 16 : 8;

        // search sprites intersected by current scanline
        for (uint8 *sprite = &m_oam[39 * 4], *min = &m_oam[0]; sprite >= min; sprite -= 4) // highest sprite priority last
        {
            uint sprite_line = get_scanline() + 16 - *sprite;
            if (sprite_line < sprite_size)
            {
                m_sprite_at_167 |= *(sprite + 1) == 167;
                m_sprites.push_back(gb_sprite(sprite, sprite_line));
            }
        }

        // order by x coordinate
        std::sort(begin(m_sprites), end(m_sprites));
        LOG("found " << m_sprites.size() << " sprite(s)");

        // at most 10 sprites per scanline
        if (m_sprites.size() > 10)
        {
            m_sprites.erase(begin(m_sprites) + 10, end(m_sprites));
        }
    }
}





//---------------------------------------------------------
//
//   public rendering methods
//
//---------------------------------------------------------

void age::gb_lcd_ppu::scanline_init(pixel *first_scanline_pixel)
{
    m_x_scx = 0;
    m_x_current = 0;
    m_next_pixel = first_scanline_pixel;
    m_pause_plotting = false;

    m_window_started = false;
    m_plotting_window = false;
    m_window_map_offset = 0;

    m_sprites.push_back(gb_sprite()); // add sprite "terminator"
    m_current_sprite = m_sprites.size() - 1; // prevent sprite data fetching until SCX match
    m_sprite_tile_offset = 8;

    m_tile_offset = 0;
    m_tile_map_offset = 0;

    m_next_fetch_step = &tile_step_0;
    m_next_plot_step = &plot_await_scx_match;

    LOG("scanline initialized");
}

void age::gb_lcd_ppu::scanline_step()
{
    m_next_plot_step(*this);
}

bool age::gb_lcd_ppu::scanline_nearly_finished() const
{
    return m_x_current >= 166;
}

bool age::gb_lcd_ppu::scanline_finished() const
{
    return m_x_current == 168;
}

bool age::gb_lcd_ppu::scanline_flag_mode0() const
{
    bool result = (m_x_current == m_x_m0) && (m_sprites[m_current_sprite].get_x() != m_x_current);
    LOG(result);
    return result;
}

bool age::gb_lcd_ppu::scanline_mode0_interrupt() const
{
    bool result = (m_x_current == m_x_m0_int);// && (m_sprites[m_current_sprite].get_x() != m_x_current);
    LOG(result);
    return result;
}





//---------------------------------------------------------
//
//   utility methods
//
//---------------------------------------------------------

void age::gb_lcd_ppu::calculate_xflip(uint8_array<256> &xflip)
{
    for (uint byte = 0; byte < 256; ++byte)
    {
        uint8 flip_byte = 0;
        for (uint bit = 0x01, flip_bit = 0x80;
             bit < 0x100;
             bit += bit, flip_bit >>= 1)
        {
            if ((byte & bit) > 0)
            {
                flip_byte |= flip_bit;
            }
        }
        xflip[byte] = flip_byte;
    }
}

void age::gb_lcd_ppu::create_tile_cache(uint8_array<1024> &cache)
{
    uint i = 0;
    for (uint tile = 0; tile < 256; ++tile)
    {
        for (uint index = tile, pixel = 0; pixel < 4; ++pixel)
        {
            uint color_index = ((index & 0x80) >> 7) + ((index & 0x08) >> 2);
            AGE_ASSERT(color_index < 4);
            cache[i] = static_cast<uint8>(color_index);
            index += index;
            ++i;
        }
    }
}





//---------------------------------------------------------
//
//   plotting
//
//---------------------------------------------------------

void age::gb_lcd_ppu::plot_await_scx_match(gb_lcd_ppu &ppu)
{
    // SCX matches -> wait for tile data
    if ((ppu.m_x_scx & 7) == (ppu.m_scx & 7))
    {
        LOG("SCX match, switching to plotting method");
        ppu.m_current_sprite = 0; // allow sprite data fetching
        ppu.update_pause_plotting();
        AGE_ASSERT(!ppu.m_plotting_window);

        //
        // verified by gambatte tests
        //
        // On a DMG the tile name offset calculation is off
        // by 1 once SCX matches.
        //
        //      scx_during_m3/scx1_scx0_during_m3_1
        //      scx_during_m3/scx2_scx1_during_m3_1
        //      scx_during_m3/scx_0761c0/scx_during_m3_1
        //      scx_during_m3/scx_0761c0/scx_during_m3_2
        //      scx_during_m3/scx_0761c0/scx_during_m3_3
        //      scx_during_m3/scx_0761c0/scx_during_m3_4
        //
        ppu.m_tile_map_offset = ppu.m_cgb ? 0 : 1;

        //
        // On a DMG the first pixel is plotted 12 cycles after
        // mode 3 was started (92 cycles after the mode 2
        // interrupt for LY 0 was raised). That's one cycle
        // more than on a CGB.
        //
        //      dmgpalette_during_m3/dmgpalette_during_m3_3
        //      dmgpalette_during_m3/dmgpalette_during_m3_4
        //      dmgpalette_during_m3/dmgpalette_during_m3_5
        //
        ppu.m_next_plot_step = &plot_await_data;
        if (ppu.m_cgb)
        {
            plot_await_data(ppu);
        }
    }

    // continue waiting for SCX match
    else
    {
        LOG_PLOT("no SCX match, x_scx " << (ppu.m_x_scx & 7) << ", scx " << (ppu.m_scx & 7));
        ppu.m_next_fetch_step(ppu);
        AGE_ASSERT(ppu.m_tile_offset < 8);

        ++ppu.m_x_scx;
        ++ppu.m_tile_offset;
    }
}



void age::gb_lcd_ppu::plot_await_data(gb_lcd_ppu &ppu)
{
    AGE_ASSERT(ppu.m_x_current < 8);
    ppu.m_next_fetch_step(ppu);
    AGE_ASSERT(ppu.m_tile_offset < 8);

    // continue only, if we're not waiting for any sprite data
    if (!ppu.m_pause_plotting)
    {
        if (ppu.m_sprite_tile_offset < 8)
        {
            ++ppu.m_sprite_tile_offset;
        }

        ++ppu.m_tile_offset;
        ++ppu.m_x_current;

        ppu.update_pause_plotting();

        // switch to plotting methods, if tile data is available on the next cycle
        if (ppu.m_x_current == 8)
        {
            AGE_ASSERT(ppu.m_tile_map_offset <= 1);
            LOG("starting pixel plotting (next cycle)");

            //
            // verified by gambatte tests
            //
            // When running at single speed, mode 3 is flagged until
            // X-Y cycles after the mode 2 interrupt was raised:
            //
            // SCX | m2 int cycles | mode 3 cycles
            // ----+---------------+--------------
            //   0 | 253 - 256     | 169 - 172
            //   2 | 253 - 256     | 169 - 172
            //   3 | 253 - 256     | 169 - 172
            //   5 | 257 - 260     | 173 - 176
            //
            //      m2int_m3stat/m2int_m3stat_1_dmg08_cgb_out3
            //      m2int_m3stat/m2int_m3stat_2_dmg08_cgb_out0
            //      m2int_m3stat/scx/m2int_scx2_m3stat_1_dmg08_cgb_out3
            //      m2int_m3stat/scx/m2int_scx2_m3stat_2_dmg08_cgb_out0
            //      m2int_m3stat/scx/m2int_scx3_m3stat_1_dmg08_cgb_out3
            //      m2int_m3stat/scx/m2int_scx3_m3stat_2_dmg08_cgb_out0
            //      m2int_m3stat/scx/m2int_scx5_m3stat_1_dmg08_cgb_out3
            //      m2int_m3stat/scx/m2int_scx5_m3stat_2_dmg08_cgb_out0
            //
            // When running at double speed, mode 3 is flagged until
            // X-Y cycles after the mode 2 interrupt was raised:
            //
            // SCX | m2 int cycles | mode 3 cycles
            // ----+---------------+--------------
            //   0 | 253 - 254     | 171 - 172
            //   1 | 255 - 256     | 173 - 174
            //   2 | 255 - 256     | 173 - 174
            //   3 | 257 - 258     | 175 - 176
            //   4 | 257 - 258     | 175 - 176
            //   5 | 259 - 260     | 177 - 178
            //   6 | 259 - 260     | 177 - 178
            //   7 | 261 - 262     | 179 - 180
            //   8 | 253 - 254     | 171 - 172
            //
            //      m2int_m3stat/m2int_m3stat_ds_1_out3
            //      m2int_m3stat/m2int_m3stat_ds_2_out0
            //      m2int_m3stat/scx/m2int_scx1_m3stat_ds_1_out3
            //      m2int_m3stat/scx/m2int_scx1_m3stat_ds_2_out0
            //      m2int_m3stat/scx/m2int_scx2_m3stat_ds_1_out3
            //      m2int_m3stat/scx/m2int_scx2_m3stat_ds_2_out0
            //      m2int_m3stat/scx/m2int_scx3_m3stat_ds_1_out3
            //      m2int_m3stat/scx/m2int_scx3_m3stat_ds_2_out0
            //      m2int_m3stat/scx/m2int_scx4_m3stat_ds_1_out3
            //      m2int_m3stat/scx/m2int_scx4_m3stat_ds_2_out0
            //      m2int_m3stat/scx/m2int_scx5_m3stat_ds_1_out3
            //      m2int_m3stat/scx/m2int_scx5_m3stat_ds_2_out0
            //      m2int_m3stat/scx/m2int_scx6_m3stat_ds_1_out3
            //      m2int_m3stat/scx/m2int_scx6_m3stat_ds_2_out0
            //      m2int_m3stat/scx/m2int_scx7_m3stat_ds_1_out3
            //      m2int_m3stat/scx/m2int_scx7_m3stat_ds_2_out0
            //      m2int_m3stat/scx/m2int_scx8_m3stat_ds_1_out3
            //      m2int_m3stat/scx/m2int_scx8_m3stat_ds_2_out0
            //
            // VRAM is accessible during the last 2 cycles of mode 3
            //
            // scx 0-3: VRAM accessible 256 cycles after mode 2 interrupt
            //
            //      vram_m3/postread_1_dmg08_cgb_out3
            //      vram_m3/postread_2_dmg08_cgb_out0
            //      vram_m3/postread_scx2_1_dmg08_cgb_out3
            //      vram_m3/postread_scx2_2_dmg08_cgb_out0
            //      vram_m3/postread_scx3_1_dmg08_cgb_out3
            //      vram_m3/postread_scx3_2_dmg08_cgb_out0
            //
            // scx 5: VRAM accessible 260 cycles after mode 2 interrupt
            //
            //      vram_m3/postread_scx5_1_dmg08_cgb_out3
            //      vram_m3/postread_scx5_2_dmg08_cgb_out0
            //
            ppu.m_x_m0 = ppu.m_sprite_at_167 ? (ppu.m_core.is_double_speed() ? 167 : 167 + 1 - ppu.m_tile_map_offset)
                                             : (ppu.m_core.is_double_speed() ? 167 : 166 + 1 - ppu.m_tile_map_offset);
            //
            // The mode 0 interrupt is triggered 1 cycle after
            // mode 0 has been flagged (not when running at
            // double speed though).
            //
            //      m0int_m0stat/m0int_m0stat_scx2_1_dmg08_cgb_out0
            //      m0int_m0stat/m0int_m0stat_scx2_2_dmg08_cgb_out2
            //      m0int_m0stat/m0int_m0stat_scx3_1_dmg08_cgb_out0
            //      m0int_m0stat/m0int_m0stat_scx3_2_dmg08_cgb_out2
            //      m0int_m0stat/m0int_m0stat_ds_1_out0
            //      m0int_m0stat/m0int_m0stat_ds_2_out2
            //      m0int_m0stat/m0int_m0stat_scx5_ds_1_out0
            //      m0int_m0stat/m0int_m0stat_scx5_ds_2_out2
            //
            ppu.m_x_m0_int = ppu.m_sprite_at_167 ? 167
                                                 : (ppu.m_core.is_double_speed() ? 167 : ppu.m_x_m0 + 1);
            //
            // On a DMG the first pixel is plotted 12 cycles after
            // mode 3 was started (92 cycles after the mode 2
            // interrupt for LY 0 was raised).
            //
            //      dmgpalette_during_m3/dmgpalette_during_m3_3
            //      dmgpalette_during_m3/dmgpalette_during_m3_4
            //      dmgpalette_during_m3/dmgpalette_during_m3_5
            //
            ppu.m_next_plot_step = &plot_pixel;
        }
    }
}



void age::gb_lcd_ppu::plot_pixel(gb_lcd_ppu &ppu)
{
    AGE_ASSERT(ppu.m_x_current >= 8);
    AGE_ASSERT(ppu.m_x_current < 168);

    ppu.m_next_fetch_step(ppu);
    AGE_ASSERT((ppu.m_tile_offset < 8) || ppu.m_pause_plotting);

    // plot pixel only, if we're not waiting for any sprite data
    if (!ppu.m_pause_plotting)
    {
        LOG_PLOT("plotting");
        uint8 color_index = ppu.m_tile[ppu.m_tile_offset];

        // plot sprite pixel?
        if (ppu.m_sprite_tile_offset < 8)
        {
            // background tile has priority -> don't plot sprite pixel
            if (ppu.m_tile_priority == 0)
            {
                color_index = get_sprite_pixel(ppu, color_index);
            }
            ++ppu.m_sprite_tile_offset;
        }

        // plot pixel
        AGE_ASSERT(color_index < ppu.m_colors.size());
        *ppu.m_next_pixel = ppu.m_colors[color_index];

        // next pixel
        ++ppu.m_x_current;
        ++ppu.m_next_pixel;
        ++ppu.m_tile_offset;

        ppu.update_pause_plotting();
    }
}

age::uint8 age::gb_lcd_ppu::get_sprite_pixel(const gb_lcd_ppu &ppu, uint8 bg_color_index)
{
    AGE_ASSERT((ppu.m_x_current - ppu.m_sprites[ppu.m_sprite_to_plot].get_x()) == ppu.m_sprite_tile_offset);
    uint8 result = bg_color_index;

    for (auto it = ppu.m_sprites.rend() - ppu.m_sprite_to_plot - 1; it != ppu.m_sprites.rend(); ++it)
    {
        const gb_sprite &current_sprite = *it;
        AGE_ASSERT(current_sprite.get_x() <= ppu.m_x_current);

        // if this sprite is out of range, stop
        uint tile_offset = ppu.m_x_current - current_sprite.get_x();
        if (tile_offset >= 8)
        {
            break;
        }

        // check, if this sprite's pixel is visible
        if ((current_sprite.get_priority() + (bg_color_index & 0x03)) <= 0x40)
        {
            uint8 sprite_color = current_sprite.get_color_index(tile_offset);
            if ((sprite_color & 0x03) != 0)
            {
                result = sprite_color;
                break;
            }
        }
    }

    return result;
}



void age::gb_lcd_ppu::update_pause_plotting()
{
    m_pause_plotting = m_sprites[m_current_sprite].get_x() == m_x_current;

    if (((m_wx + 1u) == m_x_current) && (get_scanline() >= m_late_wy) && m_win_enabled && !m_window_started)
    {
        LOG("initializing window data fetching for next cycle, wx " << (uint)m_wx);
        m_pause_plotting = true;
        m_next_fetch_step = &window_step_0;

        m_window_started = true;
        m_plotting_window = true;
        m_window_map_offset = -m_x_current;
    }
}





//---------------------------------------------------------
//
//   data fetching methods
//
//---------------------------------------------------------

void age::gb_lcd_ppu::fetch_tile_name()
{
    //
    // verified by gambatte tests
    //
    // The offset calculation to get the current tile's name
    // from the tile map adds the current SCX to the current
    // scanline pixel index before dividing by 8.
    //
    //      scx_during_m3/scx1_scx0_during_m3_1
    //      scx_during_m3/scx2_scx0_during_m3_1
    //      scx_during_m3/scx2_scx1_during_m3_1
    //      scx_during_m3/scx_0063c0/scx_during_m3_1
    //      scx_during_m3/scx_0063c0/scx_during_m3_2
    //
    uint tile_map_offset;
    if (m_plotting_window)
    {
        tile_map_offset = m_w_tile_name_offset;
        tile_map_offset += ((get_scanline() - m_wy) & 0xF8) << 2;
        tile_map_offset += ((m_x_current + m_window_map_offset) >> 3) & 0x1F;
    }
    else
    {
        tile_map_offset = m_b_tile_name_offset;
        tile_map_offset += ((m_scy + get_scanline()) & 0xF8) << 2;
        tile_map_offset += ((m_scx + m_x_current + m_tile_map_offset) >> 3) & 0x1F;
    }

    // get the tile's name and attributes
    m_new_tile_name = m_memory.get_video_ram()[tile_map_offset];
    m_new_tile_attributes = m_memory.get_video_ram()[0x2000 + tile_map_offset]; // for DMG attributes are always zero
}



void age::gb_lcd_ppu::fetch_tile_byte_1()
{
    uint tile_data_offset = get_tile_data_offset();
    m_new_tile_byte1 = m_memory.get_video_ram()[tile_data_offset];
}

void age::gb_lcd_ppu::fetch_tile_byte_2()
{
    uint tile_data_offset = get_tile_data_offset() + 1;
    m_new_tile_byte2 = m_memory.get_video_ram()[tile_data_offset];

    // flip horizontally, if requested
    if ((m_new_tile_attributes & gb_tile_attribute_x_flip) > 0) // attribute: horizontal flip
    {
        m_new_tile_byte1 = m_xflip_cache[m_new_tile_byte1];
        m_new_tile_byte2 = m_xflip_cache[m_new_tile_byte2];
    }

    // calculate tile cache index
    uint index1 = (m_new_tile_byte1 & 0xF0) + (m_new_tile_byte2 >> 4);
    uint index2 = ((m_new_tile_byte1 & 0x0F) << 4) + (m_new_tile_byte2 & 0x0F);
    index1 <<= 2;
    index2 <<= 2;

    // palette & color priority
    uint8 palette_offset = (m_new_tile_attributes & 0x07) << 2;

    // cache color indexes
    m_new_tile[0] = m_tile_cache[index1 + 0] + palette_offset;
    m_new_tile[1] = m_tile_cache[index1 + 1] + palette_offset;
    m_new_tile[2] = m_tile_cache[index1 + 2] + palette_offset;
    m_new_tile[3] = m_tile_cache[index1 + 3] + palette_offset;
    m_new_tile[4] = m_tile_cache[index2 + 0] + palette_offset;
    m_new_tile[5] = m_tile_cache[index2 + 1] + palette_offset;
    m_new_tile[6] = m_tile_cache[index2 + 2] + palette_offset;
    m_new_tile[7] = m_tile_cache[index2 + 3] + palette_offset;

    AGE_ASSERT(m_new_tile[0] < gb_num_palette_colors);
    AGE_ASSERT(m_new_tile[1] < gb_num_palette_colors);
    AGE_ASSERT(m_new_tile[2] < gb_num_palette_colors);
    AGE_ASSERT(m_new_tile[3] < gb_num_palette_colors);
    AGE_ASSERT(m_new_tile[4] < gb_num_palette_colors);
    AGE_ASSERT(m_new_tile[5] < gb_num_palette_colors);
    AGE_ASSERT(m_new_tile[6] < gb_num_palette_colors);
    AGE_ASSERT(m_new_tile[7] < gb_num_palette_colors);
}



age::uint age::gb_lcd_ppu::get_tile_data_offset() const
{
    // calculate the y-offset within the tile
    uint y = m_plotting_window ? (get_scanline() - m_wy) : (m_scy + get_scanline());
    uint tile_data_offset = (y & 7) << 1;

    // flip vertically, if requested
    if ((m_new_tile_attributes & gb_tile_attribute_y_flip) > 0)
    {
        tile_data_offset ^= 0x0E;
    }

    // add character bank offset (tile attribute & lcdc)
    tile_data_offset += (m_new_tile_attributes << 10) & 0x2000;
    tile_data_offset += m_bw_tile_data_offset;

    // add offset by tile name
    tile_data_offset += ((m_bw_tile_name_add + m_new_tile_name) & 0xFF) << 4;

    // done
    return tile_data_offset;
}





//---------------------------------------------------------
//
//   background data fetching
//
//---------------------------------------------------------

void age::gb_lcd_ppu::tile_step_0(gb_lcd_ppu &ppu)
{
    LOG_FETCH("fetching bg tile name and attributes");

    // make room for new tile data
    ppu.m_tile = ppu.m_new_tile;
    ppu.m_tile_priority = ppu.m_new_tile_attributes & gb_tile_attribute_priority;
    ppu.m_tile_offset = 0;

    // fetch new tile name
    ppu.fetch_tile_name();

    ppu.m_next_fetch_step = &tile_step_1;
}

void age::gb_lcd_ppu::tile_step_1(gb_lcd_ppu &ppu)
{
    LOG_FETCH("");
    ppu.m_next_fetch_step = &tile_step_2;
}

void age::gb_lcd_ppu::tile_step_2(gb_lcd_ppu &ppu)
{
    LOG_FETCH("fetching bg tile byte 1");
    ppu.fetch_tile_byte_1();
    ppu.m_next_fetch_step = &tile_step_3;
}

void age::gb_lcd_ppu::tile_step_3(gb_lcd_ppu &ppu)
{
    LOG_FETCH("");
    ppu.m_next_fetch_step = &tile_step_4;
}

void age::gb_lcd_ppu::tile_step_4(gb_lcd_ppu &ppu)
{
    LOG_FETCH("fetching bg tile byte 2");
    ppu.fetch_tile_byte_2();
    ppu.m_next_fetch_step = &tile_step_5;
}

void age::gb_lcd_ppu::tile_step_5(gb_lcd_ppu &ppu)
{
    LOG_FETCH("");

    // sprite data has to be fetched
    if (ppu.m_sprites[ppu.m_current_sprite].get_x() == ppu.m_x_current)
    {
        AGE_ASSERT(ppu.m_pause_plotting);

        // trigger sprite data fetching methods, if sprites are enabled
        if (ppu.m_obj_enabled)
        {
            sprite_step_0(ppu);
            return;
        }

        // fast forward to the next relevant sprite, if sprites are disabled
        while (ppu.m_sprites[ppu.m_current_sprite].get_x() == ppu.m_x_current)
        {
            ++ppu.m_current_sprite;
        }
        AGE_ASSERT(!ppu.m_window_started || ((ppu.m_wx + 1u) != ppu.m_x_current));
        ppu.m_pause_plotting = false;
    }

    // continue rendering until we finish this tile, then start over
    ppu.m_next_fetch_step = (ppu.m_tile_offset < 7) ? &tile_step_5 : &tile_step_0;
}





//---------------------------------------------------------
//
//   sprite data fetching
//
//---------------------------------------------------------

void age::gb_lcd_ppu::sprite_step_0(gb_lcd_ppu &ppu)
{
    //
    // verified by gambatte tests
    //
    // With SCX 0 each fully rendered sprite increases
    // mode 3 duration by 11 cycles (derived from the
    // tables below). Note that in this tests all sprites
    // are aligned with the background tiles.
    //
    // When running at single speed, mode 3 is flagged until
    // X-Y cycles after the LYC interrupt was raised.
    //
    // #sprites | LYC int cycles | mode 3 cycles
    // ---------+----------------+--------------
    //        0 | %              | 169 - 172
    //        1 | 257 - 260      | 177 - 180
    //        2 | 269 - 272      | 189 - 192
    //        3 | 281 - 284      | 201 - 204
    //        4 | 293 - 296      | 213 - 216
    //        5 | 301 - 304      | 221 - 224
    //        6 | 313 - 316      | 233 - 236
    //        7 | 325 - 328      | 245 - 248
    //        8 | 337 - 340      | 257 - 260
    //        9 | 345 - 348      | 265 - 268
    //       10 | 357 - 360      | 277 - 280
    //
    //      sprites/1spritesPrLine_m3stat_1_dmg08_cgb_out3
    //      sprites/1spritesPrLine_m3stat_2_dmg08_cgb_out0
    //      sprites/2spritesPrLine_m3stat_1_dmg08_cgb_out3
    //      sprites/2spritesPrLine_m3stat_2_dmg08_cgb_out0
    //      sprites/3spritesPrLine_m3stat_1_dmg08_cgb_out3
    //      sprites/3spritesPrLine_m3stat_2_dmg08_cgb_out0
    //      sprites/4spritesPrLine_m3stat_1_dmg08_cgb_out3
    //      sprites/4spritesPrLine_m3stat_2_dmg08_cgb_out0
    //      sprites/5spritesPrLine_m3stat_1_dmg08_cgb_out3
    //      sprites/5spritesPrLine_m3stat_2_dmg08_cgb_out0
    //      sprites/6spritesPrLine_m3stat_1_dmg08_cgb_out3
    //      sprites/6spritesPrLine_m3stat_2_dmg08_cgb_out0
    //      sprites/7spritesPrLine_m3stat_1_dmg08_cgb_out3
    //      sprites/7spritesPrLine_m3stat_2_dmg08_cgb_out0
    //      sprites/8spritesPrLine_m3stat_1_dmg08_cgb_out3
    //      sprites/8spritesPrLine_m3stat_2_dmg08_cgb_out0
    //      sprites/9spritesPrLine_m3stat_1_dmg08_cgb_out3
    //      sprites/9spritesPrLine_m3stat_2_dmg08_cgb_out0
    //      sprites/10spritesPrLine_m3stat_1_dmg08_cgb_out3
    //      sprites/10spritesPrLine_m3stat_2_dmg08_cgb_out0
    //
    // When running at double speed, mode 3 is flagged until
    // X-Y cycles after the LYC interrupt was raised.
    //
    // #sprites | LYC int cycles | mode 3 cycles
    // ---------+----------------+--------------
    //        0 | %              | 171 - 172
    //        1 | 263 - 264      | 183 - 184
    //        2 | 273 - 274      | 193 - 194
    //        3 | 285 - 286      | 205 - 206
    //        4 | 295 - 296      | 215 - 216
    //        5 | 307 - 308      | 227 - 228
    //        6 | 317 - 318      | 237 - 238
    //        7 | 329 - 330      | 249 - 250
    //        8 | 339 - 340      | 259 - 260
    //        9 | 351 - 352      | 271 - 272
    //       10 | 361 - 362      | 281 - 282
    //
    //      sprites/1spritesPrLine_m3stat_ds_1_out3
    //      sprites/1spritesPrLine_m3stat_ds_2_out0
    //      sprites/2spritesPrLine_m3stat_ds_1_out3
    //      sprites/2spritesPrLine_m3stat_ds_2_out0
    //      sprites/3spritesPrLine_m3stat_ds_1_out3
    //      sprites/3spritesPrLine_m3stat_ds_2_out0
    //      sprites/4spritesPrLine_m3stat_ds_1_out3
    //      sprites/4spritesPrLine_m3stat_ds_2_out0
    //      sprites/5spritesPrLine_m3stat_ds_1_out3
    //      sprites/5spritesPrLine_m3stat_ds_2_out0
    //      sprites/6spritesPrLine_m3stat_ds_1_out3
    //      sprites/6spritesPrLine_m3stat_ds_2_out0
    //      sprites/7spritesPrLine_m3stat_ds_1_out3
    //      sprites/7spritesPrLine_m3stat_ds_2_out0
    //      sprites/8spritesPrLine_m3stat_ds_1_out3
    //      sprites/8spritesPrLine_m3stat_ds_2_out0
    //      sprites/9spritesPrLine_m3stat_ds_1_out3
    //      sprites/9spritesPrLine_m3stat_ds_2_out0
    //      sprites/10spritesPrLine_m3stat_ds_1_out3
    //      sprites/10spritesPrLine_m3stat_ds_2_out0
    //

    AGE_ASSERT(ppu.m_pause_plotting);
    LOG_FETCH("");
    ppu.m_next_fetch_step = &sprite_step_1;
}

void age::gb_lcd_ppu::sprite_step_1(gb_lcd_ppu &ppu)
{
    AGE_ASSERT(ppu.m_pause_plotting);
    LOG_FETCH("");
    ppu.m_next_fetch_step = &sprite_step_2;
}

void age::gb_lcd_ppu::sprite_step_2(gb_lcd_ppu &ppu)
{
    AGE_ASSERT(ppu.m_pause_plotting);
    LOG_FETCH("fetching sprite tile byte 1");
    ppu.m_sprites[ppu.m_current_sprite].fetch_tile_byte1(ppu);
    ppu.m_next_fetch_step = &sprite_step_3;
}

void age::gb_lcd_ppu::sprite_step_3(gb_lcd_ppu &ppu)
{
    AGE_ASSERT(ppu.m_pause_plotting);
    LOG_FETCH("");
    ppu.m_next_fetch_step = &sprite_step_4;
}

void age::gb_lcd_ppu::sprite_step_4(gb_lcd_ppu &ppu)
{
    AGE_ASSERT(ppu.m_pause_plotting);
    LOG_FETCH("fetching sprite tile byte 2");
    ppu.m_sprites[ppu.m_current_sprite].fetch_tile_byte2(ppu);
    ppu.m_next_fetch_step = &sprite_step_5;
}

void age::gb_lcd_ppu::sprite_step_5(gb_lcd_ppu &ppu)
{
    AGE_ASSERT(ppu.m_pause_plotting);
    LOG_FETCH("");

    ppu.m_sprites[ppu.m_current_sprite].create_tile(ppu);
    ppu.m_sprite_tile_offset = 0;
    ppu.m_sprite_to_plot = ppu.m_current_sprite;

    ++ppu.m_current_sprite; // next sprite, potentially allow triggering mode 0
    ppu.m_next_fetch_step = &sprite_step_6;
}

void age::gb_lcd_ppu::sprite_step_6(gb_lcd_ppu &ppu)
{
    AGE_ASSERT(ppu.m_pause_plotting);
    LOG_FETCH("");
    ppu.update_pause_plotting();
    tile_step_5(ppu);
}





//---------------------------------------------------------
//
//   window initial data fetching
//
//---------------------------------------------------------

void age::gb_lcd_ppu::window_step_0(gb_lcd_ppu &ppu)
{
    //
    // verified by gambatte tests
    //
    // With the window visible on a scanline the mode 3 duration
    // is extended by 6 cycles, regardless of the current WX
    // and SCX values.
    //
    // When running at single speed, mode 3 is flagged until
    // X-Y cycles after the mode 2 interrupt was raised.
    //
    //  wx | scx | m2 int age | m3 duration
    //     |     | [cycles]   | [cycles]
    // ----+-----+------------+------------
    //  0  |  0  | 257 - 260  | 173 - 176
    //  3  |  0  | 257 - 260  | 173 - 176
    //  3  |  2  | 261 - 264  | 177 - 180
    //  3  |  3  | 261 - 264  | 177 - 180
    //  3  |  5  | 261 - 264  | 177 - 180
    //  7  |  0  | 257 - 260  | 173 - 176
    //  7  |  2  | 261 - 264  | 177 - 180
    //  7  |  3  | 261 - 264  | 177 - 180
    //  7  |  5  | 261 - 264  | 177 - 180
    //
    //      window/m2int_wx00_m3stat_1_dmg08_cgb_out3
    //      window/m2int_wx00_m3stat_2_dmg08_cgb_out0
    //      window/m2int_wx03_m3stat_1_dmg08_cgb_out3
    //      window/m2int_wx03_m3stat_2_dmg08_cgb_out0
    //      window/m2int_wx03_scx2_m3stat_1_dmg08_cgb_out3
    //      window/m2int_wx03_scx2_m3stat_2_dmg08_cgb_out0
    //      window/m2int_wx03_scx3_m3stat_1_dmg08_cgb_out3
    //      window/m2int_wx03_scx3_m3stat_2_dmg08_cgb_out0
    //      window/m2int_wx03_scx5_m3stat_1_dmg08_cgb_out3
    //      window/m2int_wx03_scx5_m3stat_2_dmg08_cgb_out0
    //      window/m2int_wx07_m3stat_1_dmg08_cgb_out3
    //      window/m2int_wx07_m3stat_2_dmg08_cgb_out0
    //      window/m2int_wx07_scx2_m3stat_1_dmg08_cgb_out3
    //      window/m2int_wx07_scx2_m3stat_2_dmg08_cgb_out0
    //      window/m2int_wx07_scx3_m3stat_1_dmg08_cgb_out3
    //      window/m2int_wx07_scx3_m3stat_2_dmg08_cgb_out0
    //      window/m2int_wx07_scx5_m3stat_1_dmg08_cgb_out3
    //      window/m2int_wx07_scx5_m3stat_2_dmg08_cgb_out0
    //
    // When running at double speed, mode 3 is flagged until
    // X-Y cycles after the mode 2 interrupt was raised.
    //
    //  wx | scx | m2 int age | m3 duration
    //     |     | [cycles]   | [cycles]
    // ----+-----+------------+------------
    //  3  |  0  | 259 - 260  | 177 - 178
    //  3  |  5  | 265 - 266  | 183 - 184
    //  7  |  0  | 259 - 260  | 177 - 178
    //  7  |  5  | 265 - 266  | 183 - 184
    // 12  |  0  | 259 - 260  | 177 - 178
    //
    //      window/m2int_wx03_m3stat_ds_1_out3
    //      window/m2int_wx03_m3stat_ds_2_out0
    //      window/m2int_wx03_scx5_m3stat_ds_1_out3
    //      window/m2int_wx03_scx5_m3stat_ds_2_out0
    //      window/m2int_wx07_m3stat_ds_1_out3
    //      window/m2int_wx07_m3stat_ds_2_out0
    //      window/m2int_wx07_scx5_m3stat_ds_1_out3
    //      window/m2int_wx07_scx5_m3stat_ds_2_out0
    //      window/m2int_wx0C_m3stat_ds_1_out3
    //      window/m2int_wx0C_m3stat_ds_2_out0
    //

    AGE_ASSERT(ppu.m_pause_plotting);
    LOG_FETCH("");
    ppu.fetch_tile_name();
    ppu.m_window_map_offset = 8 - ppu.m_x_current; // adjust for next tile with current m_x_current
    ppu.window_set_next_step(&window_step_1);
}

void age::gb_lcd_ppu::window_step_1(gb_lcd_ppu &ppu)
{
    AGE_ASSERT(ppu.m_pause_plotting);
    LOG_FETCH("");
    ppu.window_set_next_step(&window_step_2);
}

void age::gb_lcd_ppu::window_step_2(gb_lcd_ppu &ppu)
{
    AGE_ASSERT(ppu.m_pause_plotting);
    LOG_FETCH("");
    ppu.fetch_tile_byte_1();
    ppu.window_set_next_step(&window_step_3);
}

void age::gb_lcd_ppu::window_step_3(gb_lcd_ppu &ppu)
{
    AGE_ASSERT(ppu.m_pause_plotting);
    LOG_FETCH("");
    ppu.window_set_next_step(&window_step_4);
}

void age::gb_lcd_ppu::window_step_4(gb_lcd_ppu &ppu)
{
    AGE_ASSERT(ppu.m_pause_plotting);
    LOG_FETCH("");
    ppu.fetch_tile_byte_2();
    ppu.window_set_next_step(&window_step_5);
}

void age::gb_lcd_ppu::window_step_5(gb_lcd_ppu &ppu)
{
    AGE_ASSERT(ppu.m_pause_plotting);
    LOG_FETCH("");
    ppu.window_set_next_step(&window_step_6);
}

void age::gb_lcd_ppu::window_step_6(gb_lcd_ppu &ppu)
{
    AGE_ASSERT(ppu.m_pause_plotting);
    LOG_FETCH("");
    ppu.update_pause_plotting();
    tile_step_0(ppu);
}

void age::gb_lcd_ppu::window_set_next_step(std::function<void(gb_lcd_ppu&)> next_step)
{
    //
    // verified by gambatte tests
    //
    // On a CGB the window can be disabled even after the
    // window routine was started, if the background tile
    // was fully fetched and ready to render.
    //
    //      window/late_disable_1_dmg08_out3_cgb_out0
    //
    if (m_cgb && !m_win_enabled && (m_tile_offset == 8))
    {
        m_window_started = false;
        m_plotting_window = false;
        m_next_fetch_step = &window_step_6;
    }
    else
    {
        m_next_fetch_step = next_step;
    }
}





//---------------------------------------------------------
//
//   sprite class
//
//---------------------------------------------------------

age::gb_lcd_ppu::gb_sprite::gb_sprite()
{
}

age::gb_lcd_ppu::gb_sprite::gb_sprite(const uint8 *oam, uint line)
    : m_x(oam[1]),
      m_line(line),
      m_tile_name(oam[2]),
      m_tile_attributes(oam[3]),
      m_priority((m_tile_attributes & gb_tile_attribute_priority) >> 1)
{
    if (m_x > 167)
    {
        m_x = 255; // prevent clash with m_x_current == 168
    }
}



void age::gb_lcd_ppu::gb_sprite::fetch_tile_byte1(const gb_lcd_ppu &ppu)
{
    uint index = get_tile_data_offset(ppu);
    m_tile_byte1 = ppu.m_memory.get_video_ram()[index];
}

void age::gb_lcd_ppu::gb_sprite::fetch_tile_byte2(const gb_lcd_ppu &ppu)
{
    uint index = get_tile_data_offset(ppu);
    m_tile_byte2 = ppu.m_memory.get_video_ram()[index + 1];
}

void age::gb_lcd_ppu::gb_sprite::create_tile(const gb_lcd_ppu &ppu)
{
    uint8 palette_offset = 0x20 + (ppu.m_cgb ? ((m_tile_attributes & 0x07) << 2) : (m_tile_attributes & 0x10));

    // flip horizontally, if requested
    if ((m_tile_attributes & gb_tile_attribute_x_flip) > 0) // attribute: horizontal flip
    {
        m_tile_byte1 = ppu.m_xflip_cache[m_tile_byte1];
        m_tile_byte2 = ppu.m_xflip_cache[m_tile_byte2];
    }

    // calculate tile cache index
    uint index1 = (m_tile_byte1 & 0xF0) + (m_tile_byte2 >> 4);
    uint index2 = ((m_tile_byte1 & 0x0F) << 4) + (m_tile_byte2 & 0x0F);
    index1 <<= 2;
    index2 <<= 2;

    // cache color indexes
    m_tile[0] = ppu.m_tile_cache[index1 + 0] + palette_offset;
    m_tile[1] = ppu.m_tile_cache[index1 + 1] + palette_offset;
    m_tile[2] = ppu.m_tile_cache[index1 + 2] + palette_offset;
    m_tile[3] = ppu.m_tile_cache[index1 + 3] + palette_offset;
    m_tile[4] = ppu.m_tile_cache[index2 + 0] + palette_offset;
    m_tile[5] = ppu.m_tile_cache[index2 + 1] + palette_offset;
    m_tile[6] = ppu.m_tile_cache[index2 + 2] + palette_offset;
    m_tile[7] = ppu.m_tile_cache[index2 + 3] + palette_offset;
}

age::uint8 age::gb_lcd_ppu::gb_sprite::get_priority() const
{
    return m_priority;
}

age::uint8 age::gb_lcd_ppu::gb_sprite::get_color_index(uint offset) const
{
    return m_tile[offset];
}

age::uint age::gb_lcd_ppu::gb_sprite::get_x() const
{
    return m_x;
}

bool age::gb_lcd_ppu::gb_sprite::operator<(const gb_sprite &right) const
{
    return m_x < right.m_x;
}



age::uint age::gb_lcd_ppu::gb_sprite::get_tile_data_offset(const gb_lcd_ppu &ppu) const
{
    uint tile_data_offset = m_line << 1;

    // flip vertically, if requested
    if ((m_tile_attributes & gb_tile_attribute_y_flip) > 0)
    {
        tile_data_offset ^= 0x1E;
    }

    // adjust to object size
    if (ppu.m_obj_size_16)
    {
        tile_data_offset += (m_tile_name & 0xFE) << 4;
    }
    else
    {
        tile_data_offset &= 0x0E;
        tile_data_offset += m_tile_name << 4;
    }

    // add character bank offset (tile attribute)
    tile_data_offset += (m_tile_attributes & 0x08) << 10;

    // done
    return tile_data_offset;
}
