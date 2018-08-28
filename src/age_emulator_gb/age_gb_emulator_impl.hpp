//
// Copyright 2018 Christoph Sprenger
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

#ifndef AGE_GB_EMULATOR_IMPL_HPP
#define AGE_GB_EMULATOR_IMPL_HPP

//!
//! \file
//!

#include <age_non_copyable.hpp>
#include <age_types.hpp>
#include <emulator/age_gb_types.hpp>

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

    gb_emulator_impl(const uint8_vector &rom, gb_hardware hardware, bool dmg_green, pcm_vector &pcm_vec, screen_buffer &screen_buf);

    gb_test_info get_test_info() const;

    uint8_vector get_persistent_ram() const;
    void set_persistent_ram(const uint8_vector &source);

    void set_buttons_down(int32_t buttons);
    void set_buttons_up(int32_t buttons);

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
