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

#include "age_gb_sound.hpp"



namespace
{
    constexpr age::uint8_t gb_sound_master_switch = 0x80;
}



age::uint8_t age::gb_sound::read_pcm12()
{
    update_state();

    uint8_t c1_sample = m_c1.active() ? m_c1.get_current_pcm_amplitude() : 0;
    uint8_t c2_sample = m_c2.active() ? m_c2.get_current_pcm_amplitude() << 4 : 0;

    log() << "read PCM12 == " << log_hex8(c1_sample + c2_sample);
    return c1_sample + c2_sample;
}

age::uint8_t age::gb_sound::read_pcm34()
{
    update_state();

    uint8_t c3_sample = m_c3.active() ? m_c3.get_current_pcm_amplitude() : 0;
    uint8_t c4_sample = m_c4.active() ? m_c4.get_current_pcm_amplitude() << 4 : 0;

    log() << "read PCM34 == " << log_hex8(c3_sample + c4_sample);
    return c3_sample + c4_sample;
}





//---------------------------------------------------------
//
//   control ports
//
//---------------------------------------------------------

age::uint8_t age::gb_sound::read_nr50() const
{
    log() << "read NR50 == " << log_hex8(m_nr50);
    return m_nr50;
}

age::uint8_t age::gb_sound::read_nr51() const
{
    log() << "read NR51 == " << log_hex8(m_nr51);
    return m_nr51;
}

age::uint8_t age::gb_sound::read_nr52()
{
    update_state();

    uint8_t result = m_master_on ? gb_sound_master_switch : 0;
    result += 0x70;

    result += m_c1.active() ? 1 : 0;
    result += m_c2.active() ? 2 : 0;
    result += m_c3.active() ? 4 : 0;
    result += m_c4.active() ? 8 : 0;

    log() << "read NR52 == " << log_hex8(result);
    return result;
}



void age::gb_sound::write_nr50(uint8_t value)
{
    if (!m_master_on)
    {
        log() << "write NR50 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    log() << "write NR50 = " << log_hex8(value); // log after state update

    m_nr50 = value; // SO1 SO2 volume

    m_c1.set_multiplier(m_nr50, m_nr51);
    m_c2.set_multiplier(m_nr50, m_nr51 >> 1);
    m_c3.set_multiplier(m_nr50, m_nr51 >> 2);
    m_c4.set_multiplier(m_nr50, m_nr51 >> 3);
}

void age::gb_sound::write_nr51(uint8_t value)
{
    if (!m_master_on)
    {
        log() << "write NR51 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    log() << "write NR51 = " << log_hex8(value); // log after state update

    m_nr51 = value; // C1..4 to SO1 SO2 "routing"

    m_c1.set_multiplier(m_nr50, m_nr51);
    m_c2.set_multiplier(m_nr50, m_nr51 >> 1);
    m_c3.set_multiplier(m_nr50, m_nr51 >> 2);
    m_c4.set_multiplier(m_nr50, m_nr51 >> 3);
}

void age::gb_sound::write_nr52(uint8_t value)
{
    bool new_master_on = (value & gb_sound_master_switch) != 0;

    // sound switched off
    if (m_master_on && !new_master_on)
    {
        update_state();
        log() << "write NR52 = " << log_hex8(value) << ": all sound off"; // log after state update

        m_clk_next_apu_event = gb_no_clock_cycle;

        m_nr10 = m_nr11 = m_nr14 = 0;
        m_nr21 = m_nr24 = 0;
        m_nr30 = m_nr32 = m_nr34 = 0;
        m_nr44                   = 0;
        m_nr50 = m_nr51    = 0;
        m_current_ds_delay = 0;

        m_c1 = gb_sound_channel1(0x3F, this);
        m_c2 = gb_sound_channel2(0x3F, this);
        m_c3 = gb_sound_channel3(0xFF, this);
        m_c4 = gb_sound_channel4(0x3F, this);
    }

    // sound switched on
    else if (!m_master_on && new_master_on)
    {
        update_state();

        // calculate the number of clock cycles until the first frame sequencer step
        int clk_div_aligned = m_clk_current_state + m_clock.get_div_offset();
        int clks_into_step  = clk_div_aligned & (gb_apu_event_clock_cycles - 1);
        int clks_first_step = gb_apu_event_clock_cycles - clks_into_step;

        m_clk_bits_apu_on           = m_clk_current_state & 0xFFFF;
        m_clk_next_apu_event        = m_clk_current_state + clks_first_step;
        m_next_frame_sequencer_step = 0;
        m_delayed_disable_c1        = false;
        m_skip_frame_sequencer_step = false;

        log() << "write NR52 = " << log_hex8(value) << ": activating sound" // log after state update
              << "\n    * first frame sequencer step in " << clks_first_step << " clock cycles"
              << " (on clock cycle " << m_clk_next_apu_event << ")";

        // delay frame sequencer step 0
        // (see test rom analysis)
        int ofs = m_clock.is_double_speed() ? 2 : 4;
        if ((clk_div_aligned + ofs) & (gb_apu_event_clock_cycles / 2))
        {
            m_next_frame_sequencer_step = 7;
            m_skip_frame_sequencer_step = true;
        }
    }
    else
    {
        log() << "write NR52 = " << log_hex8(value) << ": no effect";
    }

    m_master_on = new_master_on;
}





//---------------------------------------------------------
//
//   channel 1 ports
//
//---------------------------------------------------------

age::uint8_t age::gb_sound::read_nr10() const
{
    uint8_t result = m_nr10 | 0x80;
    m_c1.log() << "read NR10 == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_sound::read_nr11() const
{
    uint8_t result = m_nr11 | 0x3F;
    m_c1.log() << "read NR11 == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_sound::read_nr12() const
{
    uint8_t result = m_c1.read_nrX2();
    m_c1.log() << "read NR12 == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_sound::read_nr14() const
{
    uint8_t result = m_nr14 | 0xBF;
    m_c1.log() << "read NR14 == " << log_hex8(result);
    return result;
}



void age::gb_sound::write_nr10(uint8_t value)
{
    if (!m_master_on)
    {
        m_c1.log() << "write NR10 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c1.log() << "write NR10 = " << log_hex8(value); // log after state update

    m_nr10 = value;
    m_c1.write_nrX0(value);
}

void age::gb_sound::write_nr11(uint8_t value)
{
    if (!m_master_on && m_cgb)
    {
        m_c1.log() << "write NR11 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c1.log() << "write NR11 = " << log_hex8(value); // log after state update

    // length counter always writable for DMG
    m_c1.write_nrX1(value);

    // wave pattern duty only changeable, if switched on
    if (m_master_on)
    {
        m_c1.set_waveform_duty(value);
        m_nr11 = value;
    }
}

void age::gb_sound::write_nr12(uint8_t value)
{
    if (!m_master_on)
    {
        m_c1.log() << "write NR12 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c1.log() << "write NR12 = " << log_hex8(value); // log after state update

    m_c1.write_nrX2(value);
}

void age::gb_sound::write_nr13(uint8_t value)
{
    if (!m_master_on)
    {
        m_c1.log() << "write NR13 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c1.log() << "write NR13 = " << log_hex8(value); // log after state update

    m_c1.set_low_frequency_bits(value);
}

void age::gb_sound::write_nr14(uint8_t value)
{
    if (!m_master_on)
    {
        m_c1.log() << "write NR14 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c1.log() << "write NR14 = " << log_hex8(value); // log after state update

    m_nr14 = value;

    m_c1.set_high_frequency_bits(value);
    m_c1.init_length_counter(value, should_dec_length_counter());

    if ((value & gb_nrX4_initialize) > 0)
    {
        m_c1.init_frequency_timer(should_align_frequency_timer());

        // one frequency sweep step is skipped if the next frame sequencer
        // step 2 or 6 is near
        // (see test rom analysis)
        AGE_ASSERT(m_clk_next_apu_event != gb_no_clock_cycle);
        int  clks_next_event = m_clk_next_apu_event - m_clk_current_state; // were updated by update_state()
        bool skip_sweep_step = (m_next_frame_sequencer_step & 2)
                               && (clks_next_event <= (m_cgb ? 8 : 4));

        bool deactivated = m_c1.init_frequency_sweep(skip_sweep_step);
        deactivated |= m_c1.init_volume_envelope(should_inc_period());

        if (!deactivated)
        {
            m_c1.activate();
        }
    }
}





//---------------------------------------------------------
//
//   Write channel 2 ports
//
//---------------------------------------------------------

age::uint8_t age::gb_sound::read_nr21() const
{
    uint8_t result = m_nr21 | 0x3F;
    m_c2.log() << "read NR21 == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_sound::read_nr22() const
{
    uint8_t result = m_c2.read_nrX2();
    m_c2.log() << "read NR22 == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_sound::read_nr24() const
{
    uint8_t result = m_nr24 | 0xBF;
    m_c2.log() << "read NR24 == " << log_hex8(result);
    return result;
}




void age::gb_sound::write_nr21(uint8_t value)
{
    if (!m_master_on && m_cgb)
    {
        m_c2.log() << "write NR21 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c2.log() << "write NR21 = " << log_hex8(value); // log after state update

    // length counter always writable for DMG
    m_c2.write_nrX1(value);

    // wave pattern duty only changeable, if switched on
    if (m_master_on)
    {
        m_c2.set_waveform_duty(value);
        m_nr21 = value;
    }
}

void age::gb_sound::write_nr22(uint8_t value)
{
    if (!m_master_on)
    {
        m_c2.log() << "write NR22 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c2.log() << "write NR22 = " << log_hex8(value); // log after state update

    m_c2.write_nrX2(value);
}

void age::gb_sound::write_nr23(uint8_t value)
{
    if (!m_master_on)
    {
        m_c2.log() << "write NR23 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c2.log() << "write NR23 = " << log_hex8(value); // log after state update

    m_c2.set_low_frequency_bits(value);
}

void age::gb_sound::write_nr24(uint8_t value)
{
    if (!m_master_on)
    {
        m_c2.log() << "write NR24 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c2.log() << "write NR24 = " << log_hex8(value); // log after state update

    m_c2.set_high_frequency_bits(value);
    m_c2.init_length_counter(value, should_dec_length_counter());

    if ((value & gb_nrX4_initialize) > 0)
    {
        m_c2.init_frequency_timer(should_align_frequency_timer());

        bool deactivated = m_c2.init_volume_envelope(should_inc_period());
        if (!deactivated)
        {
            m_c2.activate();
        }
    }

    m_nr24 = value;
}





//---------------------------------------------------------
//
//   Write channel 3 ports
//
//---------------------------------------------------------

age::uint8_t age::gb_sound::read_nr30() const
{
    uint8_t result = m_nr30 | 0x7F;
    m_c3.log() << "read NR30 == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_sound::read_nr32() const
{
    uint8_t result = m_nr32 | 0x9F;
    m_c3.log() << "read NR32 == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_sound::read_nr34() const
{
    uint8_t result = m_nr34 | 0xBF;
    m_c3.log() << "read NR34 == " << log_hex8(result);
    return result;
}




void age::gb_sound::write_nr30(uint8_t value)
{
    if (!m_master_on)
    {
        m_c3.log() << "write NR30 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c3.log() << "write NR30 = " << log_hex8(value); // log after state update

    m_nr30 = value;
    if ((m_nr30 & gb_sound_master_switch) == 0)
    {
        m_c3.deactivate();
    }
}

void age::gb_sound::write_nr31(uint8_t value)
{
    if (!m_master_on && m_cgb)
    {
        m_c3.log() << "write NR31 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c3.log() << "write NR31 = " << log_hex8(value); // log after state update

    // length counter always writable for DMG
    m_c3.write_nrX1(value);
}

void age::gb_sound::write_nr32(uint8_t value)
{
    if (!m_master_on)
    {
        m_c3.log() << "write NR32 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c3.log() << "write NR32 = " << log_hex8(value); // log after state update

    m_nr32 = value;
    switch (m_nr32 & 0x60)
    {
        case 0x00: m_c3.set_volume_shift(4); break;
        case 0x20: m_c3.set_volume_shift(0); break;
        case 0x40: m_c3.set_volume_shift(1); break;
        case 0x60: m_c3.set_volume_shift(2); break;
    }
}

void age::gb_sound::write_nr33(uint8_t value)
{
    if (!m_master_on)
    {
        m_c3.log() << "write NR33 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c3.log() << "write NR33 = " << log_hex8(value); // log after state update

    m_c3.set_low_frequency_bits(value);
}

void age::gb_sound::write_nr34(uint8_t value)
{
    if (!m_master_on)
    {
        m_c3.log() << "write NR34 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c3.log() << "write NR34 = " << log_hex8(value); // log after state update

    m_c3.set_high_frequency_bits(value);
    m_c3.init_length_counter(value, should_dec_length_counter());

    if ((value & m_nr30 & gb_nrX4_initialize) > 0)
    {
        m_c3.activate();

        // DMG: if we're about to read a wave sample,
        // wave pattern memory will be "scrambled"
        if (!m_cgb && m_c3.next_sample_reads_wave_ram())
        {
            unsigned index = (m_c3.get_wave_pattern_index() + 1) & 31;
            index >>= 1;

            if (index < 4)
            {
                set_wave_ram_byte(0, m_c3_wave_ram[index]);
            }
            else
            {
                unsigned offset = index & ~3U;
                for (unsigned i = 0; i < 4; ++i)
                {
                    set_wave_ram_byte(i, m_c3_wave_ram[i + offset]);
                }
            }
        }

        // restart the wave pattern
        m_c3.init_wave_pattern_position();
    }

    m_nr34 = value;
}





//---------------------------------------------------------
//
//   Write channel 4 ports
//
//---------------------------------------------------------

age::uint8_t age::gb_sound::read_nr42() const
{
    uint8_t result = m_c4.read_nrX2();
    m_c4.log() << "read NR42 == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_sound::read_nr43() const
{
    uint8_t result = m_c4.read_nrX3();
    m_c4.log() << "read NR43 == " << log_hex8(result);
    return result;
}

age::uint8_t age::gb_sound::read_nr44() const
{
    uint8_t result = m_nr44 | 0xBF;
    m_c4.log() << "read NR44 == " << log_hex8(result);
    return result;
}




void age::gb_sound::write_nr41(uint8_t value)
{
    if (!m_master_on && m_cgb)
    {
        m_c4.log() << "write NR41 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c4.log() << "write NR41 = " << log_hex8(value); // log after state update

    m_c4.write_nrX1(value);
}

void age::gb_sound::write_nr42(uint8_t value)
{
    if (!m_master_on)
    {
        m_c4.log() << "write NR42 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c4.log() << "write NR42 = " << log_hex8(value); // log after state update

    m_c4.write_nrX2(value);
}

void age::gb_sound::write_nr43(uint8_t value)
{
    if (!m_master_on)
    {
        m_c4.log() << "write NR43 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c4.log() << "write NR43 = " << log_hex8(value); // log after state update

    m_c4.write_nrX3(value);
}

void age::gb_sound::write_nr44(uint8_t value)
{
    if (!m_master_on)
    {
        m_c4.log() << "write NR44 = " << log_hex8(value) << " ignored: all sound off";
        return;
    }
    update_state();
    m_c4.log() << "write NR44 = " << log_hex8(value); // log after state update

    m_c4.init_length_counter(value, should_dec_length_counter());

    if ((value & gb_nrX4_initialize) > 0)
    {
        bool deactivated = m_c4.init_volume_envelope(should_inc_period());
        if (!deactivated)
        {
            m_c4.activate();
            m_c4.init_generator();
        }
    }
    m_nr44 = value;
}
