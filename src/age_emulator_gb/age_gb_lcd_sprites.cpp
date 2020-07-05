//
// Copyright 2019 Christoph Sprenger
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



age::gb_lcd_sprites::gb_lcd_sprites(bool sprite_x_priority)
    : m_sprite_x_priority(sprite_x_priority)
{
}



age::uint8_t age::gb_lcd_sprites::read_oam(int offset) const
{
    return m_oam[offset];
}

void age::gb_lcd_sprites::write_oam(int offset, uint8_t value)
{
    m_oam[offset] = value;
    m_dirty = true;
}

void age::gb_lcd_sprites::set_sprite_size(uint8_t sprite_size)
{
    m_dirty |= (sprite_size != m_sprite_size);
    m_sprite_size = sprite_size;
}



std::vector<age::gb_sprite> age::gb_lcd_sprites::get_scanline_sprites(int scanline)
{
    // find the first 10 sprites on this scanline
    std::vector<gb_sprite> sprites;

    for (auto it = begin(m_oam); it != end(m_oam); it += 4)
    {
        gb_sprite sprite = *(gb_sprite*)it; //! \todo ugly cast
        int y_diff = scanline - sprite.m_oam.m_y - 8;
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

    //! \todo sort sprites in non-CGB mode
    if (m_sprite_x_priority)
    {
    }

    return sprites;
}
