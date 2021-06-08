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

#include <emulator/age_gb_emulator.hpp>

#include "age_gb_emulator_impl.hpp"



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

age::gb_test_info age::gb_emulator::get_test_info() const
{
    return m_impl->get_test_info();
}

age::uint8_vector age::gb_emulator::get_persistent_ram() const
{
    return m_impl->get_persistent_ram();
}

void age::gb_emulator::set_persistent_ram(const uint8_vector& source)
{
    m_impl->set_persistent_ram(source);
}

void age::gb_emulator::set_buttons_down(int buttons)
{
    m_impl->set_buttons_down(buttons);
}

void age::gb_emulator::set_buttons_up(int buttons)
{
    m_impl->set_buttons_up(buttons);
}



//---------------------------------------------------------
//
//   protected methods
//
//---------------------------------------------------------

int age::gb_emulator::inner_emulate(int cycles_to_emulate)
{
    return m_impl->inner_emulate(cycles_to_emulate);
}

std::string age::gb_emulator::inner_get_emulator_title() const
{
    return m_impl->inner_get_emulator_title();
}



//---------------------------------------------------------
//
//   object creation & destruction
//
//---------------------------------------------------------

age::gb_emulator::gb_emulator(const uint8_vector& rom, gb_hardware hardware, gb_colors_hint colors_hint)
    : emulator(gb_screen_width, gb_screen_height, gb_clock_cycles_per_second / 2, gb_clock_cycles_per_second),
      m_impl(new gb_emulator_impl(rom, hardware, colors_hint, get_pcm_vector(), get_screen_buffer()))
{
}

age::gb_emulator::~gb_emulator()
{
    delete m_impl;
}
