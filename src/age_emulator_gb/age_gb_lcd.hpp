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

#include <functional>
#include <vector>

#include <age_types.hpp>
#include <gfx/age_pixel.hpp>
#include <gfx/age_screen_buffer.hpp>

#include "common/age_gb_device.hpp"
#include "common/age_gb_clock.hpp"
#include "common/age_gb_interrupts.hpp"
#include "age_gb_core.hpp"
#include "age_gb_memory.hpp"



namespace age
{

constexpr int16_t gb_screen_width = 160;
constexpr int16_t gb_screen_height = 144;

constexpr int gb_cycles_per_scanline = 456;
constexpr int gb_cycles_per_frame = 154 * gb_cycles_per_scanline;
constexpr int gb_cycles_ly153 = 4;
constexpr int gb_cycles_mode2 = 80;

constexpr uint8_t gb_lcdc_enable = 0x80;
constexpr uint8_t gb_lcdc_win_map = 0x40;
constexpr uint8_t gb_lcdc_win_enable = 0x20;
constexpr uint8_t gb_lcdc_bg_win_data = 0x10;
constexpr uint8_t gb_lcdc_bg_map = 0x08;
constexpr uint8_t gb_lcdc_obj_size = 0x04;
constexpr uint8_t gb_lcdc_obj_enable = 0x02;
constexpr uint8_t gb_lcdc_bg_enable = 0x01;

constexpr uint8_t gb_stat_interrupt_coincidence = 0x40;
constexpr uint8_t gb_stat_interrupt_mode2 = 0x20;
constexpr uint8_t gb_stat_interrupt_mode1 = 0x10;
constexpr uint8_t gb_stat_interrupt_mode0 = 0x08;
constexpr uint8_t gb_stat_coincidence = 0x04;
constexpr uint8_t gb_stat_modes = 0x03;

constexpr unsigned gb_num_palette_colors = 16 * 4;

constexpr uint8_t gb_tile_attribute_x_flip = 0x20;
constexpr uint8_t gb_tile_attribute_y_flip = 0x40;
constexpr uint8_t gb_tile_attribute_priority = 0x80;



class gb_ly_counter
{
    AGE_DISABLE_COPY(gb_ly_counter);

public:

    gb_ly_counter(const gb_device &device, const gb_clock &clock, gb_core &core, gb_interrupt_trigger &interrupts);

    uint8_t get_ly_port(bool lcd_enabled) const;
    uint8_t get_ly() const;
    uint8_t get_scanline() const;
    int get_next_scanline_cycle_offset(int current_cycle) const;

    void switch_off();
    void switch_on();
    void next_line();
    void mode1_ly0();

    void set_back_clock(int clock_cycle_offset);

protected:

    const gb_device &m_device;
    const gb_clock &m_clock;
    gb_core &m_core;
    gb_interrupt_trigger &m_interrupts;

private:

    int m_next_scanline_cycle = gb_no_clock_cycle;
    uint8_t m_scanline = 0;
    bool m_mode1_ly0 = false;
};



class gb_lyc_handler : public gb_ly_counter
{
public:

    using gb_ly_counter::gb_ly_counter;

    uint8_t get_lyc() const;
    bool is_interruptable_coincidence() const;
    uint8_t get_stat_coincidence(bool lcd_enabled) const;

    void switch_off();
    void set_stat(uint8_t value, int mode, bool lcd_enabled);
    void set_lyc(uint8_t value, int mode, bool lcd_enabled);

protected:

    bool get_stat_coincidence_interrupt() const;
    bool get_stat_mode2_interrupt() const;
    bool get_stat_mode1_interrupt() const;

private:

    bool m_stat_coincidence_lcd_off = false;
    bool m_stat_coincidence_interrupt = false;
    bool m_stat_mode0_interrupt = false;
    bool m_stat_mode1_interrupt = false;
    bool m_stat_mode2_interrupt = false;
    uint8_t m_lyc = 0;
};



class gb_lyc_interrupter : private gb_lyc_handler
{
public:

    using gb_lyc_handler::gb_lyc_handler;

    using gb_ly_counter::get_ly_port;
    using gb_ly_counter::get_ly;
    using gb_ly_counter::get_scanline;
    using gb_ly_counter::get_next_scanline_cycle_offset;
    using gb_ly_counter::next_line;
    using gb_ly_counter::mode1_ly0;

    using gb_lyc_handler::get_lyc;
    using gb_lyc_handler::is_interruptable_coincidence;
    using gb_lyc_handler::get_stat_coincidence;

    void switch_off();
    void switch_on();
    void set_stat(uint8_t value, int mode, bool lcd_enabled);
    void set_lyc(uint8_t value, int mode, bool lcd_enabled);
    void lyc_event();

    void set_back_clock(int clock_cycle_offset);

private:

    int calculate_next_event_cycle(bool stat_coincidence_interrupt, uint8_t for_lyc);
    void schedule_next_event(int next_event_cycle);

    bool m_stat_coincidence_interrupt_int = false;
    bool m_stat_mode1_interrupt_int = false;
    bool m_stat_mode2_interrupt_int = false;
    uint8_t m_lyc_int = 0;
    int m_next_event_cycle = int_max;
};





class gb_lcd_ppu : public gb_lyc_interrupter
{
public:

    gb_lcd_ppu(const gb_device &device, const gb_clock &clock, gb_core &core, gb_interrupt_trigger &interrupts, const gb_memory &memory, bool dmg_green);

    uint8_t read_lcdc() const;
    uint8_t read_scx() const;
    uint8_t read_scy() const;
    uint8_t read_wx() const;
    uint8_t read_wy() const;

    void write_lcdc(uint8_t value);
    void write_scx(uint8_t value);
    void write_scy(uint8_t value);
    void write_wx(uint8_t value);
    void write_wy(uint8_t value);

    int get_mode0_interrupt_cycle_offset() const;
    uint8_t* get_oam();

    void create_classic_palette(unsigned index, uint8_t colors);
    void update_color(unsigned index, uint8_t high_byte, uint8_t low_byte);
    void white_screen(pixel_vector &screen);
    void write_late_wy();
    void search_sprites();

    void scanline_init(pixel *first_scanline_pixel);
    void scanline_step();
    bool scanline_nearly_finished() const;
    bool scanline_finished() const;
    bool scanline_flag_mode0() const;
    bool scanline_mode0_interrupt() const;

protected:

    const gb_device &m_device;
    const gb_clock &m_clock;

private:

    class gb_sprite
    {
    public:

        gb_sprite();
        gb_sprite(const uint8_t* oam, int16_t line);

        void fetch_tile_byte1(const gb_lcd_ppu &ppu);
        void fetch_tile_byte2(const gb_lcd_ppu &ppu);
        void create_tile(const gb_lcd_ppu &ppu);

        uint8_t get_priority() const;
        uint8_t get_color_index(int offset) const;
        int16_t get_x() const;

        bool operator<(const gb_sprite &right) const;

    private:

        int get_tile_data_offset(const gb_lcd_ppu &ppu) const;

        int16_t m_x = 255;
        int16_t m_line = 0;
        uint8_t m_tile_name = 0;
        uint8_t m_tile_attributes = 0;
        uint8_t m_tile_byte1 = 0;
        uint8_t m_tile_byte2 = 0;

        uint8_array<8> m_tile;
        uint8_t m_priority = 0;
    };

    static void calculate_xflip(uint8_array<256> &xflip);
    static void create_tile_cache(uint8_array<1024> &data);

    static void plot_await_scx_match(gb_lcd_ppu &ppu);
    static void plot_await_data(gb_lcd_ppu &ppu);
    static void plot_pixel(gb_lcd_ppu &ppu);
    static uint8_t get_sprite_pixel(const gb_lcd_ppu &ppu, uint8_t bg_color_index);
    void update_pause_plotting();

    void fetch_tile_name();
    void fetch_tile_byte_1();
    void fetch_tile_byte_2();
    int get_tile_data_offset() const;

    static void tile_step_0(gb_lcd_ppu &ppu);
    static void tile_step_1(gb_lcd_ppu &ppu);
    static void tile_step_2(gb_lcd_ppu &ppu);
    static void tile_step_3(gb_lcd_ppu &ppu);
    static void tile_step_4(gb_lcd_ppu &ppu);
    static void tile_step_5(gb_lcd_ppu &ppu);
    static void tile_step_6(gb_lcd_ppu &ppu);

    static void sprite_step_0(gb_lcd_ppu &ppu);
    static void sprite_step_1(gb_lcd_ppu &ppu);
    static void sprite_step_2(gb_lcd_ppu &ppu);
    static void sprite_step_3(gb_lcd_ppu &ppu);
    static void sprite_step_4(gb_lcd_ppu &ppu);
    static void sprite_step_5(gb_lcd_ppu &ppu);
    static void sprite_step_6(gb_lcd_ppu &ppu);

    static void window_step_0(gb_lcd_ppu &ppu);
    static void window_step_1(gb_lcd_ppu &ppu);
    static void window_step_2(gb_lcd_ppu &ppu);
    static void window_step_3(gb_lcd_ppu &ppu);
    static void window_step_4(gb_lcd_ppu &ppu);
    static void window_step_5(gb_lcd_ppu &ppu);
    static void window_step_6(gb_lcd_ppu &ppu);
    void window_set_next_step(std::function<void(gb_lcd_ppu&)> next_step);

    // common stuff
    const bool m_dmg_green;
    const gb_memory &m_memory;
    uint8_array<0xA0> m_oam;
    std::function<void(gb_lcd_ppu&)> m_next_fetch_step = nullptr;
    std::function<void(gb_lcd_ppu&)> m_next_plot_step = nullptr;

    // buffers & caches
    uint8_array<1024> m_tile_cache;
    uint8_array<256> m_xflip_cache;
    pixel_vector m_colors = pixel_vector(gb_num_palette_colors, pixel(0, 0, 0));

    // precalculated from LCDC
    bool m_obj_enabled = false;
    bool m_obj_size_16 = false;
    bool m_win_enabled = false;
    int16_t m_b_tile_name_offset = 0;
    int16_t m_bw_tile_name_add = 0;
    int16_t m_w_tile_name_offset = 0;
    int16_t m_bw_tile_data_offset = 0;

    // ports
    uint8_t m_lcdc = gb_lcdc_enable;
    uint8_t m_scy = 0;
    uint8_t m_scx = 0;
    uint8_t m_wy = 0;
    uint8_t m_wx = 0;

    // rendering
    int8_t m_x_scx = 0;
    int16_t m_x_current = 0;
    int16_t m_x_m0 = 168;
    int16_t m_x_m0_int = 168;
    pixel *m_next_pixel = nullptr;
    bool m_pause_plotting = false;

    bool m_window_started = false;
    bool m_plotting_window = false;
    int16_t m_window_map_offset = 0;
    uint8_t m_late_wy = 0;

    std::vector<gb_sprite> m_sprites;
    uint8_t m_current_sprite = 0;
    uint8_t m_sprite_to_plot = 0;
    bool m_sprite_at_167 = false;
    int8_t m_sprite_tile_offset = 0;

    uint8_array<8> m_tile;
    uint8_t m_tile_offset = 0;
    uint8_t m_tile_priority = 0;

    uint8_array<8> m_new_tile;
    uint8_t m_tile_map_offset = 0;
    uint8_t m_new_tile_name = 0;
    uint8_t m_new_tile_attributes = 0;
    uint8_t m_new_tile_byte1 = 0;
    uint8_t m_new_tile_byte2 = 0;
};





class gb_lcd : private gb_lcd_ppu
{
public:

    gb_lcd(const gb_device &device, const gb_clock &clock, gb_core &core, gb_interrupt_trigger &interrupts, const gb_memory &memory, screen_buffer &frame_handler, bool dmg_green);

    using gb_lcd_ppu::read_lcdc;
    using gb_lcd_ppu::read_scx;
    using gb_lcd_ppu::read_scy;
    using gb_lcd_ppu::read_wx;
    using gb_lcd_ppu::read_wy;

    uint8_t read_stat() const;
    uint8_t read_ly() const;
    uint8_t read_lyc() const;
    uint8_t read_bgp() const;
    uint8_t read_obp0() const;
    uint8_t read_obp1() const;

    uint8_t read_bcps() const;
    uint8_t read_bcpd() const;
    uint8_t read_ocps() const;
    uint8_t read_ocpd() const;

    void write_lcdc(uint8_t value);
    void write_stat(uint8_t value);
    void write_scx(uint8_t value);
    void write_scy(uint8_t value);
    void write_lyc(uint8_t value);
    void write_bgp(uint8_t value);
    void write_obp0(uint8_t value);
    void write_obp1(uint8_t value);
    void write_wx(uint8_t value);
    void write_wy(uint8_t value);

    void write_bcps(uint8_t value);
    void write_bcpd(uint8_t value);
    void write_ocps(uint8_t value);
    void write_ocpd(uint8_t value);

    using gb_lyc_interrupter::lyc_event;
    using gb_lcd_ppu::get_oam;

    bool is_video_ram_accessible() const;
    bool is_oam_readable() const;
    bool is_oam_writable() const;
    bool is_hdma_active() const;

    void emulate();
    void set_hdma_active(bool hdma_active);

    void set_back_clock(int clock_cycle_offset);

private:

    void emulate(int to_cycle);
    void next_step(int cycle_offset, std::function<void(gb_lcd&)> next_method);
    void switch_frames();

    void update_color(unsigned index);
    bool is_cgb_palette_accessible() const;

    // LCD mode emulation
    void mode0_start_lcd_enabled();
    void mode0_interrupt();
    void mode0_next_event();

    static void mode1_start_line(gb_lcd &lcd);
    static void mode1_start_ly0(gb_lcd &lcd);
    static void mode1_update_ly0_lyc(gb_lcd &lcd);
    static void mode1_last_cycles_mode0(gb_lcd &lcd);

    static void mode2_start_ly0(gb_lcd &lcd);
    static void mode2_early_interrupt(gb_lcd &lcd);
    static void mode2_start(gb_lcd &lcd);

    static void mode3_start(gb_lcd &lcd);
    static void mode3_render(gb_lcd &lcd);

    // common stuff
    gb_core &m_core;
    gb_interrupt_trigger &m_interrupts;
    screen_buffer &m_screen_buffer;
    uint8_array<gb_num_palette_colors * 2> m_palette; // 2 bytes per color
    std::function<void(gb_lcd&)> m_next_event = nullptr;
    int m_next_event_cycle = gb_no_clock_cycle;
    int m_last_cycle_m3_finished = 0;
    bool m_hdma_active = false;

    // precalculated from LCDC and STAT
    uint8_t m_stat_mode0_int = false;
    uint8_t m_lyc_mode0_int = 0;
    bool m_skip_next_mode1_interrupt = false;
    bool m_allow_mode1_interrupt = false;
    bool m_allow_mode2_interrupt = false;
    bool m_allow_mode2_ly0_interrupt = false;
    bool m_allow_mode2_ly0_interrupt_int = false;
    bool m_lcd_enabled = true;

    // ports
    uint8_t m_stat = 1;
    uint8_t m_bgp = 0;
    uint8_t m_obp0 = 0;
    uint8_t m_obp1 = 0;
    uint8_t m_bcps = 0xC0;
    uint8_t m_ocps = 0xC1;
};

} // namespace age



#endif // AGE_GB_LCD_HPP
