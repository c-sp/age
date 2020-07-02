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

#ifndef AGE_GB_LCD_HPP
#define AGE_GB_LCD_HPP

//!
//! \file
//!

#include <age_types.hpp>
#include <gfx/age_screen_buffer.hpp>

#include "common/age_gb_clock.hpp"
#include "common/age_gb_device.hpp"
#include "common/age_gb_events.hpp"
#include "common/age_gb_interrupts.hpp"
#include "age_gb_memory.hpp"
#include "age_gb_lcd_render.hpp"



namespace age
{

constexpr int gb_clock_cycles_per_scanline = 456;
constexpr int gb_scanline_count = 154;
constexpr int gb_clock_cycles_per_frame = gb_scanline_count * gb_clock_cycles_per_scanline;

constexpr uint8_t gb_stat_irq_ly_match = 0x40;
//constexpr uint8_t gb_stat_irq_mode2 = 0x20;
constexpr uint8_t gb_stat_irq_mode1 = 0x10;
//constexpr uint8_t gb_stat_irq_mode0 = 0x08;
constexpr uint8_t gb_stat_ly_match = 0x04;
constexpr uint8_t gb_stat_modes = 0x03;



constexpr int gb_scanline_lcd_off = int_max;

class gb_lcd_scanline
{
    AGE_DISABLE_COPY(gb_lcd_scanline);
public:

    gb_lcd_scanline(const gb_clock &clock);

    int clk_frame_start() const;
    int current_scanline() const;

    uint8_t stat_flags() const;
    uint8_t current_ly() const;

    void lcd_on();
    void lcd_off();
    void fast_forward_frames();
    void set_back_clock(int clock_cycle_offset);

private:

    uint8_t stat_mode(int scanline, int scanline_clks) const;
    uint8_t stat_ly_match(int scanline, int scanline_clks) const;
    void calculate_scanline(int &scanline, int &scanline_clks) const;

    const gb_clock &m_clock;
    int m_clk_frame_start = gb_no_clock_cycle;
    int m_first_frame = false;
    uint8_t m_retained_ly_match = 0;

public:

    uint8_t m_lyc = 0;
};



class gb_lcd_interrupts
{
    AGE_DISABLE_COPY(gb_lcd_interrupts);
public:

    gb_lcd_interrupts(const gb_clock &clock,
                      const gb_lcd_scanline &scanline,
                      gb_events &events,
                      gb_interrupt_trigger &interrupts);

    uint8_t read_stat() const;
    void write_stat(uint8_t value);

    void trigger_interrupt_vblank();
    void trigger_interrupt_lyc();
    void set_back_clock(int clock_cycle_offset);

    void lcd_on();
    void lcd_off();
    void lyc_update();

private:

    static int add_total_frames(int clk_last, int clk_current);
    void schedule_lyc_irq();

    const gb_clock &m_clock;
    const gb_lcd_scanline &m_scanline;
    gb_events &m_events;
    gb_interrupt_trigger &m_interrupts;

    int m_clk_next_vblank_irq = gb_no_clock_cycle;
    int m_clk_next_lyc_irq = gb_no_clock_cycle;

    uint8_t m_stat = 0x80;
};



class gb_lcd
{
    AGE_DISABLE_COPY(gb_lcd);
public:

    gb_lcd(const gb_device &device,
           const gb_clock &clock,
           const gb_memory &memory,
           gb_events &events,
           gb_interrupt_trigger &interrupts,
           screen_buffer &screen_buffer,
           bool dmg_green);

    uint8_t read_lcdc() const;
    uint8_t read_stat();
    uint8_t read_scy() const;
    uint8_t read_scx() const;
    uint8_t read_ly();
    uint8_t read_lyc() const;
    uint8_t read_bgp() const;
    uint8_t read_obp0() const;
    uint8_t read_obp1() const;
    uint8_t read_wy() const;
    uint8_t read_wx() const;
    uint8_t read_bcps() const;
    uint8_t read_bcpd() const;
    uint8_t read_ocps() const;
    uint8_t read_ocpd() const;

    void write_lcdc(uint8_t value);
    void write_stat(uint8_t value);
    void write_scy(uint8_t value);
    void write_scx(uint8_t value);
    void write_lyc(uint8_t value);
    void write_bgp(uint8_t value);
    void write_obp0(uint8_t value);
    void write_obp1(uint8_t value);
    void write_wy(uint8_t value);
    void write_wx(uint8_t value);
    void write_bcps(uint8_t value);
    void write_bcpd(uint8_t value);
    void write_ocps(uint8_t value);
    void write_ocpd(uint8_t value);

    uint8_t* get_oam();
    void update_state();
    void trigger_interrupt_vblank();
    void trigger_interrupt_lyc();
    void set_back_clock(int clock_cycle_offset);

private:

    const gb_device &m_device;
    const gb_clock &m_clock;
    gb_lcd_scanline m_scanline;
    gb_lcd_interrupts m_lcd_interrupts;
    gb_lcd_palettes m_palettes;
    gb_lcd_render m_render;
};

} // namespace age



#endif // AGE_GB_LCD_HPP
