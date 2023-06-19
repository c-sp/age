//
// Copyright 2022 Christoph Sprenger
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

#ifndef AGE_GB_LCD_RENDERER_COMMON_HPP
#define AGE_GB_LCD_RENDERER_COMMON_HPP

//!
//! \file
//!

#include "../../common/age_gb_device.hpp"
#include "../common/age_gb_lcd_common.hpp"
#include "age_gb_lcd_sprites.hpp"

#include <age_types.hpp>



namespace age
{
    //! common logic used by line- and fifo-renderer
    //!
    class gb_lcd_renderer_common
    {
        AGE_DISABLE_COPY(gb_lcd_renderer_common);
        AGE_DISABLE_MOVE(gb_lcd_renderer_common);

    public:
        gb_lcd_renderer_common(const gb_device& device, gb_lcd_sprites& sprites)
            : m_xflip_cache(calculate_xflip_lut()),
              m_device(device),
              m_sprites(sprites)
        {
            set_lcdc(0x91);
        }

        ~gb_lcd_renderer_common() = default;



        const uint8_array<256> m_xflip_cache;
        const gb_device&       m_device;

        int     m_bg_tile_map_offset  = 0;
        int     m_win_tile_map_offset = 0;
        int     m_tile_data_offset    = 0;
        uint8_t m_tile_xor            = 0;
        uint8_t m_priority_mask       = 0xFF;

        uint8_t m_scy = 0;
        uint8_t m_scx = 0;
        uint8_t m_wy  = 0;
        uint8_t m_wx  = 0;



        [[nodiscard]] uint8_t get_lcdc() const
        {
            return m_lcdc;
        }

        void set_lcdc(uint8_t lcdc)
        {
            m_lcdc = lcdc;

            m_sprites.set_sprite_size((lcdc & gb_lcdc_obj_size) ? 16 : 8);

            m_bg_tile_map_offset  = (lcdc & gb_lcdc_bg_map) ? 0x1C00 : 0x1800;
            m_win_tile_map_offset = (lcdc & gb_lcdc_win_map) ? 0x1C00 : 0x1800;

            m_tile_data_offset = (lcdc & gb_lcdc_bg_win_data) ? 0x0000 : 0x0800;
            m_tile_xor         = (lcdc & gb_lcdc_bg_win_data) ? 0 : 0x80;

            if (m_device.cgb_mode())
            {
                // CGB: if LCDC bit 0 is 0, sprites are always displayed above
                // BG & window regardless of any priority flags
                m_priority_mask = (lcdc & gb_lcdc_bg_enable) ? 0xFF : 0x00;
            }
        }

    private:
        uint8_t m_lcdc = 0;

        gb_lcd_sprites& m_sprites;

        static age::uint8_array<256> calculate_xflip_lut()
        {
            age::uint8_array<256> xflip;

            for (unsigned byte = 0; byte < 256; ++byte)
            {
                age::uint8_t flip_byte = 0;

                for (unsigned bit = 0x01, flip_bit = 0x80;
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

            return xflip;
        }
    };

} // namespace age



#endif // AGE_GB_LCD_RENDERER_COMMON_HPP
