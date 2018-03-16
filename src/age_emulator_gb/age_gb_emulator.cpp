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

#include <emulator/age_gb_emulator.hpp>

#include "age_gb_emulator_impl.hpp"

namespace age {

constexpr uint gb_sampling_rate = gb_machine_cycles_per_second >> gb_sample_cycle_shift;

}



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

bool age::gb_emulator::is_cgb() const
{
    return m_impl->is_cgb();
}

age::gb_test_info age::gb_emulator::get_test_info() const
{
    return m_impl->get_test_info();
}

age::uint8_vector age::gb_emulator::get_persistent_ram() const
{
    return m_impl->get_persistent_ram();
}

void age::gb_emulator::set_persistent_ram(const uint8_vector &source)
{
    m_impl->set_persistent_ram(source);
}

void age::gb_emulator::set_buttons_down(uint buttons)
{
    m_impl->set_buttons_down(buttons);
}

void age::gb_emulator::set_buttons_up(uint buttons)
{
    m_impl->set_buttons_up(buttons);
}



//---------------------------------------------------------
//
//   protected methods
//
//---------------------------------------------------------

age::uint64 age::gb_emulator::inner_emulate(uint64 min_cycles_to_emulate)
{
    return m_impl->inner_emulate(min_cycles_to_emulate);
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

age::gb_emulator::gb_emulator(const uint8_vector &rom, bool force_dmg, bool dmg_green)
    : emulator(gb_screen_width, gb_screen_height, gb_sampling_rate, gb_machine_cycles_per_second),
      m_impl(new gb_emulator_impl(rom, force_dmg, dmg_green, get_pcm_vector(), get_screen_buffer()))
{
}

age::gb_emulator::~gb_emulator()
{
    if (m_impl != nullptr)
    {
        delete m_impl;
        m_impl = nullptr;
    }
}
