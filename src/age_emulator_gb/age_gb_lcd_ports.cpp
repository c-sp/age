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

#include "age_gb_lcd.hpp"

#define CLOG(log) AGE_GB_CLOG(AGE_GB_CLOG_LCD)(log)



namespace
{

constexpr unsigned palette_bgp = 0x00;
constexpr unsigned palette_obp0 = 0x20;
constexpr unsigned palette_obp1 = 0x30;

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



age::uint8_t age::gb_lcd::read_lcdc() const {
    CLOG("read LCDC " << AGE_LOG_HEX8(m_lcdc));
    return m_lcdc;
}

age::uint8_t age::gb_lcd::read_stat() {
    CLOG("read STAT " << AGE_LOG_HEX8(m_stat));
    return m_stat;
}

age::uint8_t age::gb_lcd::read_scy() const {
    CLOG("read SCY " << AGE_LOG_HEX8(m_renderer.m_scy));
    return m_renderer.m_scy;
}

age::uint8_t age::gb_lcd::read_scx() const {
    CLOG("read SCX " << AGE_LOG_HEX8(m_renderer.m_scx));
    return m_renderer.m_scx;
}

age::uint8_t age::gb_lcd::read_ly() {
    update_state();
    uint8_t ly = m_scanline.current_ly() & 0xFF;
    CLOG("read LY " << AGE_LOG_HEX8(ly));
    return ly;
}

age::uint8_t age::gb_lcd::read_lyc() const {
    CLOG("read LYC " << AGE_LOG_HEX8(m_lyc));
    return m_lyc;
}

age::uint8_t age::gb_lcd::read_bgp() const
{
    CLOG("read BGP " << AGE_LOG_HEX8(m_bgp));
    return m_bgp;
}

age::uint8_t age::gb_lcd::read_obp0() const
{
    CLOG("read OBP0 " << AGE_LOG_HEX8(m_obp0));
    return m_obp0;
}

age::uint8_t age::gb_lcd::read_obp1() const
{
    CLOG("read OBP1 " << AGE_LOG_HEX8(m_obp1));
    return m_obp1;
}

age::uint8_t age::gb_lcd::read_wx() const
{
    CLOG("read WX " << AGE_LOG_HEX8(m_renderer.m_wx));
    return m_renderer.m_wx;
}

age::uint8_t age::gb_lcd::read_wy() const
{
    CLOG("read WY " << AGE_LOG_HEX8(m_renderer.m_wy));
    return m_renderer.m_wy;
}

age::uint8_t age::gb_lcd::read_bcps() const
{
    AGE_ASSERT(m_device.is_cgb());
    CLOG("read BCPS " << AGE_LOG_HEX8(m_bcps));
    return m_bcps;
}

age::uint8_t age::gb_lcd::read_bcpd() const
{
    AGE_ASSERT(m_device.is_cgb());
    uint8_t result = m_cpd[m_bcps & 0x3F];
    CLOG("read BCPD " << AGE_LOG_HEX8(result));
    return result;
}

age::uint8_t age::gb_lcd::read_ocps() const
{
    AGE_ASSERT(m_device.is_cgb());
    CLOG("read OCPS " << AGE_LOG_HEX8(m_ocps));
    return m_ocps;
}

age::uint8_t age::gb_lcd::read_ocpd() const
{
    AGE_ASSERT(m_device.is_cgb());
    uint8_t result = m_cpd[m_ocps & 0x3F];
    CLOG("read OCPD " << AGE_LOG_HEX8(result));
    return result;
}



void age::gb_lcd::write_lcdc(uint8_t value)
{
    CLOG("write LCDC " << AGE_LOG_HEX8(value));
    update_state();

    int diff = m_lcdc ^ value;
    m_lcdc = value;
    m_renderer.set_lcdc(value);

    if (!(diff & gb_lcdc_enable))
    {
        return;
    }

    // LCD switched on/off
    if (m_lcdc & gb_lcdc_enable)
    {
        m_scanline.lcd_on();
        m_renderer.new_frame();
    }
    else
    {
        m_scanline.lcd_off();
    }
}



void age::gb_lcd::write_stat(uint8_t value)
{
    CLOG("write STAT " << AGE_LOG_HEX8(value));
    update_state();
    m_stat = value & ~gb_stat_modes;
}



void age::gb_lcd::write_scy(uint8_t value)
{
    CLOG("write SCY " << AGE_LOG_HEX8(value));
    update_state();
    m_renderer.m_scy = value;
}

void age::gb_lcd::write_scx(uint8_t value)
{
    CLOG("write SCX " << AGE_LOG_HEX8(value));
    update_state();
    m_renderer.m_scx = value;
}



void age::gb_lcd::write_lyc(uint8_t value)
{
    CLOG("write LYC " << AGE_LOG_HEX8(value));
    update_state();
    m_lyc = value;
}



void age::gb_lcd::write_bgp(uint8_t value)
{
    CLOG("write LYC " << AGE_LOG_HEX8(value));
    update_state();
    m_bgp = value;
    m_renderer.create_dmg_palette(palette_bgp, m_bgp);
}

void age::gb_lcd::write_obp0(uint8_t value)
{
    CLOG("write OBP0 " << AGE_LOG_HEX8(value));
    update_state();
    m_obp0 = value;
    m_renderer.create_dmg_palette(palette_obp0, m_obp0);
}

void age::gb_lcd::write_obp1(uint8_t value)
{
    CLOG("write OBP1 " << AGE_LOG_HEX8(value));
    update_state();
    m_obp1 = value;
    m_renderer.create_dmg_palette(palette_obp1, m_obp1);
}



void age::gb_lcd::write_wy(uint8_t value)
{
    CLOG("write WY " << AGE_LOG_HEX8(value));
    update_state();
    m_renderer.m_wy = value;
}

void age::gb_lcd::write_wx(uint8_t value)
{
    CLOG("write WX " << AGE_LOG_HEX8(value));
    update_state();
    m_renderer.m_wx = value;
}



void age::gb_lcd::write_bcps(uint8_t value)
{
    AGE_ASSERT(m_device.is_cgb());
    CLOG("write BCPS " << AGE_LOG_HEX8(value));
    m_bcps = value | 0x40;
}

void age::gb_lcd::write_bcpd(uint8_t value)
{
    AGE_ASSERT(m_device.is_cgb());
    CLOG("write BCPD " << AGE_LOG_HEX8(value));
    update_state();

    m_cpd[m_bcps & 0x3F] = value;
    update_color((m_bcps & 0x3F) / 2);
    m_bcps = increment_cps(m_bcps);
}

void age::gb_lcd::write_ocps(uint8_t value)
{
    AGE_ASSERT(m_device.is_cgb());
    CLOG("write OCPS " << AGE_LOG_HEX8(value));
    m_ocps = value | 0x40;
}

void age::gb_lcd::write_ocpd(uint8_t value)
{
    AGE_ASSERT(m_device.is_cgb());
    CLOG("write OCPD " << AGE_LOG_HEX8(value));
    update_state();

    m_cpd[0x40 + (m_ocps & 0x3F)] = value;
    update_color(0x20 + (m_ocps & 0x3F) / 2);
    m_ocps = increment_cps(m_ocps);
}
