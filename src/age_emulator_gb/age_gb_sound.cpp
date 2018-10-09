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

constexpr int gb_apu_event_samples = 1 << 12;
constexpr int gb_frequency_sweep_check_delay = 2;

// memory dumps,
// based on *.bin files used by gambatte tests and gambatte source code
// (initstate.cpp)

constexpr const age::uint8_array<0x10> dmg_wave_ram =
{{
     0x71, 0x72, 0xD5, 0x91, 0x58, 0xBB, 0x2A, 0xFA,
     0xCF, 0x3C, 0x54, 0x75, 0x48, 0xCF, 0x8F, 0xD9
 }};

constexpr const age::uint8_array<0x10> cgb_wave_ram =
{{
     0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
     0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF
 }};

}



#define SAMPLE_COUNT(cycle) ((cycle) >> 1)
#define CORE_SAMPLE_COUNT SAMPLE_COUNT(m_core.get_oscillation_cycle())

#define ASSERT_UPDATED AGE_ASSERT(m_sample_count == CORE_SAMPLE_COUNT)





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
    if (m_c3.active())
    {
        update_state();
        if (!m_is_cgb && !m_c3.timer_reload_on_last_sample())
        {
            return 0xFF;
        }
        offset = m_c3.get_wave_pattern_index();
        offset >>= 1;
    }

    auto result = m_c3_wave_ram[offset];
    return result;
}

void age::gb_sound::write_wave_ram(unsigned offset, uint8_t value)
{
    AGE_ASSERT(offset < m_c3_wave_ram.size());

    // if channel 3 is currently active, we can only write the last accessed
    // wave sample (DMG: only if within clock range)
    if (m_c3.active())
    {
        update_state();
        if (!m_is_cgb && !m_c3.timer_reload_on_last_sample())
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
    int sample_count = CORE_SAMPLE_COUNT;

    while (sample_count >= m_sample_next_apu_event)
    {
        generate_samples(m_sample_next_apu_event);
        int samples = apu_event();
        m_sample_next_apu_event += samples;
    }

    generate_samples(sample_count);
    AGE_ASSERT(m_sample_count < m_sample_next_apu_event);
}



void age::gb_sound::set_back_cycles()
{
    // update_state() must have been called before
    // (with m_core.set_back_cycle() already having been called,
    // there is no way we can assert that)

    AGE_ASSERT(m_sample_count >= CORE_SAMPLE_COUNT);
    int sample_diff = m_sample_count - CORE_SAMPLE_COUNT;

    m_sample_count -= sample_diff;
    m_sample_next_apu_event -= sample_diff;

    AGE_ASSERT((m_sample_next_apu_event % gb_apu_event_samples) == 0);
    AGE_ASSERT(m_sample_count < m_sample_next_apu_event);
}





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

bool age::gb_sound::inc_period() const
{
    ASSERT_UPDATED;

    // the initial volume envelope period is increased by one,
    // if the next frame sequencer step 7 is near
    return
            // frame sequncer step 7 is next
            (
                (m_next_frame_sequencer_step == 7)
                && !m_skip_frame_sequencer_step
                )
            // frame sequencer step 6 is at most 2 samples away
            || (
                (m_next_frame_sequencer_step == 6)
                && !m_delayed_disable_c1
                && (m_sample_next_apu_event - m_sample_count <= 2)
                );
}



int age::gb_sound::apu_event()
{
    AGE_ASSERT((m_next_frame_sequencer_step >= 0)
               && (m_next_frame_sequencer_step <= 7));

    // no frame sequencer activity if the APU is switched off
    if (!m_master_on)
    {
        LOG("ignored at sample " << m_sample_count << ": APU off");
        return gb_apu_event_samples;
    }

    // skip this frame sequencer step
    // (triggered by switching on the APU at specific cycles)
    if (m_skip_frame_sequencer_step)
    {
        LOG("skipping step at sample " << m_sample_count);
        AGE_ASSERT(!m_delayed_disable_c1);
        AGE_ASSERT(m_next_frame_sequencer_step == 7);
        m_next_frame_sequencer_step = 0;
        m_skip_frame_sequencer_step = false;
        return gb_apu_event_samples;
    }

    // delayed disabling of channel 1 due to frequency sweep overflow
    if (m_delayed_disable_c1)
    {
        LOG("delayed disable c1 at sample " << m_sample_count);
        m_c1.deactivate();
        m_delayed_disable_c1 = false;
        return gb_apu_event_samples - gb_frequency_sweep_check_delay;
    }

    // perform next frame sequencer step
    LOG("step " << AGE_LOG_DEC(m_next_frame_sequencer_step)
        << " at sample " << m_sample_count
        << " (disable c1: " << m_delayed_disable_c1 << ")");

    int samples = gb_apu_event_samples;
    switch (m_next_frame_sequencer_step)
    {
        case 2:
        case 6:
            if (m_c1.sweep_frequency())
            {
                m_delayed_disable_c1 = true;
                samples = gb_frequency_sweep_check_delay;
            }
            // fall through
            [[clang::fallthrough]];
        case 0:
        case 4:
            length_counter_tick<gb_channel_1>();
            length_counter_tick<gb_channel_2>();
            length_counter_tick<gb_channel_3>();
            length_counter_tick<gb_channel_4>();
            // fall through
            [[clang::fallthrough]];
        case 1:
        case 3:
        case 5:
            ++m_next_frame_sequencer_step;
            break;

        case 7:
            m_c1.volume_envelope();
            m_c2.volume_envelope();
            m_c4.volume_envelope();
            m_next_frame_sequencer_step = 0;
            break;
    }

    return samples;
}



void age::gb_sound::generate_samples(int sample_count)
{
    AGE_ASSERT(sample_count >= m_sample_count);

    int samples_to_generate = sample_count - m_sample_count;
    if (samples_to_generate <= 0)
    {
        return;
    }
    m_sample_count = sample_count;

    // allocate silence
    AGE_ASSERT(m_samples.size() <= int_max);
    int sample_index = static_cast<int>(m_samples.size());

    AGE_ASSERT(int_max >= sample_index + samples_to_generate);
    m_samples.resize(static_cast<unsigned>(sample_index + samples_to_generate));

    // fill the silence, if audio is enabled
    if (m_master_on)
    {
        //! \todo find out when frequency timers are counting and when not
        if (m_c1.active())
        {
            m_c1.generate_samples(m_samples, sample_index, samples_to_generate);
        }
        if (m_c2.active())
        {
            m_c2.generate_samples(m_samples, sample_index, samples_to_generate);
        }
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

age::gb_sound::gb_sound(const gb_core &core, pcm_vector &samples)
    : m_core(core),
      m_is_cgb(m_core.is_cgb()),
      m_samples(samples),
      m_sample_count(CORE_SAMPLE_COUNT)
{
    // initialize frame sequencer
    int fs_steps = m_sample_count / gb_apu_event_samples;
    m_sample_next_apu_event = (fs_steps + 1) * gb_apu_event_samples;
    m_next_frame_sequencer_step = m_is_cgb ? 0 : 1;

    // initialize wave ram
    const uint8_array<0x10> &src = m_is_cgb ? cgb_wave_ram : dmg_wave_ram;
    std::copy(begin(src), end(src), begin(m_c3_wave_ram));

    // initialize channel 1
    m_c1.activate();
    m_c1.write_nrX2(0xF3);
    m_c1.set_wave_pattern_duty(0x80);

    // channel 1 initial state:
    // we know from gambatte test roms at which cycle
    // the duty waveform reaches position 5
    // and that each waveform step takes 126 samples
    int duty_pos5_sample = SAMPLE_COUNT(m_is_cgb ? 9500 : 44508);
    AGE_ASSERT(m_sample_count < duty_pos5_sample);

    int sample_diff = duty_pos5_sample - m_sample_count;
    int sample_offset = sample_diff % 126;
    int index = 4 - (sample_diff / 126);

    LOG("channel 1 init (cgb " << m_is_cgb << "): "
        << "sample_diff " << sample_diff
        << ", sample_offset " << sample_offset
        << ", index " << index << "(" << (index & 7) << ")"
        );
    m_c1.init_duty_waveform_position(0x7C1, sample_offset, index & 7);
}
