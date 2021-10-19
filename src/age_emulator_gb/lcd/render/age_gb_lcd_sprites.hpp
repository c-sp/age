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

#include <age_types.hpp>



namespace age
{
    constexpr uint8_t gb_tile_attrib_palette   = 0x07;
    constexpr uint8_t gb_tile_attrib_vram_bank = 0x08;
    constexpr uint8_t gb_tile_attrib_flip_x    = 0x20;
    constexpr uint8_t gb_tile_attrib_flip_y    = 0x40;
    constexpr uint8_t gb_tile_attrib_priority  = 0x80;



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
        explicit gb_lcd_sprites(bool cgb_mode);
        ~gb_lcd_sprites() = default;

        [[nodiscard]] uint8_t read_oam(int offset) const;
        void                  write_oam(int offset, uint8_t value);

        [[nodiscard]] uint8_t get_tile_nr_mask() const;
        [[nodiscard]] uint8_t get_sprite_size() const;
        void                  set_sprite_size(uint8_t sprite_size);

        [[nodiscard]] std::vector<gb_sprite> get_line_sprites(int line) const;

    private:
        uint8_array<160> m_oam{};

        uint8_t m_sprite_size  = 8;
        uint8_t m_tile_nr_mask = 0xFF;

        const bool    m_cgb_mode;
        const uint8_t m_attribute_mask;
    };

} // namespace age



#endif // AGE_GB_LCD_SPRITES_HPP
