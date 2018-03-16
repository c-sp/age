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

#ifndef AGE_GB_EMULATOR_IMPL_HPP
#define AGE_GB_EMULATOR_IMPL_HPP

//!
//! \file
//!

#include <age_non_copyable.hpp>
#include <age_types.hpp>
#include <emulator/age_gb_emulator.hpp>

#include "age_gb_bus.hpp"
#include "age_gb_core.hpp"
#include "age_gb_cpu.hpp"
#include "age_gb_joypad.hpp"
#include "age_gb_lcd.hpp"
#include "age_gb_memory.hpp"
#include "age_gb_serial.hpp"
#include "age_gb_sound.hpp"
#include "age_gb_timer.hpp"



namespace age {



class gb_emulator_impl : public non_copyable
{
public:

    gb_emulator_impl(const uint8_vector &rom, bool force_dmg, bool dmg_green, pcm_vector &pcm_vec, screen_buffer &screen_buf);

    bool is_cgb() const;
    gb_test_info get_test_info() const;

    uint8_vector get_persistent_ram() const;
    void set_persistent_ram(const uint8_vector &source);

    void set_buttons_down(uint buttons);
    void set_buttons_up(uint buttons);

    uint64 inner_emulate(uint64 min_cycles_to_emulate);

    std::string inner_get_emulator_title() const;

private:

    gb_memory m_memory;
    gb_core m_core;
    gb_sound m_sound;
    gb_lcd m_lcd;
    gb_timer m_timer;
    gb_joypad m_joypad;
    gb_serial m_serial;
    gb_bus m_bus;
    gb_cpu m_cpu;
};



} // namespace age

#endif // AGE_GB_EMULATOR_IMPL_HPP
