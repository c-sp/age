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

#ifndef AGE_GB_LCD_SPRITES_HPP
#define AGE_GB_LCD_SPRITES_HPP

//!
//! \file
//!

#include "../palettes/age_gb_lcd_palettes.hpp"

#include <age_debug.hpp>
#include <age_types.hpp>

#include <algorithm> // std::sort



namespace age
{
    constexpr uint8_t gb_tile_attrib_cgb_palette = 0x07;
    constexpr uint8_t gb_tile_attrib_vram_bank   = 0x08;
    constexpr uint8_t gb_tile_attrib_dmg_palette = 0x10;
    constexpr uint8_t gb_tile_attrib_flip_x      = 0x20;
    constexpr uint8_t gb_tile_attrib_flip_y      = 0x40;
    constexpr uint8_t gb_tile_attrib_priority    = 0x80;

    constexpr uint8_t gb_oam_ofs_y          = 0;
    constexpr uint8_t gb_oam_ofs_x          = 1;
    constexpr uint8_t gb_oam_ofs_tile       = 2;
    constexpr uint8_t gb_oam_ofs_attributes = 3;



    struct gb_sprite
    {
        uint8_t m_y;
        uint8_t m_x;
        uint8_t m_tile_nr;
        uint8_t m_attributes;
        uint8_t m_sprite_id;
        uint8_t m_palette_idx;
    };



    class gb_lcd_sprites
    {
        AGE_DISABLE_COPY(gb_lcd_sprites);
        AGE_DISABLE_MOVE(gb_lcd_sprites);

    public:
        explicit gb_lcd_sprites(bool cgb_mode)
            : m_cgb_mode(cgb_mode),
              m_attribute_mask(cgb_mode ? 0xFF : 0xF0)
        {}

        ~gb_lcd_sprites() = default;



        [[nodiscard]] uint8_t read_oam(int offset) const
        {
            AGE_ASSERT((offset >= 0) && (offset < 160))
            return m_oam[offset];
        }

        void write_oam(int offset, uint8_t value)
        {
            AGE_ASSERT((offset >= 0) && (offset < 160))
            m_oam[offset] = value;
        }



        [[nodiscard]] uint8_t get_tile_nr_mask() const
        {
            return m_tile_nr_mask;
        }

        [[nodiscard]] uint8_t get_sprite_size() const
        {
            return m_sprite_size;
        }

        void set_sprite_size(uint8_t sprite_size)
        {
            AGE_ASSERT((sprite_size == 8) || (sprite_size == 16))
            m_sprite_size  = sprite_size;
            m_tile_nr_mask = (sprite_size == 16) ? 0xFE : 0xFF;
        }



        [[nodiscard]] std::vector<gb_sprite> get_line_sprites(int line, bool sort_by_x) const
        {
            // find the first 10 sprites on this line
            std::vector<gb_sprite> sprites;

            for (uint8_t sprite_id = 0; sprite_id < 40; ++sprite_id)
            {
                unsigned oam_ofs = sprite_id * 4U;

                uint8_t y      = m_oam[oam_ofs + gb_oam_ofs_y];
                int     y_diff = line + 16 - y;
                if ((y_diff < 0) || (y_diff >= m_sprite_size))
                {
                    continue;
                }

                gb_sprite sprite{};
                sprite.m_y           = y;
                sprite.m_x           = m_oam[oam_ofs + gb_oam_ofs_x];
                sprite.m_tile_nr     = m_oam[oam_ofs + gb_oam_ofs_tile];
                sprite.m_attributes  = m_oam[oam_ofs + gb_oam_ofs_attributes];
                sprite.m_sprite_id   = sprite_id;
                sprite.m_palette_idx = m_cgb_mode
                                           ? (8 + (sprite.m_attributes & gb_tile_attrib_cgb_palette))
                                       : (sprite.m_attributes & gb_tile_attrib_dmg_palette)
                                           ? gb_palette_obp1
                                           : gb_palette_obp0;

                sprites.push_back(sprite);
                if (sprites.size() >= 10)
                {
                    break;
                }
            }

            // non-CGB mode: sort sprites by X coordinate
            if (sort_by_x)
            {
                std::sort(begin(sprites),
                          end(sprites),
                          [&](const gb_sprite& a, const gb_sprite& b) {
                              return (a.m_x == b.m_x)
                                         ? (a.m_sprite_id < b.m_sprite_id)
                                         : (a.m_x < b.m_x);
                          });
            }

            return sprites;
        }

    private:
        uint8_array<160> m_oam{};

        uint8_t m_sprite_size  = 8;
        uint8_t m_tile_nr_mask = 0xFF;

        const bool    m_cgb_mode;
        const uint8_t m_attribute_mask;
    };

} // namespace age



#endif // AGE_GB_LCD_SPRITES_HPP
