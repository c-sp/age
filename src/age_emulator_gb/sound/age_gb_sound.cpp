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

#include <age_debug.hpp>

#include "age_gb_sound.hpp"

#include <algorithm>



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
        if (!m_device.cgb_mode() && !m_c3.wave_ram_just_read())
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
        if (!m_device.cgb_mode() && !m_c3.wave_ram_just_read())
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
    AGE_ASSERT(m_clk_next_apu_event != gb_no_clock_cycle)
    while (current_clk >= m_clk_next_apu_event)
    {
        generate_samples(m_clk_next_apu_event);
        int clks_next_event = apu_event();
        m_clk_next_apu_event += clks_next_event;
    }

    generate_samples(current_clk);
    AGE_ASSERT(m_clk_current_state < m_clk_next_apu_event)
}



void age::gb_sound::after_div_reset(bool during_stop)
{
    if (!m_master_on)
    {
        log() << "sound off at DIV reset";
        return;
    }
    // everything must be up to date
    AGE_ASSERT((m_clock.get_clock_cycle() - m_clk_current_state <= 1)
               && (m_clock.get_clock_cycle() - m_clk_current_state >= 0));

    AGE_ASSERT(m_clk_next_apu_event != gb_no_clock_cycle)
    AGE_ASSERT(m_clk_current_state < m_clk_next_apu_event)

    auto reset_details = m_clock.get_div_reset_details(gb_apu_event_clock_cycles);
    AGE_ASSERT((m_clk_next_apu_event == m_clk_current_state + reset_details.m_old_next_increment)
               || (m_clk_next_apu_event - 2 == m_clk_current_state + reset_details.m_old_next_increment));

    if (!during_stop)
    {
        reset_details.m_old_next_increment += m_current_ds_delay;
        reset_details.m_new_next_increment += m_current_ds_delay;
    }

    auto msg = log();
    msg << "frame sequencer at DIV reset:";

    // speed change sound STOP glitch (see Gambatte speedchange_ch2_nr52_* tests):
    // no immediate action by div reset on the exact first machine cycle
    //! \todo we only checked the 0->1 edge, check the 1->0 edge too
    if (during_stop && (reset_details.m_clk_adjust == -gb_apu_event_clock_cycles / 2))
    {
        msg << "\n    * sound STOP glitch: immediate frame sequencer step by DIV reset not on this machine cycle";
        AGE_ASSERT(reset_details.m_new_next_increment == -reset_details.m_clk_adjust * 2)
        reset_details.m_clk_adjust = -reset_details.m_clk_adjust;
    }

    msg << "\n    * next step (old) in " << log_in_clks(reset_details.m_old_next_increment, m_clk_current_state)
        << "\n    * next step (new) in " << log_in_clks(reset_details.m_new_next_increment, m_clk_current_state)
        << "\n    * +/- clock cycles until next step: " << reset_details.m_clk_adjust;

    // immediate frame sequencer step caused by DIV reset?
    if (reset_details.m_clk_adjust < 0)
    {
        apu_event();
    }
    m_clk_next_apu_event = m_clk_current_state + reset_details.m_new_next_increment;
    AGE_ASSERT(m_clk_current_state < m_clk_next_apu_event)
}



void age::gb_sound::after_speed_change()
{
    if (!m_master_on)
    {
        log() << "sound off at speed change";
        return;
    }
    AGE_ASSERT(m_clk_next_apu_event != gb_no_clock_cycle)
    // everything must be up to date
    AGE_ASSERT((m_clock.get_clock_cycle() - m_clk_current_state <= 1)
               && (m_clock.get_clock_cycle() - m_clk_current_state >= 0));

    auto msg = log();
    msg << "sound at speed change:";

    if (m_clock.is_double_speed())
    {
        if (m_current_ds_delay == 0)
        {
            m_current_ds_delay = 2;
            msg << "\n    * delaying frame sequencer by 2 clock cycles after switching to double speed";
            m_clk_next_apu_event += 2;
        }
        else
        {
            m_current_ds_delay = 0;
            msg << "\n    * no frame sequencer delay after switching to double speed";
        }
    }
    else if (m_current_ds_delay == 0)
    {
        if (m_c1.active())
        {
            m_c1.delay_one_sample();
            msg << "\n    * delaying channel 1 frequency timer by one sample";
        }
        if (m_c2.active())
        {
            m_c2.delay_one_sample();
            msg << "\n    * delaying channel 2 frequency timer by one sample";
        }
        if (m_c3.active())
        {
            m_c3.delay_one_sample();
            msg << "\n    * delaying channel 3 frequency timer by one sample";
        }
        if (m_c4.active())
        {
            m_c4.delay_one_sample();
            msg << "\n    * delaying channel 4 frequency timer by one sample";
        }
    }
}



void age::gb_sound::set_back_clock(int clock_cycle_offset)
{
    // update_state() must have been called before
    // (with m_clock.set_back_clock() already called there is no way we can
    // assert that though)

    gb_set_back_clock_cycle(m_clk_current_state, clock_cycle_offset);
    gb_set_back_clock_cycle(m_clk_next_apu_event, clock_cycle_offset);

    AGE_ASSERT((m_clk_next_apu_event == gb_no_clock_cycle) || (m_clk_next_apu_event > m_clk_current_state))
}





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

bool age::gb_sound::should_inc_period() const
{
    AGE_ASSERT(m_master_on)
    AGE_ASSERT(m_clk_next_apu_event != gb_no_clock_cycle)
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
    AGE_ASSERT(m_master_on)
    AGE_ASSERT(m_clk_next_apu_event != gb_no_clock_cycle)
    // everything must be up to date
    AGE_ASSERT((m_clock.get_clock_cycle() - m_clk_current_state <= 1)
               && (m_clock.get_clock_cycle() - m_clk_current_state >= 0));

    return (m_next_frame_sequencer_step & 1) != 0;
}

bool age::gb_sound::should_align_frequency_timer() const
{
    AGE_ASSERT(m_master_on)
    AGE_ASSERT(m_clk_next_apu_event != gb_no_clock_cycle)
    // everything must be up to date
    AGE_ASSERT((m_clock.get_clock_cycle() - m_clk_current_state <= 1)
               && (m_clock.get_clock_cycle() - m_clk_current_state >= 0));

    //! \todo this needs more test roms (find them or write them)
    //
    // TL;DR:
    //      - during double speed the a (duty?) frequency timer is initially delayed
    //        by one sample on channel init,
    //        if bit 1 (0x02) is set in the current div
    //
    //      - when switching to single speed,
    //        running frequency timers are delayed by one sample
    //        if the double speed delay was NOT active
    //
    // frq-tm  div&2   div&2   audio1  delay
    // (spch*) (spch*) (init)  (clks)  (clks)  test
    // ------- ------- ------  ------  ------  ----
    //                                         sound off & on -> c1 init
    //                      0    1800  5-8       sound/ch1_duty0_pos6_to_pos7_timing_1/2  (128 sample period)
    //
    //                                         sound off & on -> double speed -> c1 init
    //                      0     902  5-6       speedchange_ch1_nr4init_duty0_pos6_to_pos7_timing_1/2  (64 sample period)
    //
    //                                         double speed -> sound off & on -> c1 init
    //                      2     904  7-8       sound/ch1_duty0_pos6_to_pos7_timing_ds_1/2  (64 sample period) init 60 clks after on
    //                      2     904  7-8       sound/ch1_duty0_pos6_to_pos7_timing_ds_3/4  (64 sample period) init 60 clks after on
    //                      0     902  5-6       sound/ch1_duty0_pos6_to_pos7_timing_ds_5/6  (64 sample period) init 62 clks after on
    //
    //                                         double speed -> sound off & on -> c1 init -> single speed(*)
    //     115      0       0  132872  5- 8      speedchange_ch1_duty0_pos6_to_pos7_timing_ds_1/2      (128 sample period)
    //     114      2       0  132874  7-10      speedchange_ch1_duty0_pos6_to_pos7_timing_nop_ds_1/2  (128 sample period)
    //
    //                                         double speed -> sound off & on -> c1 init -> single speed(*) -> double speed
    //      59      0       0  197512  7-8       speedchange2_ch1_duty0_pos6_to_pos7_timing_ds_1/2      (64 sample period)
    //      59      0       0  197512  7-8       speedchange2_ch1_duty0_pos6_to_pos7_timing_nop_ds_1/2  (64 sample period)
    //      58      2       0  197512  7-8       speedchange2_nop_ch1_duty0_pos6_to_pos7_timing_ds_1/2  (64 sample period)
    //
    //                                         sound off & on -> c1 init -> double speed
    //                      0   66438  5-6       speedchange_ch1_duty0_pos6_to_pos7_timing_1/2      (64 sample period)
    //                      0   66438  5-6       speedchange_ch1_duty0_pos6_to_pos7_timing_nop_1/2  (64 sample period)
    //
    //                                         sound off & on -> c1 init -> double speed -> single speed(*)
    //              0       0  198408  5-8       speedchange2_ch1_duty0_pos6_to_pos7_timing_1/2      (128 sample period)
    //              2       0  198406  3-6       speedchange2_ch1_duty0_pos6_to_pos7_timing_nop_1/2  (128 sample period)
    //
    //                                         sound off & on -> c1 init -> double speed -> single speed(*) -> double speed
    //              0       0  263046  5-6       speedchange3_ch1_duty0_pos6_to_pos7_timing_1/2      (64 sample period)
    //              0       0  263046  5-6       speedchange3_ch1_duty0_pos6_to_pos7_timing_nop_1/2  (64 sample period)
    //              2       0  263046  5-6       speedchange3_nop_ch1_duty0_pos6_to_pos7_timing_1/2  (64 sample period)
    //
    //                                         sound off & on -> c1 init -> double speed -> single speed(*) -> double speed -> single speed(*)
    //            0/0       0  395016  5- 8      speedchange4_ch1_duty0_pos6_to_pos7_timing_1/2      (128 sample period)
    //            0/2       0  395018  7-10      speedchange4_ch1_duty0_pos6_to_pos7_timing_nop_1/2  (128 sample period)
    //
    //                                         sound off & on -> c1 init -> double speed -> single speed(*) -> double speed -> single speed(*) -> double speed
    //            0/0       0  459656  7-8       speedchange5_ch1_duty0_pos6_to_pos7_timing_1/2      (64 sample period)
    //            0/0       0  459656  7-8       speedchange5_ch1_duty0_pos6_to_pos7_timing_nop_1/2  (64 sample period)
    //            0/2       0  459656  7-8       speedchange5_nop_ch1_duty0_pos6_to_pos7_timing_1/2  (64 sample period)

    // CGB-E: align frequency timer to 1mhz
    if (m_device.is_cgb_e_device())
    {
        return (((m_clk_bits_apu_on ^ m_clk_current_state) & 2) != 0);
    }

    return m_clock.is_double_speed() && (((m_clk_current_state + m_clock.get_div_offset()) & 2) != 0);
}



int age::gb_sound::apu_event()
{
    AGE_ASSERT(m_master_on)
    AGE_ASSERT(m_clk_next_apu_event != gb_no_clock_cycle)
    AGE_ASSERT((m_next_frame_sequencer_step >= 0) && (m_next_frame_sequencer_step <= 7))

    // skip this frame sequencer step
    // (triggered by switching on the APU at specific cycles)
    if (m_skip_frame_sequencer_step)
    {
        log() << "skipping frame sequencer step " << log_dec(m_next_frame_sequencer_step);
        AGE_ASSERT(!m_delayed_disable_c1)
        AGE_ASSERT(m_next_frame_sequencer_step == 7)
        m_next_frame_sequencer_step = 0;
        m_skip_frame_sequencer_step = false;
        return gb_apu_event_clock_cycles;
    }

    // delayed disabling of channel 1 due to frequency sweep overflow
    if (m_delayed_disable_c1)
    {
        log() << "delayed deactivation of channel 1 triggered by frequency sweep";
        AGE_ASSERT((m_next_frame_sequencer_step == 7) || (m_next_frame_sequencer_step == 3))
        m_c1.deactivate();
        m_delayed_disable_c1 = false;
        return gb_apu_event_clock_cycles - gb_frequency_sweep_check_delay;
    }

    // execute next frame sequencer step
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
        if (m_c1.active())
        {
            m_c1.generate_samples(m_samples, sample_index, samples_to_generate);
        }
        if (m_c2.active())
        {
            m_c2.generate_samples(m_samples, sample_index, samples_to_generate);
        }
        // if (m_c3.active()) //! \todo uncommenting this causes blargg/dmg-sound to fail
        {
            m_c3.generate_samples(m_samples, sample_index, samples_to_generate);
        }
        if (m_c4.active())
        {
            m_c4.generate_samples(m_samples, sample_index, samples_to_generate);
        }
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

age::gb_sound::gb_sound(const gb_device& device,
                        const gb_clock&  clock,
                        pcm_vector&      samples)
    : gb_sound_logger(device, clock, clock.get_clock_cycle()),
      m_samples(samples),
      m_device(device),
      // initialize frame sequencer
      // (see test rom analysis)
      m_clk_next_apu_event((m_clk_current_state / gb_apu_event_clock_cycles + 1) * gb_apu_event_clock_cycles),
      m_next_frame_sequencer_step(m_device.cgb_mode() ? 0 : 1)
{
    log() << "first frame sequencer step (" << log_dec(m_next_frame_sequencer_step)
          << ") at clock cycle " << m_clk_next_apu_event;

    // initialize wave ram
    const uint8_array<0x10>& src = m_device.cgb_mode() ? cgb_wave_ram : dmg_wave_ram;
    std::copy(begin(src), end(src), begin(m_c3_wave_ram));

    // initialize channel 1
    m_c1.activate();
    m_c1.write_nrX2(0xF3);
    m_c1.set_waveform_duty(0x80);

    // channel 1 initial state:
    // we know from gambatte test roms at which cycle
    // the duty waveform reaches position 5
    // and that each waveform step takes 126 samples
    // (see test rom analysis)
    int duty_pos5_clk = m_device.cgb_mode() ? 9500 : 44508;
    AGE_ASSERT(m_clk_current_state < duty_pos5_clk)

    int duty_clks_diff   = duty_pos5_clk - m_clk_current_state;
    int duty_clks_offset = duty_clks_diff % 252;
    int duty_index       = 4 - (duty_clks_diff / 252);

    m_c1.init_waveform_duty_position(0x7C1, duty_clks_offset / 2, duty_index & 7);
}
