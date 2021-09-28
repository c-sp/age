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
#include <emulator/age_gb_types.hpp>
#include <gfx/age_pixel.hpp>
#include <gfx/age_screen_buffer.hpp>

#include "../common/age_gb_device.hpp"



namespace age
{

    constexpr uint8_t gb_tile_attrib_palette   = 0x07;
    constexpr uint8_t gb_tile_attrib_vram_bank = 0x08;
    constexpr uint8_t gb_tile_attrib_flip_x    = 0x20;
    constexpr uint8_t gb_tile_attrib_flip_y    = 0x40;
    constexpr uint8_t gb_tile_attrib_priority  = 0x80;



    //! The CGB has 8 palettes for BG and OBJ each.
    constexpr unsigned gb_palette_count = 16;
    //! Every palette contains 4 colors.
    constexpr unsigned gb_total_color_count = gb_palette_count * 4;

    constexpr unsigned gb_palette_bgp  = 0;
    constexpr unsigned gb_palette_obp0 = 8;
    constexpr unsigned gb_palette_obp1 = 12;

    class gb_lcd_palettes
    {
        AGE_DISABLE_COPY(gb_lcd_palettes);
        AGE_DISABLE_MOVE(gb_lcd_palettes);

    public:
        gb_lcd_palettes(const gb_device& device, const uint8_t* rom_header, gb_colors_hint colors_hint);
        ~gb_lcd_palettes() = default;

        [[nodiscard]] const pixel* get_palette(unsigned palette_index) const;

        [[nodiscard]] uint8_t read_bgp() const;
        [[nodiscard]] uint8_t read_obp0() const;
        [[nodiscard]] uint8_t read_obp1() const;
        [[nodiscard]] uint8_t read_bcps() const;
        [[nodiscard]] uint8_t read_bcpd() const;
        [[nodiscard]] uint8_t read_ocps() const;
        [[nodiscard]] uint8_t read_ocpd() const;

        void write_bgp(uint8_t value);
        void write_obp0(uint8_t value);
        void write_obp1(uint8_t value);
        void write_bcps(uint8_t value);
        void write_bcpd(uint8_t value);
        void write_ocps(uint8_t value);
        void write_ocpd(uint8_t value);

    private:
        void init_dmg_colors(const age::uint8_t* rom_header);

        void update_dmg_palette(unsigned palette_index, uint8_t value);
        void update_cgb_color(unsigned color_index);
        pixel lookup_cgb_color(unsigned cgb_rgb15);

        const gb_device&     m_device;
        const gb_colors_hint m_colors_hint;

        // CGB:     0x00 - 0x3F  BG
        //          0x40 - 0x7F  OBJ
        uint8_array<gb_total_color_count * 2ULL> m_cpd; // 2 bytes per color
        //
        // DMG:     0x00 - 0x03  BGP
        //          0x20 - 0x23  OBP0
        //          0x30 - 0x33  OBP1
        //
        // CGB:     0x00 - 0x1F  BG
        //          0x20 - 0x3F  OBJ
        pixel_vector m_colors{gb_total_color_count, pixel(0, 0, 0)};

        // default classic Game Boy colors (from 0x00 to 0x03)
        std::array<pixel, 4> m_bgp_colors{{pixel(0x98C00F), pixel(0x70980F), pixel(0x30600F), pixel(0x0F380F)}};
        std::array<pixel, 4> m_obp0_colors{{pixel(0x98C00F), pixel(0x70980F), pixel(0x30600F), pixel(0x0F380F)}};
        std::array<pixel, 4> m_obp1_colors{{pixel(0x98C00F), pixel(0x70980F), pixel(0x30600F), pixel(0x0F380F)}};

        pixel_vector m_cgb_color_lut{};

        uint8_t m_bgp  = 0xFC;
        uint8_t m_obp0 = 0xFF;
        uint8_t m_obp1 = 0xFF;
        uint8_t m_bcps = 0xC0;
        uint8_t m_ocps = 0xC1;
    };

    pixel correct_cgb_color(unsigned cgb_rgb15);



    union gb_sprite
    {
        uint64_t m_value;
        struct
        {
            uint32_t m_oam;
            uint32_t m_custom;
        } m_parts;
        struct
        {
            uint8_t m_y;
            uint8_t m_x;
            uint8_t m_tile_nr;
            uint8_t m_attributes;
            uint8_t m_sprite_id;
            uint8_t m_palette_idx;
        } m_data;
    };

    static_assert(sizeof(gb_sprite) == 8, "expected gb_sprite size of 8 bytes");

    class gb_lcd_sprites
    {
        AGE_DISABLE_COPY(gb_lcd_sprites);
        AGE_DISABLE_MOVE(gb_lcd_sprites);

    public:
        explicit gb_lcd_sprites(bool cgb_features);
        ~gb_lcd_sprites() = default;

        [[nodiscard]] uint8_t read_oam(int offset) const;
        void                  write_oam(int offset, uint8_t value);

        [[nodiscard]] uint8_t get_tile_nr_mask() const;
        [[nodiscard]] uint8_t get_sprite_size() const;
        void                  set_sprite_size(uint8_t sprite_size);

        std::vector<gb_sprite> get_line_sprites(int line);

    private:
        uint8_array<160> m_oam;

        uint8_t m_sprite_size  = 8;
        uint8_t m_tile_nr_mask = 0xFF;

        const bool    m_cgb_mode;
        const uint8_t m_attribute_mask;
    };



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

    class gb_lcd_render
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

        [[nodiscard]] uint8_t get_lcdc() const;
        void                  set_lcdc(uint8_t lcdc);

        void new_frame();
        void render(int until_line);

    private:
        void   render_line(int line);
        bool   window_visible(int line);
        pixel* render_bg_tile(pixel* dst, int tile_line, int tile_vram_ofs);
        void   render_sprite_tile(pixel* dst, int tile_line, const gb_sprite& sprite);

        const gb_device&       m_device;
        const gb_lcd_palettes& m_palettes;
        gb_lcd_sprites&        m_sprites;
        screen_buffer&         m_screen_buffer;

        const uint8_t*   m_video_ram;
        uint8_array<256> m_xflip_cache;

        // 160px + 3 tiles (8px + scx + last window/sprite tile)
        pixel_vector m_line{gb_screen_width + 24, pixel(0, 0, 0)};
        int          m_rendered_lines = 0;
        int          m_wline          = -1;

        int     m_bg_tile_map_offset  = 0;
        int     m_win_tile_map_offset = 0;
        int     m_tile_data_offset    = 0;
        uint8_t m_tile_xor            = 0;
        uint8_t m_priority_mask       = 0xFF;

        uint8_t m_lcdc = 0;

    public:
        uint8_t m_scy = 0;
        uint8_t m_scx = 0;
        uint8_t m_wy  = 0;
        uint8_t m_wx  = 0;
    };

} // namespace age



#endif // AGE_GB_LCD_RENDER_HPP
