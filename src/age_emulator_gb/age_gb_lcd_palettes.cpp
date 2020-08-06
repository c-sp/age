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

#include "age_gb_lcd.hpp"

namespace
{

age::uint8_t increment_cps(age::uint8_t cps)
{
    if (cps & 0x80)
    {
        uint8_t index = (cps + 1) & 0x3F;
        cps &= 0xC0;
        cps |= index;
    }
    return cps;
}

}



age::gb_lcd_palettes::gb_lcd_palettes(const gb_device &device,
                                      bool dmg_green)
    : m_device(device),
      m_dmg_green(dmg_green)
{
    update_dmg_palette(gb_palette_bgp, m_bgp);
    update_dmg_palette(gb_palette_obp0, m_obp0);
    update_dmg_palette(gb_palette_obp1, m_obp1);
}



const age::pixel* age::gb_lcd_palettes::get_palette(unsigned palette_index) const
{
    AGE_ASSERT(palette_index < gb_palette_count);
    return &m_colors[palette_index << 2];
}

age::uint8_t age::gb_lcd_palettes::read_bgp() const
{
    return m_bgp;
}

age::uint8_t age::gb_lcd_palettes::read_obp0() const
{
    return m_obp0;
}

age::uint8_t age::gb_lcd_palettes::read_obp1() const
{
    return m_obp1;
}

age::uint8_t age::gb_lcd_palettes::read_bcps() const
{
    AGE_ASSERT(m_device.is_cgb());
    return m_bcps;
}

age::uint8_t age::gb_lcd_palettes::read_bcpd() const
{
    AGE_ASSERT(m_device.is_cgb());
    return m_cpd[m_bcps & 0x3F];
}

age::uint8_t age::gb_lcd_palettes::read_ocps() const
{
    AGE_ASSERT(m_device.is_cgb());
    return m_ocps;
}

age::uint8_t age::gb_lcd_palettes::read_ocpd() const
{
    AGE_ASSERT(m_device.is_cgb());
    return m_cpd[0x40 + (m_ocps & 0x3F)];
}



void age::gb_lcd_palettes::write_bgp(uint8_t value)
{
    update_dmg_palette(gb_palette_bgp, value);
    m_bgp = value;
}

void age::gb_lcd_palettes::write_obp0(uint8_t value)
{
    update_dmg_palette(gb_palette_obp0, value);
    m_obp0 = value;
}

void age::gb_lcd_palettes::write_obp1(uint8_t value)
{
    update_dmg_palette(gb_palette_obp1, value);
    m_obp1 = value;
}

void age::gb_lcd_palettes::write_bcps(uint8_t value)
{
    AGE_ASSERT(m_device.is_cgb());
    m_bcps = value | 0x40;
}

void age::gb_lcd_palettes::write_bcpd(uint8_t value)
{
    AGE_ASSERT(m_device.is_cgb());

    unsigned cpd_index = m_bcps & 0x3F;
    m_cpd[cpd_index] = value;
    update_cgb_color(cpd_index >> 1);

    m_bcps = increment_cps(m_bcps);
}

void age::gb_lcd_palettes::write_ocps(uint8_t value)
{
    AGE_ASSERT(m_device.is_cgb());
    m_ocps = value | 0x40;
}

void age::gb_lcd_palettes::write_ocpd(uint8_t value)
{
    AGE_ASSERT(m_device.is_cgb());

    unsigned cpd_index = 0x40 + (m_ocps & 0x3F);
    m_cpd[cpd_index] = value;
    update_cgb_color(cpd_index >> 1);

    m_ocps = increment_cps(m_ocps);
}



void age::gb_lcd_palettes::update_dmg_palette(unsigned palette_index, uint8_t value)
{
    if (m_device.is_cgb())
    {
        return;
    }
    AGE_ASSERT(palette_index < gb_palette_count);
    unsigned color_index = palette_index << 2;

    // greenish Gameboy colors
    // (taken from some googled gameboy photo)
    if (m_dmg_green)
    {
        for (unsigned idx = color_index, max = color_index + 4; idx < max; ++idx)
        {
            switch (value & 0x03)
            {
                case 0x00: m_colors[idx] = pixel(152, 192, 15); break;
                case 0x01: m_colors[idx] = pixel(112, 152, 15); break;
                case 0x02: m_colors[idx] = pixel(48, 96, 15); break;
                case 0x03: m_colors[idx] = pixel(15, 56, 15); break;
            }
            value >>= 2;
        }
        return;
    }

    // grayish Gameboy colors, used by test runner
    for (unsigned idx = color_index, max = color_index + 4; idx < max; ++idx)
    {
        switch (value & 0x03)
        {
            case 0x00: m_colors[idx] = pixel(255, 255, 255); break;
            case 0x01: m_colors[idx] = pixel(170, 170, 170); break;
            case 0x02: m_colors[idx] = pixel(85, 85, 85); break;
            case 0x03: m_colors[idx] = pixel(0, 0, 0); break;
        }
        value >>= 2;
    }
}

void age::gb_lcd_palettes::update_cgb_color(unsigned color_index)
{
    AGE_ASSERT(m_device.is_cgb());
    AGE_ASSERT(color_index < gb_total_color_count);

    unsigned cpd_index = color_index << 1;
    int low_byte = m_cpd[cpd_index];
    int high_byte = m_cpd[cpd_index + 1];
    int gb_color = (high_byte << 8) + low_byte;

    // gb-color:   red:    bits 0-4
    //             green:  bits 5-9
    //             blue:   bits 10-14
    int gb_r = gb_color & 0x1F;
    int gb_g = (gb_color >> 5) & 0x1F;
    int gb_b = (gb_color >> 10) & 0x1F;

    // formula copied from gambatte (video.cpp)
    int r = (gb_r * 13 + gb_g * 2 + gb_b) >> 1;
    int g = (gb_g * 3 + gb_b) << 1;
    int b = (gb_r * 3 + gb_g * 2 + gb_b * 11) >> 1;

    m_colors[color_index] = pixel(r, g, b);
}
