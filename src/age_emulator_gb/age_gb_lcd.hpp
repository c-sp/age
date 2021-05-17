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

#include "age_gb_lcd_render.hpp"



namespace age
{

constexpr int gb_clock_cycles_per_scanline = 456;
constexpr int gb_scanline_count = 154;
constexpr int gb_clock_cycles_per_frame = gb_scanline_count * gb_clock_cycles_per_scanline;

constexpr int gb_lcd_m_cycle_align = -3;

constexpr uint8_t gb_stat_irq_ly_match = 0x40;
constexpr uint8_t gb_stat_irq_mode2 = 0x20;
constexpr uint8_t gb_stat_irq_mode1 = 0x10;
constexpr uint8_t gb_stat_irq_mode0 = 0x08;
constexpr uint8_t gb_stat_ly_match = 0x04;
constexpr uint8_t gb_stat_modes = 0x03;



class gb_lcd_scanline
{
    AGE_DISABLE_COPY(gb_lcd_scanline);
public:

    gb_lcd_scanline(const gb_device &device, const gb_clock &clock);

    void set_back_clock(int clock_cycle_offset);

    bool lcd_is_on() const;
    void lcd_on();
    void lcd_off();

    int clk_frame_start() const;
    bool is_first_frame() const;
    void fast_forward_frames();

    void current_scanline(int &scanline, int &scanline_clks) const;

private:

    const gb_clock &m_clock;
    int m_clk_frame_start = gb_no_clock_cycle;
    mutable int m_clk_scanline_start = gb_no_clock_cycle;
    mutable int m_scanline = 0;
    bool m_first_frame = false;

public:

    uint8_t m_lyc = 0;
};



class gb_lcd_irqs
{
    AGE_DISABLE_COPY(gb_lcd_irqs);
public:

    gb_lcd_irqs(const gb_device &device,
                const gb_clock &clock,
                const gb_lcd_scanline &scanline,
                gb_events &events,
                gb_interrupt_trigger &interrupts);

    uint8_t read_stat() const;
    void write_stat(uint8_t value, int scx);
    void lyc_update();

    void trigger_irq_vblank();
    void trigger_irq_lyc();
    void trigger_irq_mode2();
    void trigger_irq_mode0(int scx);

    void lcd_on(int scx);
    void lcd_off();
    void set_back_clock(int clock_cycle_offset);

private:

    void schedule_irq_vblank();
    void schedule_irq_lyc();
    void schedule_irq_mode2();
    void schedule_irq_mode0(int scx);

    const gb_device &m_device;
    const gb_clock &m_clock;
    const gb_lcd_scanline &m_scanline;
    gb_events &m_events;
    gb_interrupt_trigger &m_interrupts;

    int m_clk_next_irq_vblank = gb_no_clock_cycle;
    int m_clk_next_irq_lyc = gb_no_clock_cycle;
    int m_clk_next_irq_mode2 = gb_no_clock_cycle;
    int m_clk_next_irq_mode0 = gb_no_clock_cycle;

    uint8_t m_stat = 0x80;
};



class gb_lcd
{
    AGE_DISABLE_COPY(gb_lcd);
public:

    gb_lcd(const gb_device &device,
           const gb_clock &clock,
           const uint8_t *video_ram,
           const uint8_t *rom_header,
           gb_events &events,
           gb_interrupt_trigger &interrupts,
           screen_buffer &screen_buffer,
           gb_colors_hint colors_hint);

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

    uint8_t read_oam(int offset);
    void write_oam(int offset, uint8_t value);
    bool is_oam_accessible();
    bool is_video_ram_accessible();

    void update_state();
    void trigger_irq_vblank();
    void trigger_irq_lyc();
    void trigger_irq_mode2();
    void trigger_irq_mode0();
    void set_back_clock(int clock_cycle_offset);

private:

    void calculate_scanline(int &scanline, int &scanline_clks);

    uint8_t get_stat_mode(int scanline, int scanline_clks, int scx) const;
    uint8_t get_stat_ly_match(int scanline, int scanline_clks) const;

    const gb_device &m_device;
    const gb_clock &m_clock;
    gb_lcd_scanline m_scanline;
    gb_lcd_irqs m_lcd_irqs;
    gb_lcd_palettes m_palettes;
    gb_lcd_sprites m_sprites;
    gb_lcd_render m_render;

    uint8_t m_retained_ly_match = 0;
};

} // namespace age



#endif // AGE_GB_LCD_HPP
