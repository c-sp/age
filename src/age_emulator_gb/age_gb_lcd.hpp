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
#include <gfx/age_pixel.hpp>
#include <gfx/age_screen_buffer.hpp>

#include "common/age_gb_clock.hpp"
#include "common/age_gb_device.hpp"
#include "common/age_gb_events.hpp"
#include "common/age_gb_interrupts.hpp"
#include "age_gb_memory.hpp"



namespace age
{

constexpr int16_t gb_screen_width = 160;
constexpr int16_t gb_screen_height = 144;

constexpr int gb_clock_cycles_per_scanline = 456;
constexpr int gb_scanline_count = 154;
constexpr int gb_clock_cycles_per_frame = gb_scanline_count * gb_clock_cycles_per_scanline;
//constexpr int gb_cycles_ly153 = 4;
//constexpr int gb_cycles_mode2 = 80;

constexpr uint8_t gb_lcdc_enable = 0x80;
constexpr uint8_t gb_lcdc_win_map = 0x40;
constexpr uint8_t gb_lcdc_win_enable = 0x20;
constexpr uint8_t gb_lcdc_bg_win_data = 0x10;
constexpr uint8_t gb_lcdc_bg_map = 0x08;
constexpr uint8_t gb_lcdc_obj_size = 0x04;
constexpr uint8_t gb_lcdc_obj_enable = 0x02;
constexpr uint8_t gb_lcdc_bg_enable = 0x01;

//constexpr uint8_t gb_stat_interrupt_coincidence = 0x40;
//constexpr uint8_t gb_stat_interrupt_mode2 = 0x20;
//constexpr uint8_t gb_stat_interrupt_mode1 = 0x10;
//constexpr uint8_t gb_stat_interrupt_mode0 = 0x08;
//constexpr uint8_t gb_stat_coincidence = 0x04;
constexpr uint8_t gb_stat_modes = 0x03;

//! The CGB has 8 palettes each for BG and OBJ.
constexpr unsigned gb_palette_count = 16;
//! Every palette contains 4 colors.
constexpr unsigned gb_total_color_count = gb_palette_count * 4;

//constexpr uint8_t gb_tile_attribute_x_flip = 0x20;
//constexpr uint8_t gb_tile_attribute_y_flip = 0x40;
//constexpr uint8_t gb_tile_attribute_priority = 0x80;



constexpr int gb_scanline_lcd_off = int_max;

class gb_lcd_scanline
{
    AGE_DISABLE_COPY(gb_lcd_scanline);
public:

    gb_lcd_scanline(const gb_clock &clock);

    int current_ly() const;
    int current_scanline() const;

    void lcd_on();
    void lcd_off();

    void fast_forward_frames();
    void set_back_clock(int clock_cycle_offset);

private:

    const gb_clock &m_clock;
    int m_clk_frame_start = gb_no_clock_cycle;
};



class gb_lcd_renderer
{
    AGE_DISABLE_COPY(gb_lcd_renderer);
public:

    gb_lcd_renderer(const gb_device &device,
                    const gb_memory &memory,
                    screen_buffer &screen_buffer,
                    bool dmg_green);

    uint8_t* get_oam();
    void set_lcdc(int lcdc);
    void create_dmg_palette(unsigned palette_idx, uint8_t colors);
    void update_color(unsigned color_idx, int gb_color);

    void render(int until_scanline);
    void new_frame();

private:

    const gb_device &m_device;
    const gb_memory &m_memory;
    screen_buffer &m_screen_buffer;

    uint8_array<0xA0> m_oam;
    pixel_vector m_colors = pixel_vector(gb_total_color_count, pixel(0, 0, 0));

    int m_rendered_scanlines = 0;
    int m_bg_tile_map_offset = 0;
    int m_win_tile_map_offset = 0;
    int m_tile_idx_offset = 0;
    int m_tile_data_offset = 0;
    const bool m_dmg_green;
};



class gb_lcd
{
    AGE_DISABLE_COPY(gb_lcd);
public:

    gb_lcd(const gb_device &device,
           const gb_clock &clock,
           const gb_memory &memory,
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
    void set_back_clock(int clock_cycle_offset);

private:

    void update_color(unsigned color_idx);

    const gb_device &m_device;
    const gb_clock &m_clock;
    gb_lcd_scanline m_scanline;
    gb_lcd_renderer m_renderer;

    uint8_array<gb_total_color_count * 2> m_cpd; // 2 bytes per color

    uint8_t m_lcdc = 0;
    uint8_t m_stat = 0;
    uint8_t m_scy = 0;
    uint8_t m_scx = 0;
    uint8_t m_lyc = 0;
    uint8_t m_bgp = 0;
    uint8_t m_obp0 = 0;
    uint8_t m_obp1 = 0;
    uint8_t m_wy = 0;
    uint8_t m_wx = 0;
    uint8_t m_bcps = 0xC0;
    uint8_t m_ocps = 0xC1;
};

} // namespace age



#endif // AGE_GB_LCD_HPP
