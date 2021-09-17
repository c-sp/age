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

#include <age_debug.hpp>

#include "age_gb_lcd_render.hpp"

#include <algorithm> // std::sort



namespace
{
    constexpr uint8_t oam_palette = 0x10;

} // namespace



age::gb_lcd_sprites::gb_lcd_sprites(bool cgb_features)
    : m_cgb_features(cgb_features),
      m_attribute_mask(cgb_features ? 0xFF : 0xF0)
{
}



age::uint8_t age::gb_lcd_sprites::read_oam(int offset) const
{
    AGE_ASSERT((offset >= 0) && (offset < 160));
    return m_oam[offset];
}

void age::gb_lcd_sprites::write_oam(int offset, uint8_t value)
{
    AGE_ASSERT((offset >= 0) && (offset < 160));
    m_oam[offset] = value;
}

age::uint8_t age::gb_lcd_sprites::get_tile_nr_mask() const
{
    return m_tile_nr_mask;
}

age::uint8_t age::gb_lcd_sprites::get_sprite_size() const
{
    return m_sprite_size;
}

void age::gb_lcd_sprites::set_sprite_size(uint8_t sprite_size)
{
    AGE_ASSERT((sprite_size == 8) || (sprite_size == 16))
    m_sprite_size  = sprite_size;
    m_tile_nr_mask = (sprite_size == 16) ? 0xFE : 0xFF;
}



std::vector<age::gb_sprite> age::gb_lcd_sprites::get_line_sprites(int line)
{
    // find the first 10 sprites on this line
    std::vector<gb_sprite> sprites;

    for (uint8_t sprite_id = 0; sprite_id < 40; ++sprite_id)
    {
        uint32_t oam = 0;
        memcpy(&oam, &m_oam[sprite_id * 4], sizeof(uint32_t));

        auto sprite               = gb_sprite{.m_parts = {.m_oam = oam, .m_custom = 0}};
        sprite.m_data.m_sprite_id = sprite_id;
        sprite.m_data.m_attributes &= m_attribute_mask;

        sprite.m_data.m_palette_idx = m_cgb_features
                                          ? (8 + (sprite.m_data.m_attributes & gb_tile_attrib_palette))
                                          : ((sprite.m_data.m_attributes & oam_palette) ? gb_palette_obp1 : gb_palette_obp0);

        int y_diff = line - (sprite.m_data.m_y - 16);
        if ((y_diff < 0) || (y_diff >= m_sprite_size))
        {
            continue;
        }

        sprites.push_back(sprite);
        if (sprites.size() >= 10)
        {
            break;
        }
    }

    // non-CGB mode: sort sprites by X coordinate
    if (!m_cgb_features)
    {
        std::sort(begin(sprites),
                  end(sprites),
                  [&](const gb_sprite& a, const gb_sprite& b) {
                      return (a.m_data.m_x == b.m_data.m_x)
                                 ? (a.m_data.m_sprite_id < b.m_data.m_sprite_id)
                                 : (a.m_data.m_x < b.m_data.m_x);
                  });
    }

    return sprites;
}
