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



//---------------------------------------------------------
//
//   lcdc
//
//---------------------------------------------------------

age::uint8_t age::gb_lcd::read_lcdc() const
{
    auto result = m_render.get_lcdc();
    log_reg() << "read LCDC == " << log_hex8(result);
    return result;
}

void age::gb_lcd::write_lcdc(uint8_t value)
{
    update_state();
    auto msg = log_reg();
    msg << "write LCDC = " << log_hex8(value);

    int diff = m_render.get_lcdc() ^ value;
    m_render.set_lcdc(value);

    if (!(diff & gb_lcdc_enable))
    {
        msg << "\n    * LCD already" << ((value & gb_lcdc_enable) ? "on" : "off");
        return;
    }

    // LCD switched on
    if (value & gb_lcdc_enable)
    {
        msg << "\n    * LCD switched on";

        m_scanline.lcd_on();
        m_lcd_irqs.lcd_on(m_render.m_scx);
        m_render.new_frame();
    }

    // LCD switched off
    else
    {
        msg << "\n    * LCD switched off";

        int scanline      = -1;
        int scanline_clks = -1;
        calculate_scanline(scanline, scanline_clks);

        // switch frame buffers, if the current frame is finished
        // (otherwise it would be lost because we did not reach
        // the last v-blank scanline)
        if (scanline >= gb_screen_height)
        {
            m_render.new_frame();
        }

        // The STAT LY match flag is retained when switching off the LCD.
        // Mooneye GB tests:
        //      acceptance/ppu/stat_lyc_onoff
        m_retained_ly_match = get_stat_ly_match(scanline, scanline_clks);

        msg << "\n    * retained LY match " << log_hex8(m_retained_ly_match)
            << " for LYC " << log_hex8(m_scanline.m_lyc)
            << " (scanline " << scanline << ")";

        m_lcd_irqs.lcd_off();
        m_scanline.lcd_off();
    }
}



//---------------------------------------------------------
//
//   stat
//
//---------------------------------------------------------

age::uint8_t age::gb_lcd::read_stat()
{
    uint8_t result = m_lcd_irqs.read_stat();

    // LCD off: return retained LY match flag
    if (!m_scanline.lcd_is_on())
    {
        AGE_ASSERT((result & gb_stat_modes) == 0)
        return result | m_retained_ly_match;
    }

    // LCD on: calculate current mode and LY match flag
    int scanline      = -1;
    int scanline_clks = -1;
    calculate_scanline(scanline, scanline_clks);

    result |= get_stat_ly_match(scanline, scanline_clks)
              | get_stat_mode(scanline, scanline_clks, m_render.m_scx);

    log_reg() << "read STAT == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_lcd::get_stat_mode(int scanline,
                                        int scanline_clks,
                                        int scx) const
{
    AGE_ASSERT(scanline < gb_scanline_count)

    // vblank
    if (scanline >= gb_screen_height)
    {
        // mode 0 is signalled for v-blank's last machine cycle
        // (not for double speed though)
        if (m_clock.is_double_speed() || (scanline < 153))
        {
            return 1;
        }
        return (scanline_clks >= gb_clock_cycles_per_scanline - 4) ? 0 : 1;
    }

    // first scanline after restarting the LCD:
    // 80 cycles mode 0 instead of mode 2.
    // Note that this scanline also is 3 T4-cycles shorter than usual
    // which we handle by offsetting the frame
    // (see m_scanline.lcd_on).
    if (m_scanline.is_first_frame() && !scanline && (scanline_clks < 80 - gb_lcd_m_cycle_align))
    {
        return 0;
    }

    // mode 2
    if (scanline_clks < 80)
    {
        return 2;
    }

    // mode 3 and mode 0
    //! \todo test rom analyses for double-speed-delay (using m2int_m3stat?)
    //! \todo too simple: mode 3 timing also depends on sprites & window
    int m3_end = 80 + 172 + (scx & 7) + m_clock.is_double_speed();
    return (scanline_clks < m3_end) ? 3 : 0;
}

age::uint8_t age::gb_lcd::get_stat_ly_match(int scanline, int scanline_clks) const
{
    int match_scanline = scanline;

    // special timing for scanline 153
    if (scanline == 153)
    {
        //! \todo need more timing details
        if (scanline_clks > 4)
        {
            match_scanline = 0;
        }
    }

    // "regular" timing
    else
    {
        // when not running at double speed LY-match is cleared early
        int clks_next_scanline = gb_clock_cycles_per_scanline - scanline_clks;
        if (clks_next_scanline <= (m_clock.is_double_speed() ? 0 : 2))
        {
            match_scanline = -1;
        }
    }

    return m_scanline.m_lyc == match_scanline ? gb_stat_ly_match : 0;
}



void age::gb_lcd::write_stat(uint8_t value)
{
    update_state();
    log_reg() << "write STAT = " << log_hex8(value);
    m_lcd_irqs.write_stat(value, m_render.m_scx);
}



//---------------------------------------------------------
//
//   ly
//
//---------------------------------------------------------

void age::gb_lcd::calculate_scanline(int& scanline, int& scanline_clks)
{
    AGE_ASSERT(m_scanline.lcd_is_on())
    m_scanline.current_scanline(scanline, scanline_clks);
    if (scanline >= gb_scanline_count)
    {
        update_state();
        m_scanline.current_scanline(scanline, scanline_clks);
    }
    AGE_ASSERT(scanline < gb_scanline_count)
}

age::uint8_t age::gb_lcd::read_ly()
{
    if (!m_scanline.lcd_is_on())
    {
        return 0;
    }

    int scanline      = -1;
    int scanline_clks = -1;
    calculate_scanline(scanline, scanline_clks);

    // LY = 153 only for 2-3 T4-cycles
    if ((scanline >= 153) && (scanline_clks > 2 + m_clock.is_double_speed()))
    {
        return 0;
    }

    // Ly is incremented 2 clock cycles earlier than the respective scanline
    int ly = scanline + (scanline_clks >= gb_clock_cycles_per_scanline - 2);

    AGE_GB_CLOG_LCD_PORTS_LY("read LY = " << AGE_LOG_HEX8(ly & 0xFF))
    return ly & 0xFF;
}



//---------------------------------------------------------
//
//   other registers
//
//---------------------------------------------------------

age::uint8_t age::gb_lcd::read_scy() const
{
    log_reg() << "read SCY == " << log_hex8(m_render.m_scy);
    return m_render.m_scy;
}

age::uint8_t age::gb_lcd::read_scx() const
{
    log_reg() << "read SCX == " << log_hex8(m_render.m_scx);
    return m_render.m_scx;
}

age::uint8_t age::gb_lcd::read_lyc() const
{
    log_reg() << "read LYC == " << log_hex8(m_scanline.m_lyc);
    return m_scanline.m_lyc;
}

age::uint8_t age::gb_lcd::read_bgp() const
{
    auto result = m_palettes.read_bgp();
    log_reg() << "read BGP == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_lcd::read_obp0() const
{
    auto result = m_palettes.read_obp0();
    log_reg() << "read OBP0 == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_lcd::read_obp1() const
{
    auto result = m_palettes.read_obp1();
    log_reg() << "read OBP1 == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_lcd::read_wx() const
{
    log_reg() << "read WX == " << log_hex8(m_render.m_wx);
    return m_render.m_wx;
}

age::uint8_t age::gb_lcd::read_wy() const
{
    log_reg() << "read WY == " << log_hex8(m_render.m_wy);
    return m_render.m_wy;
}

age::uint8_t age::gb_lcd::read_bcps() const
{
    auto result = m_palettes.read_bcps();
    log_reg() << "read BCPS == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_lcd::read_bcpd() const
{
    AGE_ASSERT(m_device.is_cgb())
    auto result = m_palettes.read_bcpd();
    log_reg() << "read BCPD == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_lcd::read_ocps() const
{
    AGE_ASSERT(m_device.is_cgb())
    auto result = m_palettes.read_ocps();
    log_reg() << "read OCPS == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_lcd::read_ocpd() const
{
    AGE_ASSERT(m_device.is_cgb())
    auto result = m_palettes.read_ocpd();
    log_reg() << "read OCPD == " << log_hex8(result);
    return result;
}



void age::gb_lcd::write_scy(uint8_t value)
{
    update_state();
    log_reg() << "write SCY = " << log_hex8(value);
    m_render.m_scy = value;
}

void age::gb_lcd::write_scx(uint8_t value)
{
    update_state();
    log_reg() << "write SCX = " << log_hex8(value);
    m_render.m_scx = value;
}



void age::gb_lcd::write_lyc(uint8_t value)
{
    auto msg = log_reg();
    msg << "write LYC = " << log_hex8(value);

    if (m_scanline.m_lyc == value)
    {
        msg << "\n    * same value as before";
        return;
    }
    update_state();
    m_scanline.m_lyc = value;
    m_lcd_irqs.lyc_update();
}



void age::gb_lcd::write_bgp(uint8_t value)
{
    update_state();
    log_reg() << "write BGP = " << log_hex8(value);
    m_palettes.write_bgp(value);
}

void age::gb_lcd::write_obp0(uint8_t value)
{
    update_state();
    log_reg() << "write OBP0 = " << log_hex8(value);
    m_palettes.write_obp0(value);
}

void age::gb_lcd::write_obp1(uint8_t value)
{
    update_state();
    log_reg() << "write OBP1 = " << log_hex8(value);
    m_palettes.write_obp1(value);
}



void age::gb_lcd::write_wy(uint8_t value)
{
    update_state();
    log_reg() << "write WY = " << log_hex8(value);
    m_render.m_wy = value;
}

void age::gb_lcd::write_wx(uint8_t value)
{
    update_state();
    log_reg() << "write WX = " << log_hex8(value);
    m_render.m_wx = value;
}



void age::gb_lcd::write_bcps(uint8_t value)
{
    log_reg() << "write BCPS = " << log_hex8(value);
    m_palettes.write_bcps(value);
}

void age::gb_lcd::write_bcpd(uint8_t value)
{
    update_state();
    log_reg() << "write BCPD = " << log_hex8(value);
    m_palettes.write_bcpd(value);
}

void age::gb_lcd::write_ocps(uint8_t value)
{
    log_reg() << "write OCPS = " << log_hex8(value);
    m_palettes.write_ocps(value);
}

void age::gb_lcd::write_ocpd(uint8_t value)
{
    update_state();
    log_reg() << "write OCPD = " << log_hex8(value);
    m_palettes.write_ocpd(value);
}
