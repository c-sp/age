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



age::uint8_t age::gb_lcd::read_lcdc() const {
    AGE_GB_CLOG_LCD_PORTS("read LCDC = " << AGE_LOG_HEX8(m_render.get_lcdc()));
    return m_render.get_lcdc();
}

age::uint8_t age::gb_lcd::read_stat() {
    update_state();
    uint8_t result = m_stat | stat_lyc() | m_scanline.stat_mode();
    AGE_GB_CLOG_LCD_PORTS("read STAT = " << AGE_LOG_HEX8(result));
    return result;
}

age::uint8_t age::gb_lcd::read_scy() const {
    AGE_GB_CLOG_LCD_PORTS("read SCY = " << AGE_LOG_HEX8(m_render.m_scy));
    return m_render.m_scy;
}

age::uint8_t age::gb_lcd::read_scx() const {
    AGE_GB_CLOG_LCD_PORTS("read SCX = " << AGE_LOG_HEX8(m_render.m_scx));
    return m_render.m_scx;
}

age::uint8_t age::gb_lcd::read_ly() {
    update_state();
    auto ly = m_scanline.current_ly();
    AGE_GB_CLOG_LCD_PORTS_LY("read LY = " << AGE_LOG_HEX8(ly));
    return ly & 0xFF;
}

age::uint8_t age::gb_lcd::read_lyc() const {
    AGE_GB_CLOG_LCD_PORTS("read LYC = " << AGE_LOG_HEX8(m_lyc));
    return m_lyc;
}

age::uint8_t age::gb_lcd::read_bgp() const
{
    auto result = m_palettes.read_bgp();
    AGE_GB_CLOG_LCD_PORTS("read BGP = " << AGE_LOG_HEX8(result));
    return result;
}

age::uint8_t age::gb_lcd::read_obp0() const
{
    auto result = m_palettes.read_obp0();
    AGE_GB_CLOG_LCD_PORTS("read OBP0 = " << AGE_LOG_HEX8(result));
    return result;
}

age::uint8_t age::gb_lcd::read_obp1() const
{
    auto result = m_palettes.read_obp1();
    AGE_GB_CLOG_LCD_PORTS("read OBP1 = " << AGE_LOG_HEX8(result));
    return result;
}

age::uint8_t age::gb_lcd::read_wx() const
{
    AGE_GB_CLOG_LCD_PORTS("read WX = " << AGE_LOG_HEX8(m_render.m_wx));
    return m_render.m_wx;
}

age::uint8_t age::gb_lcd::read_wy() const
{
    AGE_GB_CLOG_LCD_PORTS("read WY = " << AGE_LOG_HEX8(m_render.m_wy));
    return m_render.m_wy;
}

age::uint8_t age::gb_lcd::read_bcps() const
{
    auto result = m_palettes.read_bcps();
    AGE_GB_CLOG_LCD_PORTS("read BCPS = " << AGE_LOG_HEX8(result));
    return result;
}

age::uint8_t age::gb_lcd::read_bcpd() const
{
    AGE_ASSERT(m_device.is_cgb());
    auto result = m_palettes.read_bcpd();
    AGE_GB_CLOG_LCD_PORTS("read BCPD = " << AGE_LOG_HEX8(result));
    return result;
}

age::uint8_t age::gb_lcd::read_ocps() const
{
    AGE_ASSERT(m_device.is_cgb());
    auto result = m_palettes.read_ocps();
    AGE_GB_CLOG_LCD_PORTS("read OCPS = " << AGE_LOG_HEX8(result));
    return result;
}

age::uint8_t age::gb_lcd::read_ocpd() const
{
    AGE_ASSERT(m_device.is_cgb());
    auto result = m_palettes.read_ocpd();
    AGE_GB_CLOG_LCD_PORTS("read OCPD = " << AGE_LOG_HEX8(result));
    return result;
}



void age::gb_lcd::write_lcdc(uint8_t value)
{
    AGE_GB_CLOG_LCD_PORTS("write LCDC = " << AGE_LOG_HEX8(value));
    update_state();

    int diff = m_render.get_lcdc() ^ value;
    m_render.set_lcdc(value);

    if (!(diff & gb_lcdc_enable))
    {
        return;
    }

    // LCD switched on
    if (value & gb_lcdc_enable)
    {
        AGE_GB_CLOG_LCD_RENDER("LCD switched on");

        // Clear STAT LY match flag as we calculate it dynamically
        // when the LCD is switched on
        // (the value was saved on LCD off).
        m_stat &= ~gb_stat_ly_match;

        m_scanline.lcd_on();
        m_render.new_frame();
        schedule_vblank_irq();
    }

    // LCD switched off
    else
    {
        AGE_GB_CLOG_LCD_RENDER("LCD switched off");

        // The STAT LY match flag keeps it's current value.
        // Gambatte tests:
        //      enable_display/disable_display_regs_1_dmg08_cgb04c_out66e46666009266666666
        m_stat |= stat_lyc();

        // switch frame buffers, if the current frame is finished
        // (otherwise it would be lost because we did not reach
        // the last v-blank scanline)
        if (m_scanline.current_scanline() >= gb_screen_height)
        {
            AGE_GB_CLOG_LCD_RENDER("    * switching frame buffers (scanline "
                                   << m_scanline.current_scanline() << ")");
            m_render.new_frame();
        }

        m_events.remove_event(gb_event::lcd_interrupt);
        m_scanline.lcd_off();
        m_next_vblank_irq = gb_no_clock_cycle;
    }
}



void age::gb_lcd::write_stat(uint8_t value)
{
    AGE_GB_CLOG_LCD_PORTS("write STAT = " << AGE_LOG_HEX8(value));
    update_state();
    m_stat = (value & ~(gb_stat_modes | gb_stat_ly_match)) | 0x80;
}



void age::gb_lcd::write_scy(uint8_t value)
{
    AGE_GB_CLOG_LCD_PORTS("write SCY = " << AGE_LOG_HEX8(value));
    update_state();
    m_render.m_scy = value;
}

void age::gb_lcd::write_scx(uint8_t value)
{
    AGE_GB_CLOG_LCD_PORTS("write SCX = " << AGE_LOG_HEX8(value));
    update_state();
    m_render.m_scx = value;
}



void age::gb_lcd::write_lyc(uint8_t value)
{
    AGE_GB_CLOG_LCD_PORTS("write LYC = " << AGE_LOG_HEX8(value));
    update_state();
    m_lyc = value;
}



void age::gb_lcd::write_bgp(uint8_t value)
{
    AGE_GB_CLOG_LCD_PORTS("write LYC = " << AGE_LOG_HEX8(value));
    update_state();
    m_palettes.write_bgp(value);
}

void age::gb_lcd::write_obp0(uint8_t value)
{
    AGE_GB_CLOG_LCD_PORTS("write OBP0 = " << AGE_LOG_HEX8(value));
    update_state();
    m_palettes.write_obp0(value);
}

void age::gb_lcd::write_obp1(uint8_t value)
{
    AGE_GB_CLOG_LCD_PORTS("write OBP1 = " << AGE_LOG_HEX8(value));
    update_state();
    m_palettes.write_obp1(value);
}



void age::gb_lcd::write_wy(uint8_t value)
{
    AGE_GB_CLOG_LCD_PORTS("write WY = " << AGE_LOG_HEX8(value));
    update_state();
    m_render.m_wy = value;
}

void age::gb_lcd::write_wx(uint8_t value)
{
    AGE_GB_CLOG_LCD_PORTS("write WX = " << AGE_LOG_HEX8(value));
    update_state();
    m_render.m_wx = value;
}



void age::gb_lcd::write_bcps(uint8_t value)
{
    AGE_GB_CLOG_LCD_PORTS("write BCPS = " << AGE_LOG_HEX8(value));
    m_palettes.write_bcps(value);
}

void age::gb_lcd::write_bcpd(uint8_t value)
{
    AGE_GB_CLOG_LCD_PORTS("write BCPD = " << AGE_LOG_HEX8(value));
    update_state();
    m_palettes.write_bcpd(value);
}

void age::gb_lcd::write_ocps(uint8_t value)
{
    AGE_GB_CLOG_LCD_PORTS("write OCPS = " << AGE_LOG_HEX8(value));
    m_palettes.write_ocps(value);
}

void age::gb_lcd::write_ocpd(uint8_t value)
{
    AGE_GB_CLOG_LCD_PORTS("write OCPD = " << AGE_LOG_HEX8(value));
    update_state();
    m_palettes.write_ocpd(value);
}
