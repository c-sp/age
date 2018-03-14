//
// Copyright (c) 2010-2017 Christoph Sprenger
//
// This file is part of AGE ("Another Gameboy Emulator").
// <https://gitlab.com/csprenger/AGE>
//
// AGE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// AGE is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with AGE.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AGE_GB_LCD_HPP
#define AGE_GB_LCD_HPP

//!
//! \file
//!

#include <functional>
#include <vector>

#include <age_graphics.hpp>
#include <age_non_copyable.hpp>
#include <age_types.hpp>

#include "age_gb_core.hpp"
#include "age_gb_memory.hpp"



namespace age
{



class ly_counter : public non_copyable
{
public:

    ly_counter(gb_core &core);

    uint8 get_ly_port(bool lcd_enabled) const;
    uint8 get_ly() const;
    uint get_scanline() const;
    uint get_next_scanline_cycle_offset(uint current_cycle) const;

    void switch_off();
    void switch_on();
    void next_line();
    void mode1_ly0();

protected:

    gb_core *const m_core;
    const bool m_cgb;

private:

    uint m_next_scanline_cycle = gb_no_cycle;
    uint m_scanline = 0;
    bool m_mode1_ly0 = false;
};



class lyc_handler : public ly_counter
{
public:

    using ly_counter::ly_counter;

    uint8 get_lyc() const;
    bool is_interruptable_coincidence() const;
    uint8 get_stat_coincidence(bool lcd_enabled) const;

    void switch_off();
    void set_stat(uint8 value, uint mode, bool lcd_enabled);
    void set_lyc(uint8 value, uint mode, bool lcd_enabled);

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
    uint8 m_lyc = 0;
};



class lyc_interrupter : private lyc_handler
{
public:

    using lyc_handler::lyc_handler;

    using ly_counter::get_ly_port;
    using ly_counter::get_ly;
    using ly_counter::get_scanline;
    using ly_counter::get_next_scanline_cycle_offset;
    using ly_counter::next_line;
    using ly_counter::mode1_ly0;

    using lyc_handler::get_lyc;
    using lyc_handler::is_interruptable_coincidence;
    using lyc_handler::get_stat_coincidence;

    void switch_off();
    void switch_on();
    void set_stat(uint8 value, uint mode, bool lcd_enabled);
    void set_lyc(uint8 value, uint mode, bool lcd_enabled);
    void lyc_event();

private:

    uint calculate_next_event_cycle(bool stat_coincidence_interrupt, uint8 for_lyc);
    void schedule_next_event();

    bool m_stat_coincidence_interrupt_int = false;
    bool m_stat_mode1_interrupt_int = false;
    bool m_stat_mode2_interrupt_int = false;
    uint8 m_lyc_int = 0;
    uint m_next_event_cycle = gb_no_cycle;
};





class gb_lcd_ppu : public lyc_interrupter
{
public:

    gb_lcd_ppu(gb_core &core, const gb_memory &memory, bool dmg_green);

    uint8 read_lcdc() const;
    uint8 read_scx() const;
    uint8 read_scy() const;
    uint8 read_wx() const;
    uint8 read_wy() const;

    void write_lcdc(uint8 value);
    void write_scx(uint8 value);
    void write_scy(uint8 value);
    void write_wx(uint8 value);
    void write_wy(uint8 value);

    uint get_mode0_interrupt_cycle_offset() const;
    uint8* get_oam();

    void create_classic_palette(uint index, uint8 colors);
    void update_color(uint index, uint8 high_byte, uint8 low_byte);
    void white_screen(pixel_vector &screen);
    void write_late_wy();
    void search_sprites();

    void scanline_init(pixel *first_scanline_pixel);
    void scanline_step();
    bool scanline_nearly_finished() const;
    bool scanline_finished() const;
    bool scanline_flag_mode0() const;
    bool scanline_mode0_interrupt() const;

private:

    class gb_sprite
    {
    public:

        gb_sprite();
        gb_sprite(const uint8* oam, uint line);

        void fetch_tile_byte1(const gb_lcd_ppu &ppu);
        void fetch_tile_byte2(const gb_lcd_ppu &ppu);
        void create_tile(const gb_lcd_ppu &ppu);

        uint8 get_priority() const;
        uint8 get_color_index(uint offset) const;
        uint get_x() const;

        bool operator<(const gb_sprite &right) const;

    private:

        uint get_tile_data_offset(const gb_lcd_ppu &ppu) const;

        uint m_x = 255;
        uint m_line = 0;
        uint8 m_tile_name = 0;
        uint8 m_tile_attributes = 0;
        uint8 m_tile_byte1 = 0;
        uint8 m_tile_byte2 = 0;

        uint8_array<8> m_tile;
        uint8 m_priority = 0;
    };

    static void calculate_xflip(uint8_array<256> &xflip);
    static void create_tile_cache(uint8_array<1024> &data);

    static void plot_await_scx_match(gb_lcd_ppu &ppu);
    static void plot_await_data(gb_lcd_ppu &ppu);
    static void plot_pixel(gb_lcd_ppu &ppu);
    static uint8 get_sprite_pixel(const gb_lcd_ppu &ppu, uint8 bg_color_index);
    void update_pause_plotting();

    void fetch_tile_name();
    void fetch_tile_byte_1();
    void fetch_tile_byte_2();
    uint get_tile_data_offset() const;

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
    const bool m_cgb;
    const bool m_dmg_green;
    gb_core &m_core;
    const gb_memory &m_memory;
    uint8_array<gb_oam_size> m_oam;
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
    uint m_b_tile_name_offset = 0;
    uint m_bw_tile_name_add = 0;
    uint m_w_tile_name_offset = 0;
    uint m_bw_tile_data_offset = 0;

    // ports
    uint8 m_lcdc = gb_lcdc_enable;
    uint8 m_scy = 0;
    uint8 m_scx = 0;
    uint8 m_wy = 0;
    uint8 m_wx = 0;

    // rendering
    uint m_x_scx = 0;
    uint m_x_current = 0;
    uint m_x_m0 = 168;
    uint m_x_m0_int = 168;
    pixel *m_next_pixel = nullptr;
    bool m_pause_plotting = false;

    bool m_window_started = false;
    bool m_plotting_window = false;
    uint m_window_map_offset = 0;
    uint8 m_late_wy = 0;

    std::vector<gb_sprite> m_sprites;
    uint m_current_sprite = 0;
    uint m_sprite_to_plot = 0;
    bool m_sprite_at_167 = false;
    uint m_sprite_tile_offset = 0;

    uint8_array<8> m_tile;
    uint m_tile_offset = 0;
    uint8 m_tile_priority = 0;

    uint8_array<8> m_new_tile;
    uint m_tile_map_offset = 0;
    uint8 m_new_tile_name = 0;
    uint8 m_new_tile_attributes = 0;
    uint8 m_new_tile_byte1 = 0;
    uint8 m_new_tile_byte2 = 0;
};





class gb_lcd : private gb_lcd_ppu
{
public:

    gb_lcd(gb_core &core, const gb_memory &memory, video_buffer_handler &frame_handler, bool dmg_green);

    using gb_lcd_ppu::read_lcdc;
    using gb_lcd_ppu::read_scx;
    using gb_lcd_ppu::read_scy;
    using gb_lcd_ppu::read_wx;
    using gb_lcd_ppu::read_wy;

    uint8 read_stat() const;
    uint8 read_ly() const;
    uint8 read_lyc() const;
    uint8 read_bgp() const;
    uint8 read_obp0() const;
    uint8 read_obp1() const;

    uint8 read_bcps() const;
    uint8 read_bcpd() const;
    uint8 read_ocps() const;
    uint8 read_ocpd() const;

    void write_lcdc(uint8 value);
    void write_stat(uint8 value);
    void write_scx(uint8 value);
    void write_scy(uint8 value);
    void write_lyc(uint8 value);
    void write_bgp(uint8 value);
    void write_obp0(uint8 value);
    void write_obp1(uint8 value);
    void write_wx(uint8 value);
    void write_wy(uint8 value);

    void write_bcps(uint8 value);
    void write_bcpd(uint8 value);
    void write_ocps(uint8 value);
    void write_ocpd(uint8 value);

    using lyc_interrupter::lyc_event;
    using gb_lcd_ppu::get_oam;

    bool is_video_ram_accessible() const;
    bool is_oam_readable() const;
    bool is_oam_writable() const;
    bool is_hdma_active() const;

    void emulate();
    void set_hdma_active(bool hdma_active);

private:

    void emulate(uint to_cycle);
    void next_step(uint cycle_offset, std::function<void(gb_lcd&)> next_method);
    void switch_frames();

    void update_color(uint index);
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
    const bool m_cgb;
    gb_core &m_core;
    video_buffer_handler &m_video_buffer_handler;
    uint8_array<gb_num_palette_colors * 2> m_palette; // 2 bytes per color
    std::function<void(gb_lcd&)> m_next_event = nullptr;
    uint m_next_event_cycle = 0;
    uint m_m3_last_finished = 0;
    bool m_hdma_active = false;

    // precalculated from LCDC and STAT
    uint8 m_stat_mode0_int = false;
    uint8 m_lyc_mode0_int = 0;
    bool m_skip_next_mode1_interrupt = false;
    bool m_allow_mode1_interrupt = false;
    bool m_allow_mode2_interrupt = false;
    bool m_allow_mode2_ly0_interrupt = false;
    bool m_allow_mode2_ly0_interrupt_int = false;
    bool m_lcd_enabled = true;

    // ports
    uint8 m_stat = 1;
    uint8 m_bgp = 0;
    uint8 m_obp0 = 0;
    uint8 m_obp1 = 0;
    uint8 m_bcps = 0xC0;
    uint8 m_ocps = 0xC1;
};

} // namespace age



#endif // AGE_GB_LCD_HPP
