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

#include "age_gb_lcd.hpp"



age::uint8_t age::gb_lcd::read_oam(int offset)
{
    gb_current_line line{};
    if (!is_oam_readable(line))
    {
        m_clock.log(gb_log_category::lc_lcd_oam) << "read OAM[" << log_hex8(offset) << "] == " << log_hex8(0xFF)
                                                 << " - OAM not readable" << log_line_clks(m_line);
        return 0xFF;
    }
    auto result = m_sprites.read_oam(offset);

    m_clock.log(gb_log_category::lc_lcd_oam) << "read OAM[" << log_hex8(offset) << "] == " << log_hex8(result)
                                             << " - OAM readable" << log_line_clks(m_line);
    return result;
}

bool age::gb_lcd::is_oam_readable(gb_current_line& line)
{
    if (!m_line.lcd_is_on())
    {
        return true;
    }
    line = calculate_line();

    // timing for lines 1-153 on edge of mode 2
    int clk_adjust = m_device.is_cgb_e_device() && m_clock.is_double_speed() ? 1 : 0;
    if (line.m_line_clks >= gb_clock_cycles_per_lcd_line - 1 - clk_adjust)
    {
        return (line.m_line >= gb_screen_height - 1) && (line.m_line < gb_lcd_line_count - 1);
    }
    clk_adjust = m_device.is_cgb_e_device() && !m_clock.is_double_speed() ? 1 : 0;
    // different timing for frame 0 line 0
    if (m_line.is_first_frame() && !line.m_line)
    {
        return (line.m_line_clks < 82)
               || (line.m_line_clks >= (82 + 172 + (m_render.m_scx & 7) + clk_adjust));
    }
    // timing for lines 1-153
    return (line.m_line >= gb_screen_height)
           || (line.m_line_clks >= (80 + 172 + (m_render.m_scx & 7) + clk_adjust));
}



void age::gb_lcd::write_oam(int offset, uint8_t value)
{
    gb_current_line line{};
    if (!is_oam_writable(line))
    {
        m_clock.log(gb_log_category::lc_lcd_oam) << "write OAM[" << log_hex8(offset) << "] = " << log_hex8(value)
                                                 << " ignored - OAM not writable" << log_line_clks(m_line);
        return;
    }
    m_clock.log(gb_log_category::lc_lcd_oam) << "write OAM[" << log_hex8(offset) << "] = " << log_hex8(value)
                                             << " - OAM writable" << log_line_clks(m_line);
    write_oam_dma(offset, value);
}

bool age::gb_lcd::is_oam_writable(gb_current_line& line)
{
    if (!m_line.lcd_is_on())
    {
        return true;
    }
    line = calculate_line();

    // timing for lines 1-153 on edge of mode 2
    int clk_adjust = m_clock.is_double_speed() ? 1 : 0;
    if (m_device.is_cgb_device() && (line.m_line_clks >= gb_clock_cycles_per_lcd_line - 1 - clk_adjust))
    {
        return (line.m_line >= gb_screen_height - 1) && (line.m_line < gb_lcd_line_count - 1);
    }
    // different timing for frame 0 line 0
    if (m_line.is_first_frame() && !line.m_line)
    {
        if (m_device.is_cgb_device())
        {
            return (line.m_line_clks < 80)
                   || (line.m_line_clks >= (82 + 172 + (m_render.m_scx & 7)));
        }
        // This is odd (write-timing-oam-dmgC.asm) ...
        return (line.m_line_clks < 84)
               || (line.m_line_clks >= (84 + 172 + (m_render.m_scx >= 6 ? 4 : 0)));
    }
    // timing for lines 1-153
    return (line.m_line >= gb_screen_height)
           || (line.m_line_clks >= (80 + 172 + (m_render.m_scx & 7)));
}



void age::gb_lcd::write_oam_dma(int offset, uint8_t value)
{
    update_state(); // make sure everything is rendered up to now
    m_sprites.write_oam(offset, value);
}
