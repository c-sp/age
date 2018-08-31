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

#include <limits>

#include "age_gb_emulator_impl.hpp"



age::gb_test_info age::gb_emulator_impl::get_test_info() const
{
    return m_cpu.get_test_info();
}

age::uint8_vector age::gb_emulator_impl::get_persistent_ram() const
{
    return m_memory.get_persistent_ram();
}

void age::gb_emulator_impl::set_persistent_ram(const uint8_vector &source)
{
    m_memory.set_persistent_ram(source);
}

void age::gb_emulator_impl::set_buttons_down(int buttons)
{
    m_joypad.set_buttons_down(buttons);
}

void age::gb_emulator_impl::set_buttons_up(int buttons)
{
    m_joypad.set_buttons_up(buttons);
}



age::uint64 age::gb_emulator_impl::inner_emulate(uint64 min_cycles_to_emulate)
{
    constexpr int32_t cycle_limit = std::numeric_limits<int32_t>::max();

    int32_t starting_cycle = m_core.get_oscillation_cycle();
    AGE_ASSERT(min_cycles_to_emulate <= static_cast<uint64>(cycle_limit - starting_cycle));

    int32_t cycle_to_go = starting_cycle + static_cast<int32_t>(min_cycles_to_emulate);
    while (m_core.get_oscillation_cycle() < cycle_to_go)
    {
        m_bus.handle_events(); // may change the current gb_state
        switch (m_core.get_state())
        {
            case gb_state::halted:
                m_core.oscillate_cpu_cycle();
                break;

            case gb_state::cpu_active:
                m_cpu.emulate_instruction();
                break;

            case gb_state::dma:
                AGE_ASSERT(m_core.is_cgb());
                m_bus.handle_dma();
                m_core.finish_dma();
                break;
        }
    }

    // make sure audio output is complete
    m_sound.generate_samples();

    int32_t cycles_emulated = m_core.get_oscillation_cycle() - starting_cycle;
    AGE_ASSERT(cycles_emulated >= 0);

    if (m_core.get_oscillation_cycle() > (cycle_limit / 2))
    {
        int32_t cycles_to_keep = m_core.get_oscillation_cycle() % gb_machine_cycles_per_second;
        int32_t offset = m_core.get_oscillation_cycle() - cycles_to_keep;

        AGE_ASSERT(offset > 0);
        AGE_ASSERT(offset < m_core.get_oscillation_cycle());

        m_core.set_back_cycles(offset);
        m_sound.set_back_cycles(offset);
        m_lcd.set_back_cycles(offset);
        m_timer.set_back_cycles(offset);
        m_serial.set_back_cycles(offset);
        m_bus.set_back_cycles(offset);
    }

    return static_cast<uint64>(cycles_emulated);
}



std::string age::gb_emulator_impl::inner_get_emulator_title() const
{
    return m_memory.get_cartridge_title();
}



//---------------------------------------------------------
//
//   object creation & destruction
//
//---------------------------------------------------------

age::gb_emulator_impl::gb_emulator_impl(const uint8_vector &rom,
                                        gb_hardware hardware,
                                        bool dmg_green,
                                        pcm_vector &pcm_vec,
                                        screen_buffer &screen_buf)
    : m_memory(rom, hardware),
      m_core(m_memory.get_mode()),
      m_sound(m_core, pcm_vec),
      m_lcd(m_core, m_memory, screen_buf, dmg_green),
      m_timer(m_core),
      m_joypad(m_core),
      m_serial(m_core),
      m_bus(m_core, m_memory, m_sound, m_lcd, m_timer, m_joypad, m_serial),
      m_cpu(m_core, m_bus)
{
}
