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

#include "age_gb_emulator_impl.hpp"

#if 0
#define LOG(x) AGE_GB_CLOCK_LOG(x)
#else
#define LOG(x)
#endif



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



int age::gb_emulator_impl::inner_emulate(int cycles_to_emulate)
{
    AGE_ASSERT(cycles_to_emulate > 0);

    // make sure we have some headroom since we usually emulate
    // a few more cycles than requested
    constexpr int cycle_limit = int_max - gb_clock_cycles_per_second;
    constexpr int cycle_setback_limit = 2 * gb_clock_cycles_per_second;

    // calculate the number of cycles to emulate based on the current
    // cycle and the cycle limit
    int starting_cycle = m_clock.get_clock_cycle();
    AGE_ASSERT(starting_cycle < cycle_setback_limit);

    int cycle_to_go = starting_cycle + std::min(cycles_to_emulate, cycle_limit - starting_cycle);

    // emulate until we reach the calculated cycle
    // (depending on CPU instruction length or running DMA
    // we usually emulate a little bit past that cycle)
    while (m_clock.get_clock_cycle() < cycle_to_go)
    {
        m_bus.handle_events(); // may change the current gb_state
        switch (m_core.get_state())
        {
            case gb_state::halted:
                m_clock.tick_machine_cycle();
                break;

            case gb_state::cpu_active:
                m_cpu.emulate_instruction();
                break;

            case gb_state::dma:
                AGE_ASSERT(m_device.is_cgb());
                m_bus.handle_dma();
                m_core.finish_dma();
                break;
        }
    }

    // make sure audio output is complete
    m_sound.update_state();

    // calculate the cycles actually emulated
    int current_cycle = m_clock.get_clock_cycle();
    int cycles_emulated = current_cycle - starting_cycle;
    AGE_ASSERT(cycles_emulated >= 0);

    // if the cycle counter reaches a certain threshold,
    // set back all stored cycle values to keep the
    // cycle counter from overflowing
    if (current_cycle >= cycle_setback_limit)
    {
        AGE_ASSERT(cycle_setback_limit >= 2 * gb_clock_cycles_per_second);

        // keep a minimum of cycles to prevent negative cycle values
        // (which should still work but is kind of unintuitive)
        int cycles_to_keep = gb_clock_cycles_per_second
                + (current_cycle % gb_clock_cycles_per_second);

        int clock_cycle_offset = current_cycle - cycles_to_keep;
        AGE_ASSERT(clock_cycle_offset > 0);
        AGE_ASSERT(clock_cycle_offset < current_cycle);

        LOG("set back cycles: " << current_cycle
            << " -> " << cycles_to_keep
            << " (-" << offset << ")");

        m_clock.set_back_clock(clock_cycle_offset);
        m_core.set_back_clock(clock_cycle_offset);
        m_sound.set_back_clock();
        m_lcd.set_back_clock(clock_cycle_offset);
        m_timer.set_back_clock(clock_cycle_offset);
        m_serial.set_back_clock(clock_cycle_offset);
        m_bus.set_back_clock(clock_cycle_offset);
    }

    return cycles_emulated;
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
    : m_memory(rom),
      m_device(m_memory.read_byte(gb_cia_ofs_cgb), hardware),
      m_clock(m_device),
      m_core(m_device, m_clock),
      m_sound(m_clock, m_device.is_cgb(), pcm_vec),
      m_lcd(m_device, m_clock, m_core, m_memory, screen_buf, dmg_green),
      m_timer(m_clock, m_core),
      m_joypad(m_device, m_core),
      m_serial(m_device, m_clock, m_core),
      m_bus(m_device, m_clock, m_core, m_memory, m_sound, m_lcd, m_timer, m_joypad, m_serial),
      m_cpu(m_device, m_clock, m_core, m_bus)
{
    m_memory.init_vram(m_device.is_cgb_hardware());
}
