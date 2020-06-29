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



age::uint8_t age::gb_lcd::read_lcdc() const {
    AGE_GB_CLOG_LCD("read LCDC = " << AGE_LOG_HEX8(m_renderer.get_lcdc()));
    return m_renderer.get_lcdc();
}

age::uint8_t age::gb_lcd::read_stat() {
    AGE_GB_CLOG_LCD("read STAT = " << AGE_LOG_HEX8(m_stat));
    return m_stat;
}

age::uint8_t age::gb_lcd::read_scy() const {
    AGE_GB_CLOG_LCD("read SCY = " << AGE_LOG_HEX8(m_renderer.m_scy));
    return m_renderer.m_scy;
}

age::uint8_t age::gb_lcd::read_scx() const {
    AGE_GB_CLOG_LCD("read SCX = " << AGE_LOG_HEX8(m_renderer.m_scx));
    return m_renderer.m_scx;
}

age::uint8_t age::gb_lcd::read_ly() {
    update_state();
    uint8_t ly = m_scanline.current_ly() & 0xFF;
    AGE_GB_CLOG_LCD("read LY = " << AGE_LOG_HEX8(ly));
    return ly;
}

age::uint8_t age::gb_lcd::read_lyc() const {
    AGE_GB_CLOG_LCD("read LYC = " << AGE_LOG_HEX8(m_lyc));
    return m_lyc;
}

age::uint8_t age::gb_lcd::read_bgp() const
{
    AGE_GB_CLOG_LCD("read BGP = " << AGE_LOG_HEX8(m_bgp));
    return m_bgp;
}

age::uint8_t age::gb_lcd::read_obp0() const
{
    AGE_GB_CLOG_LCD("read OBP0 = " << AGE_LOG_HEX8(m_obp0));
    return m_obp0;
}

age::uint8_t age::gb_lcd::read_obp1() const
{
    AGE_GB_CLOG_LCD("read OBP1 = " << AGE_LOG_HEX8(m_obp1));
    return m_obp1;
}

age::uint8_t age::gb_lcd::read_wx() const
{
    AGE_GB_CLOG_LCD("read WX = " << AGE_LOG_HEX8(m_renderer.m_wx));
    return m_renderer.m_wx;
}

age::uint8_t age::gb_lcd::read_wy() const
{
    AGE_GB_CLOG_LCD("read WY = " << AGE_LOG_HEX8(m_renderer.m_wy));
    return m_renderer.m_wy;
}

age::uint8_t age::gb_lcd::read_bcps() const
{
    AGE_ASSERT(m_device.is_cgb());
    AGE_GB_CLOG_LCD("read BCPS = " << AGE_LOG_HEX8(m_bcps));
    return m_bcps;
}

age::uint8_t age::gb_lcd::read_bcpd() const
{
    AGE_ASSERT(m_device.is_cgb());
    uint8_t result = m_cpd[m_bcps & 0x3F];
    AGE_GB_CLOG_LCD("read BCPD = " << AGE_LOG_HEX8(result));
    return result;
}

age::uint8_t age::gb_lcd::read_ocps() const
{
    AGE_ASSERT(m_device.is_cgb());
    AGE_GB_CLOG_LCD("read OCPS = " << AGE_LOG_HEX8(m_ocps));
    return m_ocps;
}

age::uint8_t age::gb_lcd::read_ocpd() const
{
    AGE_ASSERT(m_device.is_cgb());
    uint8_t result = m_cpd[m_ocps & 0x3F];
    AGE_GB_CLOG_LCD("read OCPD = " << AGE_LOG_HEX8(result));
    return result;
}



void age::gb_lcd::write_lcdc(uint8_t value)
{
    AGE_GB_CLOG_LCD("write LCDC = " << AGE_LOG_HEX8(value));
    update_state();

    int diff = m_renderer.get_lcdc() ^ value;
    m_renderer.set_lcdc(value);

    if (!(diff & gb_lcdc_enable))
    {
        return;
    }

    // LCD switched on/off
    if (value & gb_lcdc_enable)
    {
        AGE_GB_CLOG_LCD("LCD switched on");
        m_scanline.lcd_on();
        m_renderer.new_frame();
        schedule_vblank_irq();
    }
    else
    {
        AGE_GB_CLOG_LCD("LCD switched off");

        // switch frame buffers, if the current frame is finished
        // (otherwise it would be lost because we did not reach
        // the last v-blank scanline)
        if (m_scanline.current_scanline() >= gb_screen_height)
        {
            AGE_GB_CLOG_LCD("    * switching frame buffers (scanline "
                            << m_scanline.current_scanline() << ")");
            m_renderer.new_frame();
        }

        m_events.remove_event(gb_event::lcd_interrupt);
        m_scanline.lcd_off();
        m_next_vblank_irq = gb_no_clock_cycle;
    }
}



void age::gb_lcd::write_stat(uint8_t value)
{
    AGE_GB_CLOG_LCD("write STAT = " << AGE_LOG_HEX8(value));
    update_state();
    m_stat = value & ~gb_stat_modes;
}



void age::gb_lcd::write_scy(uint8_t value)
{
    AGE_GB_CLOG_LCD("write SCY = " << AGE_LOG_HEX8(value));
    update_state();
    m_renderer.m_scy = value;
}

void age::gb_lcd::write_scx(uint8_t value)
{
    AGE_GB_CLOG_LCD("write SCX = " << AGE_LOG_HEX8(value));
    update_state();
    m_renderer.m_scx = value;
}



void age::gb_lcd::write_lyc(uint8_t value)
{
    AGE_GB_CLOG_LCD("write LYC = " << AGE_LOG_HEX8(value));
    update_state();
    m_lyc = value;
}



void age::gb_lcd::write_bgp(uint8_t value)
{
    AGE_GB_CLOG_LCD("write LYC = " << AGE_LOG_HEX8(value));
    update_state();
    m_bgp = value;
    m_renderer.create_dmg_palette(gb_palette_bgp, m_bgp);
}

void age::gb_lcd::write_obp0(uint8_t value)
{
    AGE_GB_CLOG_LCD("write OBP0 = " << AGE_LOG_HEX8(value));
    update_state();
    m_obp0 = value;
    m_renderer.create_dmg_palette(gb_palette_obp0, m_obp0);
}

void age::gb_lcd::write_obp1(uint8_t value)
{
    AGE_GB_CLOG_LCD("write OBP1 = " << AGE_LOG_HEX8(value));
    update_state();
    m_obp1 = value;
    m_renderer.create_dmg_palette(gb_palette_obp1, m_obp1);
}



void age::gb_lcd::write_wy(uint8_t value)
{
    AGE_GB_CLOG_LCD("write WY = " << AGE_LOG_HEX8(value));
    update_state();
    m_renderer.m_wy = value;
}

void age::gb_lcd::write_wx(uint8_t value)
{
    AGE_GB_CLOG_LCD("write WX = " << AGE_LOG_HEX8(value));
    update_state();
    m_renderer.m_wx = value;
}



void age::gb_lcd::write_bcps(uint8_t value)
{
    AGE_ASSERT(m_device.is_cgb());
    AGE_GB_CLOG_LCD("write BCPS = " << AGE_LOG_HEX8(value));
    m_bcps = value | 0x40;
}

void age::gb_lcd::write_bcpd(uint8_t value)
{
    AGE_ASSERT(m_device.is_cgb());
    AGE_GB_CLOG_LCD("write BCPD = " << AGE_LOG_HEX8(value));
    update_state();

    m_cpd[m_bcps & 0x3F] = value;
    update_color((m_bcps & 0x3F) / 2);
    m_bcps = increment_cps(m_bcps);
}

void age::gb_lcd::write_ocps(uint8_t value)
{
    AGE_ASSERT(m_device.is_cgb());
    AGE_GB_CLOG_LCD("write OCPS = " << AGE_LOG_HEX8(value));
    m_ocps = value | 0x40;
}

void age::gb_lcd::write_ocpd(uint8_t value)
{
    AGE_ASSERT(m_device.is_cgb());
    AGE_GB_CLOG_LCD("write OCPD = " << AGE_LOG_HEX8(value));
    update_state();

    m_cpd[0x40 + (m_ocps & 0x3F)] = value;
    update_color(0x20 + (m_ocps & 0x3F) / 2);
    m_ocps = increment_cps(m_ocps);
}
