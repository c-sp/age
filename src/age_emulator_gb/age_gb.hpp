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

#ifndef AGE_GB_HPP
#define AGE_GB_HPP

//!
//! \file
//!

#include <age_debug.hpp>
#include <age_types.hpp>



#ifdef AGE_DEBUG
#define AGE_GB_CYCLE_LOG(x) AGE_LOG("cycle " << m_core.get_oscillation_cycle() << ": " << x)
#else
#define AGE_GB_CYCLE_LOG(x)
#endif



namespace age
{

constexpr uint gb_no_cycle = uint_max;
constexpr uint gb_machine_cycles_per_second = 4194304;

constexpr const uint8_array<5> gb_interrupt_pc_lookup =
{{
     0x00, 0x40, 0x48, 0x48, 0x50
 }};

constexpr uint8 gb_tac_start_timer = 0x04;

constexpr uint8 gb_hdma_start = 0x80;



// memory

constexpr uint gb_cart_rom_bank_size = 0x4000;
constexpr uint gb_cart_ram_bank_size = 0x2000;
constexpr uint gb_internal_ram_bank_size = 0x1000;
constexpr uint gb_video_ram_bank_size = 0x2000;

constexpr uint gb_internal_ram_size = 8 * gb_internal_ram_bank_size;
constexpr uint gb_high_ram_size = 0x200; // 0xFE00 - 0xFFFF (including OAM ram and i/o ports for easier handling)
constexpr uint gb_video_ram_size = 2 * gb_video_ram_bank_size;

constexpr uint gb_oam_size = 0xA0;



// serial i/o

constexpr uint gb_serial_transfer_cycles = gb_machine_cycles_per_second / (8192 / 8); // bit transfer with 8192 Hz

constexpr uint8 gb_sc_start_transfer = 0x80;
constexpr uint8 gb_sc_terminal_selection = 0x01;
constexpr uint8 gb_sc_shift_clock = 0x02;



// cartridge information area

constexpr uint gb_header_size = 0x0150;
constexpr uint gb_cia_ofs_title = 0x0134;
constexpr uint gb_cia_ofs_cgb = 0x0143;
//constexpr uint gb_cia_ofs_licensee_new_low = 0x0144;
//constexpr uint gb_cia_ofs_licensee_new_high = 0x0145;
//constexpr uint gb_cia_ofs_sgb = 0x0146;
constexpr uint gb_cia_ofs_type = 0x0147;
constexpr uint gb_cia_ofs_rom_size = 0x0148;
constexpr uint gb_cia_ofs_ram_size = 0x0149;
//constexpr uint gb_cia_ofs_destination = 0x014A;
//constexpr uint gb_cia_ofs_licensee = 0x014B;
//constexpr uint gb_cia_ofs_version = 0x014C;
//constexpr uint gb_cia_ofs_header_checksum = 0x014D;
//constexpr uint gb_cia_ofs_global_checksum_low = 0x014E;
//constexpr uint gb_cia_ofs_global_checksum_high = 0x014F;



// joypad

constexpr uint gb_right = 0x01;
constexpr uint gb_left = 0x02;
constexpr uint gb_up = 0x04;
constexpr uint gb_down = 0x08;
constexpr uint gb_a = 0x10;
constexpr uint gb_b = 0x20;
constexpr uint gb_select = 0x40;
constexpr uint gb_start = 0x80;

constexpr uint8 gb_p14 = 0x10;
constexpr uint8 gb_p15 = 0x20;



// lcd

constexpr uint gb_screen_width = 160;
constexpr uint gb_screen_height = 144;

constexpr uint gb_cycles_per_scanline = 456;
constexpr uint gb_cycles_per_frame = 154 * gb_cycles_per_scanline;
constexpr uint gb_cycles_ly153 = 4;
constexpr uint gb_cycles_mode2 = 80;

constexpr uint8 gb_lcdc_enable = 0x80;
constexpr uint8 gb_lcdc_win_map = 0x40;
constexpr uint8 gb_lcdc_win_enable = 0x20;
constexpr uint8 gb_lcdc_bg_win_data = 0x10;
constexpr uint8 gb_lcdc_bg_map = 0x08;
constexpr uint8 gb_lcdc_obj_size = 0x04;
constexpr uint8 gb_lcdc_obj_enable = 0x02;
constexpr uint8 gb_lcdc_bg_enable = 0x01;

constexpr uint8 gb_stat_interrupt_coincidence = 0x40;
constexpr uint8 gb_stat_interrupt_mode2 = 0x20;
constexpr uint8 gb_stat_interrupt_mode1 = 0x10;
constexpr uint8 gb_stat_interrupt_mode0 = 0x08;
constexpr uint8 gb_stat_coincidence = 0x04;
constexpr uint8 gb_stat_modes = 0x03;

constexpr uint gb_num_palette_colors = 16 * 4;
constexpr uint gb_sprite_cache_size = 40;
constexpr uint gb_palette_bgp = 0x00;
constexpr uint gb_palette_obp0 = 0x20;
constexpr uint gb_palette_obp1 = 0x30;

constexpr uint8 gb_tile_attribute_x_flip = 0x20;
constexpr uint8 gb_tile_attribute_y_flip = 0x40;
constexpr uint8 gb_tile_attribute_priority = 0x80;



// sound

constexpr uint gb_sample_cycle_shift = 1; // 2097152 samples per second for easier emulation (will be downsampled later on)
constexpr uint gb_sampling_rate = gb_machine_cycles_per_second >> gb_sample_cycle_shift;
constexpr uint gb_cycles_per_sample = 1 << gb_sample_cycle_shift;
constexpr uint gb_cycle_sample_mask = ~(gb_cycles_per_sample - 1);

constexpr uint gb_channel_1 = 0;
constexpr uint gb_channel_2 = 1;
constexpr uint gb_channel_3 = 2;
constexpr uint gb_channel_4 = 3;

constexpr uint gb_frame_sequencer_cycle_shift = 13;

constexpr uint8 gb_nrX4_length_counter = 0x40;
constexpr uint8 gb_nrX4_initialize = 0x80;
constexpr uint8 gb_master_switch = 0x80;

constexpr const std::array<uint8_array<32>, 4> gb_wave_pattern_duty =
{{
     {{  0, 0, 0, 0,  15, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 }},
     {{  0, 0, 0, 0,  15,15, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 }},
     {{  0, 0,15,15,  15,15, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 }},
     {{ 15,15,15,15,   0, 0,15,15,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 }},
 }};

constexpr const uint8_array<4> gb_channel_bit =
{{
     0x01, 0x02, 0x04, 0x08
 }};



// cpu

constexpr uint8 gb_zero_flag = 0x80;
constexpr uint8 gb_subtract_flag = 0x40;
constexpr uint8 gb_half_carry_flag = 0x20;
constexpr uint8 gb_carry_flag = 0x10;

constexpr uint gb_hcs_shift = 4;
constexpr uint gb_hcs_half_carry = gb_half_carry_flag << gb_hcs_shift;
constexpr uint gb_hcs_subtract = gb_subtract_flag << gb_hcs_shift;
constexpr uint gb_hcs_old_carry = gb_carry_flag << gb_hcs_shift;
constexpr uint gb_hcs_flags = gb_hcs_half_carry + gb_hcs_subtract;

} // namespace age



#endif // AGE_GB_HPP
