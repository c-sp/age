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

#include "age_gb_lcd_palettes.hpp"

#include <age_debug.hpp>



namespace
{
    age::uint8_t increment_cps(age::uint8_t cps)
    {
        if (cps & 0x80)
        {
            uint8_t index = (cps + 1U) & 0x3FU;
            cps &= 0xC0;
            cps |= index;
        }
        return cps;
    }

} // namespace



age::gb_lcd_palettes::gb_lcd_palettes(const gb_device& device,
                                      const uint8_t*   rom_header,
                                      gb_colors_hint   colors_hint)
    : m_device(device),
      m_colors_hint(colors_hint)
{
    if (!m_device.cgb_mode())
    {
        // setup DMG palettes
        if (device.cgb_in_dmg_mode())
        {
            init_dmg_colors(rom_header);
        }
        else if (m_colors_hint == gb_colors_hint::dmg_greyscale)
        {
            m_bgp_colors  = {{pixel(0xFFFFFF), pixel(0xAAAAAA), pixel(0x555555), pixel(0x000000)}};
            m_obp0_colors = m_bgp_colors;
            m_obp1_colors = m_bgp_colors;
        }

        update_dmg_palette(gb_palette_bgp, m_bgp);
        update_dmg_palette(gb_palette_obp0, m_obp0);
        update_dmg_palette(gb_palette_obp1, m_obp1);
    }
}



const age::pixel* age::gb_lcd_palettes::get_palette(unsigned palette_index) const
{
    AGE_ASSERT(palette_index < gb_palette_count)
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
    AGE_ASSERT(m_device.cgb_mode())
    return m_bcps;
}

age::uint8_t age::gb_lcd_palettes::read_bcpd() const
{
    AGE_ASSERT(m_device.cgb_mode())
    return m_cpd[m_bcps & 0x3F];
}

age::uint8_t age::gb_lcd_palettes::read_ocps() const
{
    AGE_ASSERT(m_device.cgb_mode())
    return m_ocps;
}

age::uint8_t age::gb_lcd_palettes::read_ocpd() const
{
    AGE_ASSERT(m_device.cgb_mode())
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
    AGE_ASSERT(m_device.cgb_mode())
    m_bcps = value | 0x40;
}

void age::gb_lcd_palettes::write_bcpd(uint8_t value)
{
    AGE_ASSERT(m_device.cgb_mode())

    unsigned cpd_index = m_bcps & 0x3F;
    m_cpd[cpd_index]   = value;
    update_cgb_color(cpd_index >> 1);

    m_bcps = increment_cps(m_bcps);
}

void age::gb_lcd_palettes::write_ocps(uint8_t value)
{
    AGE_ASSERT(m_device.cgb_mode())
    m_ocps = value | 0x40;
}

void age::gb_lcd_palettes::write_ocpd(uint8_t value)
{
    AGE_ASSERT(m_device.cgb_mode())

    unsigned cpd_index = 0x40 + (m_ocps & 0x3F);
    m_cpd[cpd_index]   = value;
    update_cgb_color(cpd_index >> 1);

    m_ocps = increment_cps(m_ocps);
}



void age::gb_lcd_palettes::update_dmg_palette(unsigned palette_index, uint8_t value)
{
    if (m_device.cgb_mode())
    {
        return;
    }
    AGE_ASSERT((palette_index == gb_palette_bgp) || (palette_index == gb_palette_obp0) || (palette_index == gb_palette_obp1))
    unsigned color_index = palette_index << 2;

    const auto& color_src = [&]() {
        switch (palette_index)
        {
            case gb_palette_obp0:
                return m_obp0_colors;

            case gb_palette_obp1:
                return m_obp1_colors;

            default:
                return m_bgp_colors;
        }
    }();

    for (unsigned idx = color_index, max = color_index + 4; idx < max; ++idx)
    {
        m_colors[idx] = color_src[value & 0x03];
        value >>= 2;
    }
}



void age::gb_lcd_palettes::update_cgb_color(unsigned color_index)
{
    AGE_ASSERT(m_device.cgb_mode())
    AGE_ASSERT(color_index < gb_total_color_count)

    unsigned cpd_index = color_index << 1;
    unsigned low_byte  = m_cpd[cpd_index];
    unsigned high_byte = m_cpd[cpd_index + 1];
    unsigned cgb_rgb15 = (high_byte << 8) | low_byte;

    // gb-color:   red:    bits 0-4
    //             green:  bits 5-9
    //             blue:   bits 10-14
    unsigned gb_r = cgb_rgb15 & 0x1FU;
    unsigned gb_g = (cgb_rgb15 >> 5) & 0x1FU;
    unsigned gb_b = (cgb_rgb15 >> 10) & 0x1FU;

    // Matt Currie's formula
    if (m_colors_hint == gb_colors_hint::cgb_acid2)
    {
        m_colors[color_index] = pixel((gb_r << 3) | (gb_r >> 2),
                                      (gb_g << 3) | (gb_g >> 2),
                                      (gb_b << 3) | (gb_b >> 2));
        return;
    }

    // formula copied from gambatte (video.cpp)
    if (m_colors_hint == gb_colors_hint::cgb_gambatte)
    {
        m_colors[color_index] = pixel((gb_r * 13 + gb_g * 2 + gb_b) >> 1,
                                      (gb_g * 3 + gb_b) << 1,
                                      (gb_r * 3 + gb_g * 2 + gb_b * 11) >> 1);
        return;
    }

    // natural CGB color
    m_colors[color_index] = lookup_cgb_color(cgb_rgb15);
}
