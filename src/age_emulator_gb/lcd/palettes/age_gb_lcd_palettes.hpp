//
// © 2021 Christoph Sprenger <https://github.com/c-sp>
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

#ifndef AGE_GB_LCD_PALETTES_HPP
#define AGE_GB_LCD_PALETTES_HPP

//!
//! \file
//!

#include "../../common/age_gb_device.hpp"

#include <emulator/age_gb_types.hpp>

#include <age_types.hpp>
#include <gfx/age_pixel.hpp>

#include <span>



namespace age
{
    //! The CGB has 8 palettes for BG and OBJ each.
    constexpr unsigned gb_palette_count = 16;
    //! Every palette contains 4 colors.
    constexpr unsigned gb_total_color_count = gb_palette_count * 4;

    constexpr unsigned gb_palette_bgp  = 0;
    constexpr unsigned gb_palette_obp0 = 8;
    constexpr unsigned gb_palette_obp1 = 12;



    pixel cgb_color_correction(unsigned cgb_rgb15);



    class gb_lcd_palettes
    {
        AGE_DISABLE_COPY(gb_lcd_palettes);
        AGE_DISABLE_MOVE(gb_lcd_palettes);

    public:
        gb_lcd_palettes(const gb_device&         device,
                        std::span<uint8_t const> rom_header,
                        gb_colors_hint           colors_hint);

        ~gb_lcd_palettes() = default;

        [[nodiscard]] std::span<const pixel, 4> get_palette(unsigned palette_index) const;
        [[nodiscard]] pixel                     get_color(unsigned color_index) const;
        [[nodiscard]] pixel                     get_color_bgp_glitch(unsigned color_index) const;
        [[nodiscard]] pixel                     get_color_zero_dmg() const;

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
        void init_dmg_colors(std::span<uint8_t const> rom_header);

        void  update_dmg_palette(unsigned palette_index, uint8_t value);
        void  update_cgb_color(unsigned color_index);
        pixel lookup_cgb_color(unsigned cgb_rgb15);

        const gb_device&     m_device;
        const gb_colors_hint m_colors_hint;

        // CGB:     0x00 - 0x3F  BG
        //          0x40 - 0x7F  OBJ
        uint8_array<gb_total_color_count * 2ULL> m_cpd{}; // 2 bytes per color
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

        uint8_t m_bgp          = 0xFC;
        uint8_t m_previous_bgp = 0xFC;
        uint8_t m_obp0         = 0xFF;
        uint8_t m_obp1         = 0xFF;
        uint8_t m_bcps         = 0xC0;
        uint8_t m_ocps         = 0xC1;
    };

} // namespace age



#endif // AGE_GB_LCD_PALETTES_HPP
