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

#include <algorithm>

#include <age_debug.hpp>

#include "age_gb_sound.hpp"



namespace
{
    constexpr int gb_frequency_sweep_check_delay = 4;

    // memory dumps,
    // based on *.bin files used by gambatte tests and gambatte source code
    // (initstate.cpp)

    constexpr const age::uint8_array<0x10> dmg_wave_ram
        = {{0x71, 0x72, 0xD5, 0x91, 0x58, 0xBB, 0x2A, 0xFA,
            0xCF, 0x3C, 0x54, 0x75, 0x48, 0xCF, 0x8F, 0xD9}};

    constexpr const age::uint8_array<0x10> cgb_wave_ram
        = {{0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
            0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF}};

} // namespace





//---------------------------------------------------------
//
//   public interface
//
//---------------------------------------------------------

age::uint8_t age::gb_sound::read_wave_ram(unsigned offset)
{
    AGE_ASSERT(offset < m_c3_wave_ram.size())

    // if channel 3 is currently active, we can only read the last accessed
    // wave sample (DMG: only if within clock range)
    if (m_c3.active())
    {
        update_state();
        if (!m_cgb && !m_c3.wave_ram_just_read())
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
    AGE_ASSERT(offset < m_c3_wave_ram.size())

    // if channel 3 is currently active, we can only write the last accessed
    // wave sample (DMG: only if within clock range)
    if (m_c3.active())
    {
        update_state();
        if (!m_cgb && !m_c3.wave_ram_just_read())
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
    int current_clk = m_clock.get_clock_cycle();

    // apu off?
    if (!m_master_on)
    {
        generate_samples(current_clk);
        return;
    }

    // apu on
    AGE_ASSERT(m_clk_next_apu_event != gb_no_clock_cycle);
    while (current_clk >= m_clk_next_apu_event)
    {
        generate_samples(m_clk_next_apu_event);
        int clks_next_event = apu_event();
        m_clk_next_apu_event += clks_next_event;
    }

    generate_samples(current_clk);
    AGE_ASSERT(m_clk_current_state < m_clk_next_apu_event)
}



void age::gb_sound::after_div_reset()
{
    // m_master_on cannot be changed by update_state()
    // which is why we can check it first
    if (!m_master_on)
    {
        AGE_GB_CLOG_SOUND("apu off at DIV reset => nothing to do")
        return;
    }

    update_state();
    AGE_ASSERT(m_clk_next_apu_event != gb_no_clock_cycle);
    AGE_ASSERT(m_clk_current_state < m_clk_next_apu_event)

    auto reset_details = m_div.calculate_reset_details(gb_apu_event_clock_cycles);
    AGE_ASSERT(m_clk_next_apu_event == m_clk_current_state + reset_details.m_old_next_increment);

    if (reset_details.m_clk_adjust < 0)
    {
        // immediate frame sequencer step caused by DIV reset
        apu_event();
    }
    m_clk_next_apu_event = m_clk_current_state + reset_details.m_new_next_increment;

    AGE_GB_CLOG_SOUND("apu frame sequencer at DIV reset:")
    AGE_GB_CLOG_SOUND("    * next step (old) in " << reset_details.m_old_next_increment << " clock cycles")
    AGE_GB_CLOG_SOUND("    * next step (new) in " << reset_details.m_new_next_increment << " clock cycles")
    AGE_GB_CLOG_SOUND("    * +/- clock cycles until next step: " << reset_details.m_clk_adjust)
}



void age::gb_sound::set_back_clock(int clock_cycle_offset)
{
    // update_state() must have been called before
    // (with m_clock.set_back_clock() already called there is no way we can
    // assert that though)

    gb_set_back_clock_cycle(m_clk_current_state, clock_cycle_offset);
    gb_set_back_clock_cycle(m_clk_next_apu_event, clock_cycle_offset);

    AGE_ASSERT((m_clk_next_apu_event == gb_no_clock_cycle) || (m_clk_next_apu_event > m_clk_current_state));
}





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

bool age::gb_sound::should_inc_period() const
{
    AGE_ASSERT(m_master_on);
    AGE_ASSERT(m_clk_next_apu_event != gb_no_clock_cycle);
    // everything must be up to date
    AGE_ASSERT((m_clock.get_clock_cycle() - m_clk_current_state <= 1)
               && (m_clock.get_clock_cycle() - m_clk_current_state >= 0));

    // the initial volume envelope period is increased by one,
    // if the next frame sequencer step 7 is near
    // (see test rom analysis)
    return
        // frame sequencer step 7 is next
        ((m_next_frame_sequencer_step == 7) && !m_skip_frame_sequencer_step)
        // frame sequencer step 6 is at most 4 clock cycles away
        || ((m_next_frame_sequencer_step == 6)
            && !m_delayed_disable_c1
            && (m_clk_next_apu_event - m_clk_current_state <= 4));
}

bool age::gb_sound::should_dec_length_counter() const
{
    AGE_ASSERT(m_master_on);
    AGE_ASSERT(m_clk_next_apu_event != gb_no_clock_cycle);
    // everything must be up to date
    AGE_ASSERT((m_clock.get_clock_cycle() - m_clk_current_state <= 1)
               && (m_clock.get_clock_cycle() - m_clk_current_state >= 0));

    return m_next_frame_sequencer_step & 1;
}



int age::gb_sound::apu_event()
{
    AGE_ASSERT(m_master_on);
    AGE_ASSERT(m_clk_next_apu_event != gb_no_clock_cycle);
    AGE_ASSERT((m_next_frame_sequencer_step >= 0) && (m_next_frame_sequencer_step <= 7))

    // skip this frame sequencer step
    // (triggered by switching on the APU at specific cycles)
    if (m_skip_frame_sequencer_step)
    {
        AGE_GB_CLOG_SOUND("skipping apu step at clock cycle " << m_clk_current_state)
        AGE_ASSERT(!m_delayed_disable_c1)
        AGE_ASSERT(m_next_frame_sequencer_step == 7)
        m_next_frame_sequencer_step = 0;
        m_skip_frame_sequencer_step = false;
        return gb_apu_event_clock_cycles;
    }

    // delayed disabling of channel 1 due to frequency sweep overflow
    if (m_delayed_disable_c1)
    {
        AGE_GB_CLOG_SOUND("delayed disabling c1 at clock cycle " << m_clk_current_state)
        AGE_ASSERT((m_next_frame_sequencer_step == 7) || (m_next_frame_sequencer_step == 3))
        m_c1.deactivate();
        m_delayed_disable_c1 = false;
        return gb_apu_event_clock_cycles - gb_frequency_sweep_check_delay;
    }

    // perform next frame sequencer step
    AGE_GB_CLOG_SOUND("apu step " << AGE_LOG_DEC(m_next_frame_sequencer_step)
                                  << " at clock cycle " << m_clk_current_state)

    int clks_next_apu_event = gb_apu_event_clock_cycles;
    switch (m_next_frame_sequencer_step)
    {
        case 2:
        case 6:
            if (m_c1.sweep_frequency())
            {
                // deactivation of channel 1 is delayed by 2 samples
                // (see test rom analysis)
                m_delayed_disable_c1 = true;
                clks_next_apu_event  = gb_frequency_sweep_check_delay;
            }
            // fall through
        case 0:
        case 4:
            m_c1.decrement_length_counter();
            m_c2.decrement_length_counter();
            m_c3.decrement_length_counter();
            m_c4.decrement_length_counter();
            // fall through
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

    return clks_next_apu_event;
}



void age::gb_sound::generate_samples(int for_clk)
{
    AGE_ASSERT(for_clk >= m_clk_current_state)

    int samples_to_generate = (for_clk - m_clk_current_state) / 2;
    if (samples_to_generate <= 0)
    {
        return;
    }
    m_clk_current_state += samples_to_generate * 2;

    // allocate silence
    AGE_ASSERT(m_samples.size() <= int_max)
    int sample_index = static_cast<int>(m_samples.size());

    AGE_ASSERT(int_max >= sample_index + samples_to_generate)
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

age::gb_sound::gb_sound(const gb_clock& clock,
                        const gb_div&   div,
                        bool            cgb_features,
                        pcm_vector&     samples)
    : m_samples(samples),
      m_clock(clock),
      m_div(div),
      m_cgb(cgb_features),
      m_clk_current_state(m_clock.get_clock_cycle()),
      // initialize frame sequencer
      // (see test rom analysis)
      m_clk_next_apu_event((m_clk_current_state / gb_apu_event_clock_cycles + 1) * gb_apu_event_clock_cycles),
      m_next_frame_sequencer_step(m_cgb ? 0 : 1)
{
    AGE_GB_CLOG_SOUND("next apu event at clock " << m_clk_next_apu_event)

    // initialize wave ram
    const uint8_array<0x10>& src = m_cgb ? cgb_wave_ram : dmg_wave_ram;
    std::copy(begin(src), end(src), begin(m_c3_wave_ram));

    // initialize channel 1
    m_c1.activate();
    m_c1.write_nrX2(0xF3);
    m_c1.set_duty_waveform(0x80);

    // channel 1 initial state:
    // we know from gambatte test roms at which cycle
    // the duty waveform reaches position 5
    // and that each waveform step takes 126 samples
    // (see test rom analysis)
    int duty_pos5_clk = m_cgb ? 9500 : 44508;
    AGE_ASSERT(m_clk_current_state < duty_pos5_clk)

    int duty_clks_diff   = duty_pos5_clk - m_clk_current_state;
    int duty_clks_offset = duty_clks_diff % 252;
    int duty_index       = 4 - (duty_clks_diff / 252);

    m_c1.init_duty_waveform_position(0x7C1, duty_clks_offset / 2, duty_index & 7);

    AGE_GB_CLOG_SOUND("channel 1 init (cgb " << m_cgb << "): "
                                             << "duty_clks_diff " << duty_clks_diff
                                             << ", duty_clks_offset " << duty_clks_offset
                                             << ", index " << duty_index << " (" << (duty_index & 7) << ")")
}
