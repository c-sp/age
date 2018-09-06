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

#include <algorithm>

#include <age_debug.hpp>

#include "age_gb_sound.hpp"



namespace age
{

constexpr int8_t gb_frame_sequencer_cycle_shift = 13;

// memory dumps,
// based on *.bin files used by gambatte tests and gambatte source code (initstate.cpp)

constexpr const uint8_array<0x10> dmg_wave_ram_dump =
{{
     0x71, 0x72, 0xD5, 0x91, 0x58, 0xBB, 0x2A, 0xFA, 0xCF, 0x3C, 0x54, 0x75, 0x48, 0xCF, 0x8F, 0xD9
 }};

constexpr const uint8_array<0x10> cgb_wave_ram_dump =
{{
     0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF
 }};

}





//---------------------------------------------------------
//
//   Access wave pattern ram
//
//---------------------------------------------------------

age::uint8_t age::gb_sound::read_wave_ram(unsigned offset)
{
    AGE_ASSERT(offset < m_c3_wave_ram.size());

    // if channel 3 is currently active, we can only read the last accessed
    // wave sample (DMG: only if within clock range)
    if ((m_nr52 & gb_channel_bit[gb_channel_3]) > 0)
    {
        generate_samples();
        if (!m_is_cgb && (m_core.get_oscillation_cycle() != m_c3_last_wave_access_cycle))
        {
            return 0xFF;
        }
        offset = m_c3.get_wave_pattern_index();
        offset >>= 1;
    }

    uint8_t result = m_c3_wave_ram[offset];
    return result;
}

void age::gb_sound::write_wave_ram(unsigned offset, uint8_t value)
{
    AGE_ASSERT(offset < m_c3_wave_ram.size());

    // if channel 3 is currently active, we can only write the last accessed
    // wave sample (DMG: only if within clock range)
    if ((m_nr52 & gb_channel_bit[gb_channel_3]) > 0)
    {
        generate_samples();
        if (!m_is_cgb && (m_core.get_oscillation_cycle() != m_c3_last_wave_access_cycle))
        {
            return;
        }
        offset = m_c3.get_wave_pattern_index();
        offset >>= 1;
    }

    set_wave_ram_byte(offset, value);
}





//---------------------------------------------------------
//
//   other public methods
//
//---------------------------------------------------------

void age::gb_sound::frame_sequencer_cycle()
{
    // see http://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Frame_Sequencer

    switch (m_next_frame_sequencer_step)
    {
        case 2:
        case 6:
            if (m_c1.sweep_frequency())
            {
                generate_samples();
                deactivate_channel<gb_channel_1>();
            }
            // fall through
        case 0:
        case 4:
            cycle_length_counter<gb_channel_1>();
            cycle_length_counter<gb_channel_2>();
            cycle_length_counter<gb_channel_3>();
            cycle_length_counter<gb_channel_4>();
            // fall through
        case 1:
        case 3:
        case 5:
            ++m_next_frame_sequencer_step;
            break;

        case 7:
            generate_samples();
            m_c1.sweep_volume();
            m_c2.sweep_volume();
            m_c4.sweep_volume();
            m_next_frame_sequencer_step = 0;
            break;
    }

    m_next_frame_sequencer_step_odd = (m_next_frame_sequencer_step & 1) != 0;

    int current_cycle = m_core.get_oscillation_cycle();
    int cycle_offset = ((current_cycle >> gb_frame_sequencer_cycle_shift) + 1) << gb_frame_sequencer_cycle_shift;
    cycle_offset -= current_cycle;

    m_core.insert_event(cycle_offset, gb_event::sound_frame_sequencer);
}



void age::gb_sound::generate_samples()
{
    int cycles_elapsed = (m_core.get_oscillation_cycle() - m_last_generate_samples_cycle) & gb_cycle_sample_mask;
    if (cycles_elapsed > 0)
    {
        m_last_generate_samples_cycle += cycles_elapsed;

        int samples_to_generate = cycles_elapsed >> gb_sample_cycle_shift;
        AGE_ASSERT(samples_to_generate > 0);

        size_t sample_index = m_samples.size();
        m_samples.resize(sample_index + samples_to_generate);

        if (m_master_on)
        {
            m_c1.generate(m_samples, sample_index, cycles_elapsed);
            m_c2.generate(m_samples, sample_index, cycles_elapsed);

            int cycle_offset = m_c3.generate(m_samples, sample_index, cycles_elapsed);
            if (cycle_offset != -1)
            {
                AGE_ASSERT(cycle_offset <= cycles_elapsed);
                m_c3_last_wave_access_cycle = m_last_generate_samples_cycle - cycle_offset;
            }

            m_c4.generate(m_samples, sample_index, cycles_elapsed);
        }
    }
}



void age::gb_sound::set_back_cycles(int offset)
{
    AGE_GB_SET_BACK_CYCLES(m_last_generate_samples_cycle, offset);
    AGE_GB_SET_BACK_CYCLES_OVERFLOW(m_c3_last_wave_access_cycle, offset);
}





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

void age::gb_sound::set_wave_ram_byte(unsigned offset, uint8_t value)
{
    m_c3_wave_ram[offset] = value;
    m_c3.set_wave_pattern_byte(offset, value);
}





//---------------------------------------------------------
//
//   Object creation
//
//---------------------------------------------------------

age::gb_sound::gb_sound(gb_core &core, pcm_vector &samples)
    : m_core(core),
      m_is_cgb(m_core.is_cgb()),
      m_last_generate_samples_cycle(m_core.get_oscillation_cycle()),
      m_samples(samples)
{
    // initialize wave ram
    const uint8_array<0x10> &src = m_core.is_cgb() ? cgb_wave_ram_dump : dmg_wave_ram_dump;
    std::copy(begin(src), end(src), begin(m_c3_wave_ram));
    write_wave_ram(0, m_c3_wave_ram[0]);

    // perform a soft reset for initialization
    write_nr52(0x00);
    write_nr52(0xF0);

    // write initial values
    write_nr11(0xBF);
    write_nr12(0xF3);
    write_nr50(0x77);
    write_nr51(0xF3);
    m_nr52 |= gb_channel_bit[gb_channel_1];

    // schedule inital events
    m_core.insert_event(4, gb_event::sound_frame_sequencer);
}
