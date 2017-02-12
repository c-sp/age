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

#include "age_gb_simulator.hpp"



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

age::uint8_vector age::gb_simulator::get_persistent_ram() const
{
    return m_memory.get_persistent_ram();
}

void age::gb_simulator::set_persistent_ram(const uint8_vector &source)
{
    m_memory.set_persistent_ram(source);
}

void age::gb_simulator::set_buttons_down(uint buttons)
{
    m_joypad.set_buttons_down(buttons);
}

void age::gb_simulator::set_buttons_up(uint buttons)
{
    m_joypad.set_buttons_up(buttons);
}



//---------------------------------------------------------
//
//   protected methods
//
//---------------------------------------------------------

age::uint64 age::gb_simulator::inner_simulate(uint64 min_ticks_to_simulate)
{
    uint starting_cycle = m_core.get_oscillation_cycle();
    uint cycle_to_go = starting_cycle + min_ticks_to_simulate;

    while (m_core.get_oscillation_cycle() < cycle_to_go)
    {
        m_bus.handle_events(); // may change the current gb_mode
        switch (m_core.get_mode())
        {
            case gb_mode::halted:
                m_core.oscillate_cpu_tick();
                break;

            case gb_mode::cpu_active:
                m_cpu.simulate_instruction();
                break;

            case gb_mode::dma:
                AGE_ASSERT(m_core.is_cgb());
                m_bus.handle_dma();
                m_core.finish_dma();
                break;
        }
    }

    m_sound.generate_samples();

    uint cycles_simulated = m_core.get_oscillation_cycle() - starting_cycle;
    return cycles_simulated;
}



std::string age::gb_simulator::inner_get_simulator_title() const
{
    return m_memory.get_cartridge_title();
}



//---------------------------------------------------------
//
//   object creation & destruction
//
//---------------------------------------------------------

age::gb_simulator::gb_simulator(const uint8_vector &rom, bool force_dmg)
    : simulator(gb_screen_width, gb_screen_height, gb_sampling_rate, gb_cycles_per_second),
      m_memory(rom, force_dmg),
      m_core(m_memory.is_cgb()),
      m_sound(m_core, get_pcm_vector()),
      m_lcd(m_core, m_memory, get_video_buffer_handler()),
      m_timer(m_core),
      m_joypad(m_core),
      m_serial(m_core),
      m_bus(m_core, m_memory, m_sound, m_lcd, m_timer, m_joypad, m_serial),
      m_cpu(m_core, m_bus)
{
}
