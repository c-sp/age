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
    auto msg = log_reg();
    msg << "write LCDC = " << log_hex8(value) << log_line_clks(m_line);

    uint8_t diff = m_render.get_lcdc() ^ value;

    // LCD remains on/off
    if (!(diff & gb_lcdc_enable))
    {
        msg << "\n    * LCD already " << ((value & gb_lcdc_enable) ? "on" : "off");

        // LCD remains off
        if (!m_line.lcd_is_on())
        {
            m_render.set_lcdc(value);
            return;
        }

        // LCD remains on -> update state before updating LCDC
        update_state();
        auto line = m_line.current_line();

        // tile data bit changed
        if (m_device.is_cgb_device() && (diff & gb_lcdc_bg_win_data))
        {
            m_render.set_clks_tile_data_change(line);
            msg << "\n    * potential CGB glitch: tile data bit switched (LCD on)";
        }

        // window flag switched
        if (diff & gb_lcdc_win_enable)
        {
            if (value & gb_lcdc_win_enable)
            {
                // update LCDC but delay updating the window-enable-flag
                m_render.set_lcdc((m_render.get_lcdc() & gb_lcdc_win_enable) | (value & ~gb_lcdc_win_enable));
                update_state(1);
            }
            else
            {
                m_render.window_switched_off(line);
            }
        }

        // update LCDC
        m_render.set_lcdc(value);

        // check for late WY match on this line
        m_render.check_for_wy_match(line, m_render.m_wy);

        return;
    }

    // LCD switched on
    if (value & gb_lcdc_enable)
    {
        msg << "\n    * LCD switched on";
        m_render.set_lcdc(value);
        m_line.lcd_on();
        m_lcd_irqs.lcd_on(m_render.m_scx);
        m_render.new_frame();
    }

    // LCD switched off
    else
    {
        msg << "\n    * LCD switched off";

        auto line = m_line.current_line();

        // switch frame buffers, if the current frame is finished
        // (otherwise it would be lost just because we did not reach
        // the last v-blank line)
        if (line.m_line >= gb_screen_height)
        {
            //! \todo blank frame
            update_state();
            m_render.new_frame();
        }

        // The STAT LY match flag is retained when switching off the LCD.
        // Mooneye GB tests:
        //      acceptance/ppu/stat_lyc_onoff
        m_retained_ly_match = get_stat_ly_match(line);

        msg << "\n    * retained LY match " << log_hex8(m_retained_ly_match)
            << " for LYC " << log_hex8(m_line.m_lyc)
            << " (line " << line.m_line << ")";

        m_lcd_irqs.lcd_off();
        m_line.lcd_off();
        m_render.set_lcdc(value);
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
    if (!m_line.lcd_is_on())
    {
        AGE_ASSERT((result & gb_stat_modes) == 0)
        return result | m_retained_ly_match;
    }

    // LCD on: calculate current mode and LY match flag
    auto line = calculate_line();

    result |= get_stat_ly_match(line) | get_stat_mode(line);

    log_reg() << "read STAT == " << log_hex8(result) << log_line_clks(m_line);
    return result;
}

age::uint8_t age::gb_lcd::get_stat_mode(const gb_current_line& current_line)
{
    AGE_ASSERT(current_line.m_line < gb_lcd_line_count)

    // vblank
    if (current_line.m_line >= gb_screen_height)
    {
        // mode 0 is signalled for v-blank's last machine cycle
        // (not for double speed and not on CGB-E though)
        if (m_clock.is_double_speed() || m_device.is_cgb_e_device() || (current_line.m_line < 153))
        {
            return 1;
        }
        return (current_line.m_line_clks >= gb_clock_cycles_per_lcd_line - 4) ? 0 : 1;
    }

    // mode 2 (mode 0 for the first line after the LCD was switched on)
    bool is_line_zero = m_line.is_first_frame() && (current_line.m_line == 0);
    if (current_line.m_line_clks < (is_line_zero ? 82 : 80))
    {
        return is_line_zero ? 0 : 2;
    }

    // mode 3 or mode 0
#ifdef AGE_COMPILE_FIFO_RENDERER
    update_state(1);
    return m_render.stat_mode0() ? 0 : 3;

#else
    // first line after restarting the LCD:
    // mode 0 instead of mode 2.
    int m3_end = 80 + 172 + (m_render.m_scx & 7);
    return (current_line.m_line_clks >= (is_line_zero ? m3_end + 2 : m3_end)) ? 0 : 3;
#endif
}

age::uint8_t age::gb_lcd::get_stat_ly_match(const gb_current_line& current_line) const
{
    int match_line = current_line.m_line;

    // special timing for line 153
    if (current_line.m_line == 153)
    {
        //! \todo need more timing details
        if (current_line.m_line_clks > 4)
        {
            match_line = 0;
        }
    }

    // "regular" timing
    else
    {
        // when not running at double speed LY-match is cleared early
        int clks_next_line = gb_clock_cycles_per_lcd_line - current_line.m_line_clks;
        if (clks_next_line <= (m_clock.is_double_speed() ? 0 : 2))
        {
            match_line = -1;
        }
    }

    return m_line.m_lyc == match_line ? gb_stat_ly_match : 0;
}



void age::gb_lcd::write_stat(uint8_t value)
{
    update_state();
    log_reg() << "write STAT = " << log_hex8(value) << log_line_clks(m_line);
    m_lcd_irqs.write_stat(value, m_render.m_scx);
}



//---------------------------------------------------------
//
//   ly
//
//---------------------------------------------------------

age::gb_current_line age::gb_lcd::calculate_line()
{
    AGE_ASSERT(m_line.lcd_is_on())
    auto line = m_line.current_line();
    if (line.m_line >= gb_lcd_line_count)
    {
        update_frame();
        line = m_line.current_line();
    }
    AGE_ASSERT(line.m_line < gb_lcd_line_count)
    return line;
}

age::uint8_t age::gb_lcd::read_ly()
{
    if (!m_line.lcd_is_on())
    {
        return 0;
    }

    auto line = calculate_line();

    // LY = 153 only for 2-3 T4-cycles
    if ((line.m_line >= 153) && (line.m_line_clks > 2 + (m_clock.is_double_speed() || m_device.is_cgb_e_device() ? 1 : 0)))
    {
        log_reg() << "read LY == 0" << log_line_clks(m_line);
        return 0;
    }

    // LY is incremented 2 clock cycles early
    int ly = line.m_line + ((line.m_line_clks >= gb_clock_cycles_per_lcd_line - 2) ? 1 : 0);

    log_reg() << "read LY == " << log_hex8(ly) << log_line_clks(m_line);
    return ly;
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
    log_reg() << "read LYC == " << log_hex8(m_line.m_lyc);
    return m_line.m_lyc;
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
    AGE_ASSERT(m_device.cgb_mode())
    auto result = m_palettes.read_bcpd();
    log_reg() << "read BCPD == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_lcd::read_ocps() const
{
    AGE_ASSERT(m_device.cgb_mode())
    auto result = m_palettes.read_ocps();
    log_reg() << "read OCPS == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_lcd::read_ocpd() const
{
    AGE_ASSERT(m_device.cgb_mode())
    auto result = m_palettes.read_ocpd();
    log_reg() << "read OCPD == " << log_hex8(result);
    return result;
}



void age::gb_lcd::write_scy(uint8_t value)
{
    if (m_line.lcd_is_on())
    {
        update_state(m_device.is_cgb_device() ? 1 : 0);
    }
    log_reg() << "write SCY = " << log_hex8(value) << log_line_clks(m_line);
    m_render.m_scy = value;
}

void age::gb_lcd::write_scx(uint8_t value)
{
    update_state();
    log_reg() << "write SCX = " << log_hex8(value) << log_line_clks(m_line);
    m_render.m_scx = value;
}



void age::gb_lcd::write_lyc(uint8_t value)
{
    auto msg = log_reg();
    msg << "write LYC = " << log_hex8(value);

    if (m_line.m_lyc == value)
    {
        msg << "\n    * same value as before";
        return;
    }
    update_state();
    m_line.m_lyc = value;
    m_lcd_irqs.lyc_update();
}



void age::gb_lcd::write_bgp(uint8_t value)
{
    update_state(m_device.is_dmg_device() || m_device.is_cgb_e_device() ? -1 : 0);
    if (m_device.is_dmg_device() && m_line.lcd_is_on())
    {
        m_render.set_clks_bgp_change(m_line.current_line());
    }
    log_reg() << "write BGP = " << log_hex8(value) << log_line_clks(m_line);
    m_palettes.write_bgp(value);
}

void age::gb_lcd::write_obp0(uint8_t value)
{
    update_state();
    log_reg() << "write OBP0 = " << log_hex8(value) << log_line_clks(m_line);
    m_palettes.write_obp0(value);
}

void age::gb_lcd::write_obp1(uint8_t value)
{
    update_state();
    log_reg() << "write OBP1 = " << log_hex8(value) << log_line_clks(m_line);
    m_palettes.write_obp1(value);
}



void age::gb_lcd::write_wy(uint8_t value)
{
    if (m_line.lcd_is_on())
    {
        // handle window activation on the current line
        update_state(m_device.is_dmg_device()    ? 1
                     : m_clock.is_double_speed() ? 2
                                                 : 5);
        // make sure we don't miss any late WY match for the current line
        // or the next line (e.g. on line cycle 455 for CGB)
        auto line = m_line.current_line();
        m_render.check_for_wy_match(line, m_render.m_wy);
        m_render.check_for_wy_match(line, value);
    }

    log_reg() << "write WY = " << log_hex8(value) << log_line_clks(m_line);
    m_render.m_wy = value;
}

void age::gb_lcd::write_wx(uint8_t value)
{
    if (m_line.lcd_is_on())
    {
        update_state(m_device.is_dmg_device() ? 0 : 1);
    }
    log_reg() << "write WX = " << log_hex8(value) << log_line_clks(m_line);
    m_render.m_wx = value;
}



void age::gb_lcd::write_bcps(uint8_t value)
{
    log_reg() << "write BCPS = " << log_hex8(value) << log_line_clks(m_line);
    m_palettes.write_bcps(value);
}

void age::gb_lcd::write_bcpd(uint8_t value)
{
    update_state();
    log_reg() << "write BCPD = " << log_hex8(value) << log_line_clks(m_line);
    m_palettes.write_bcpd(value);
}

void age::gb_lcd::write_ocps(uint8_t value)
{
    log_reg() << "write OCPS = " << log_hex8(value) << log_line_clks(m_line);
    m_palettes.write_ocps(value);
}

void age::gb_lcd::write_ocpd(uint8_t value)
{
    update_state();
    log_reg() << "write OCPD = " << log_hex8(value) << log_line_clks(m_line);
    m_palettes.write_ocpd(value);
}
