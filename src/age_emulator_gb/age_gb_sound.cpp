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

#if 0
#define LOG(x) AGE_GB_CYCLE_LOG(x)
#else
#define LOG(x)
#endif



namespace
{

constexpr int gb_frame_sequencer_cycle_shift = 13;
constexpr int gb_frame_sequencer_step_cycles = 1 << gb_frame_sequencer_cycle_shift;

// memory dumps,
// based on *.bin files used by gambatte tests and gambatte source code (initstate.cpp)

constexpr const age::uint8_array<0x10> dmg_wave_ram_dump =
{{
     0x71, 0x72, 0xD5, 0x91, 0x58, 0xBB, 0x2A, 0xFA, 0xCF, 0x3C, 0x54, 0x75, 0x48, 0xCF, 0x8F, 0xD9
 }};

constexpr const age::uint8_array<0x10> cgb_wave_ram_dump =
{{
     0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF
 }};

}





//---------------------------------------------------------
//
//   public interface
//
//---------------------------------------------------------

age::uint8_t age::gb_sound::read_wave_ram(unsigned offset)
{
    AGE_ASSERT(offset < m_c3_wave_ram.size());

    // if channel 3 is currently active, we can only read the last accessed
    // wave sample (DMG: only if within clock range)
    if ((m_nr52 & gb_channel_bit[gb_channel_3]) > 0)
    {
        update_state();
        if (!m_is_cgb && !m_c3.get_last_sample_new_item())
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
        update_state();
        if (!m_is_cgb && !m_c3.get_last_sample_new_item())
        {
            return;
        }
        offset = m_c3.get_wave_pattern_index();
        offset >>= 1;
    }

    set_wave_ram_byte(offset, value);
}



void age::gb_sound::update_state()
{
    // see https://gist.github.com/drhelius/3652407

    int current_cycle = m_core.get_oscillation_cycle();

    while (current_cycle >= m_next_frame_sequencer_cycle)
    {
        frame_sequencer_step(m_next_frame_sequencer_cycle);
        m_next_frame_sequencer_cycle += gb_frame_sequencer_step_cycles;
    }

    generate_samples(current_cycle);
}



void age::gb_sound::set_back_cycles(int offset)
{
    // update_state() must have been called to make sure cycle values
    // are within range and no integer underflow is happening
    AGE_ASSERT(m_core.get_oscillation_cycle() - m_last_sample_cycle < gb_cycles_per_sample);

    AGE_GB_SET_BACK_CYCLES_OVERFLOW(m_last_sample_cycle, offset);
    AGE_GB_SET_BACK_CYCLES(m_next_frame_sequencer_cycle, offset);
}





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

void age::gb_sound::frame_sequencer_step(int at_cycle)
{
    AGE_ASSERT((m_next_frame_sequencer_step >= 0) && (m_next_frame_sequencer_step <= 7));

    switch (m_next_frame_sequencer_step)
    {
        case 2:
        case 6:
            if (m_c1.sweep_frequency())
            {
                generate_samples(at_cycle);
                deactivate_channel<gb_channel_1>();
            }
            // fall through
            [[clang::fallthrough]];
        case 0:
        case 4:
            generate_samples(at_cycle);
            LOG("length counter tick for cycle " << at_cycle);
            tick_length_counter<gb_channel_1>();
            tick_length_counter<gb_channel_2>();
            tick_length_counter<gb_channel_3>();
            tick_length_counter<gb_channel_4>();
            // fall through
            [[clang::fallthrough]];
        case 1:
        case 3:
        case 5:
            ++m_next_frame_sequencer_step;
            break;

        case 7:
            generate_samples(at_cycle);
            m_c1.sweep_volume();
            m_c2.sweep_volume();
            m_c4.sweep_volume();
            m_next_frame_sequencer_step = 0;
            break;
    }
}



void age::gb_sound::generate_samples(int until_cycle)
{
    AGE_ASSERT(!(m_last_sample_cycle & ((1 << gb_sample_cycle_shift) - 1)));
    int cycles_elapsed = until_cycle - m_last_sample_cycle;
    if (cycles_elapsed <= 0)
    {
        return;
    }

    int samples_to_generate = cycles_elapsed >> gb_sample_cycle_shift;
    if (samples_to_generate <= 0)
    {
        return;
    }
    m_last_sample_cycle += samples_to_generate << gb_sample_cycle_shift;
    AGE_ASSERT(!(m_last_sample_cycle & ((1 << gb_sample_cycle_shift) - 1)));

    // allocate silence
    AGE_ASSERT(m_samples.size() <= int_max);
    int sample_index = static_cast<int>(m_samples.size());

    AGE_ASSERT(int_max >= sample_index + samples_to_generate);
    m_samples.resize(static_cast<unsigned>(sample_index + samples_to_generate));

    // fill the silence, if audio is enabled
    if (m_master_on)
    {
        m_c1.generate_samples(m_samples, sample_index, samples_to_generate);
        m_c2.generate_samples(m_samples, sample_index, samples_to_generate);
        m_c3.generate_samples(m_samples, sample_index, samples_to_generate);
        m_c4.generate_samples(m_samples, sample_index, samples_to_generate);
    }
}



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
      m_samples(samples),
      m_last_sample_cycle(m_core.get_oscillation_cycle())
{
    // no pcm samples are generated by this initialization due to
    // m_last_sample_cycle being set to the current cycle

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
}
