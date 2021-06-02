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





//---------------------------------------------------------
//
//   Read ports
//
//---------------------------------------------------------

age::uint8_t age::gb_sound::read_nr10() const
{
    uint8_t result = m_nr10 | 0x80;
    AGE_GB_CLOG_SOUND("read NR10 = " << AGE_LOG_HEX8(result))
    return result;
}

age::uint8_t age::gb_sound::read_nr11() const
{
    uint8_t result = m_nr11 | 0x3F;
    AGE_GB_CLOG_SOUND("read NR11 = " << AGE_LOG_HEX8(result))
    return result;
}

age::uint8_t age::gb_sound::read_nr12() const
{
    uint8_t result = m_c1.read_nrX2();
    AGE_GB_CLOG_SOUND("read NR12 = " << AGE_LOG_HEX8(result))
    return result;
}

age::uint8_t age::gb_sound::read_nr14() const
{
    uint8_t result = m_nr14 | 0xBF;
    AGE_GB_CLOG_SOUND("read NR14 = " << AGE_LOG_HEX8(result))
    return result;
}



age::uint8_t age::gb_sound::read_nr21() const
{
    uint8_t result = m_nr21 | 0x3F;
    AGE_GB_CLOG_SOUND("read NR21 = " << AGE_LOG_HEX8(result))
    return result;
}

age::uint8_t age::gb_sound::read_nr22() const
{
    uint8_t result = m_c2.read_nrX2();
    AGE_GB_CLOG_SOUND("read NR22 = " << AGE_LOG_HEX8(result))
    return result;
}

age::uint8_t age::gb_sound::read_nr24() const
{
    uint8_t result = m_nr24 | 0xBF;
    AGE_GB_CLOG_SOUND("read NR24 = " << AGE_LOG_HEX8(result))
    return result;
}



age::uint8_t age::gb_sound::read_nr30() const
{
    uint8_t result = m_nr30 | 0x7F;
    AGE_GB_CLOG_SOUND("read NR30 = " << AGE_LOG_HEX8(result))
    return result;
}

age::uint8_t age::gb_sound::read_nr32() const
{
    uint8_t result = m_nr32 | 0x9F;
    AGE_GB_CLOG_SOUND("read NR32 = " << AGE_LOG_HEX8(result))
    return result;
}

age::uint8_t age::gb_sound::read_nr34() const
{
    uint8_t result = m_nr34 | 0xBF;
    AGE_GB_CLOG_SOUND("read NR34 = " << AGE_LOG_HEX8(result))
    return result;
}



age::uint8_t age::gb_sound::read_nr42() const
{
    uint8_t result = m_c4.read_nrX2();
    AGE_GB_CLOG_SOUND("read NR42 = " << AGE_LOG_HEX8(result))
    return result;
}

age::uint8_t age::gb_sound::read_nr43() const
{
    uint8_t result = m_c4.read_nrX3();
    AGE_GB_CLOG_SOUND("read NR43 = " << AGE_LOG_HEX8(result))
    return result;
}

age::uint8_t age::gb_sound::read_nr44() const
{
    uint8_t result = m_nr44 | 0xBF;
    AGE_GB_CLOG_SOUND("read NR44 = " << AGE_LOG_HEX8(result))
    return result;
}



age::uint8_t age::gb_sound::read_nr50() const
{
    AGE_GB_CLOG_SOUND("read NR50 = " << AGE_LOG_HEX8(m_nr50))
    return m_nr50;
}

age::uint8_t age::gb_sound::read_nr51() const
{
    AGE_GB_CLOG_SOUND("read NR51 = " << AGE_LOG_HEX8(m_nr51))
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

    AGE_GB_CLOG_SOUND("read NR52 = " << AGE_LOG_HEX8(result))
    return result;
}





//---------------------------------------------------------
//
//   Write control ports
//
//---------------------------------------------------------

void age::gb_sound::write_nr50(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR50 = " << AGE_LOG_HEX8(value)
                                      << ", master " << m_master_on)
    if (m_master_on)
    {
        update_state();
        m_nr50 = value; // SO1 SO2 volume

        m_c1.set_multiplier(m_nr50, m_nr51);
        m_c2.set_multiplier(m_nr50, m_nr51 >> 1);
        m_c3.set_multiplier(m_nr50, m_nr51 >> 2);
        m_c4.set_multiplier(m_nr50, m_nr51 >> 3);
    }
}

void age::gb_sound::write_nr51(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR51 = " << AGE_LOG_HEX8(value)
                                      << ", master " << m_master_on)
    if (m_master_on)
    {
        update_state();
        m_nr51 = value; // C1..4 to SO1 SO2 "routing"

        m_c1.set_multiplier(m_nr50, m_nr51);
        m_c2.set_multiplier(m_nr50, m_nr51 >> 1);
        m_c3.set_multiplier(m_nr50, m_nr51 >> 2);
        m_c4.set_multiplier(m_nr50, m_nr51 >> 3);
    }
}

void age::gb_sound::write_nr52(uint8_t value)
{
    bool new_master_on = (value & gb_sound_master_switch) != 0;
    AGE_GB_CLOG_SOUND("write NR52 = " << AGE_LOG_HEX8(value)
                                      << ", master " << m_master_on << " -> " << new_master_on)
    // sound switched off
    if (m_master_on && !new_master_on)
    {
        update_state();
        AGE_GB_CLOG_SOUND("apu switched off");
        m_clk_next_apu_event = gb_no_clock_cycle;

        m_nr10 = m_nr11 = m_nr14 = 0;
        m_nr21 = m_nr24 = 0;
        m_nr30 = m_nr32 = m_nr34 = 0;
        m_nr44                   = 0;
        m_nr50 = m_nr51 = 0;

        m_c1 = gb_sound_channel1(0x3F);
        m_c2 = gb_sound_channel2(0x3F);
        m_c3 = gb_sound_channel3(0xFF);
        m_c4 = gb_sound_channel4(0x3F);
    }

    // sound switched on
    else if (!m_master_on && new_master_on)
    {
        update_state();

        // calculate the number of clock cycles until the first frame sequencer step
        int clk_div_aligned = m_clk_current_state + m_clock.get_div_offset();
        int clks_into_step  = clk_div_aligned & (gb_apu_event_clock_cycles - 1);
        int clks_first_step = gb_apu_event_clock_cycles - clks_into_step;

        m_clk_next_apu_event        = m_clk_current_state + clks_first_step;
        m_next_frame_sequencer_step = 0;
        m_delayed_disable_c1        = false;
        m_skip_frame_sequencer_step = false;

        AGE_GB_CLOG_SOUND("apu switched on");
        AGE_GB_CLOG_SOUND("    * first frame sequencer step in " << clks_first_step << " clock cycles"
                                                                 << " (on clock cycle " << m_clk_next_apu_event << ")");
        // delay frame sequencer step 0
        // (see test rom analysis)
        int ofs = m_clock.is_double_speed() ? 2 : 4;
        if ((clk_div_aligned + ofs) & (gb_apu_event_clock_cycles / 2))
        {
            m_next_frame_sequencer_step = 7;
            m_skip_frame_sequencer_step = true;
        }
    }

    m_master_on = new_master_on;
}





//---------------------------------------------------------
//
//   Write channel 1 ports
//
//---------------------------------------------------------

void age::gb_sound::write_nr10(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR10 = " << AGE_LOG_HEX8(value)
                                      << ", period " << AGE_LOG_DEC((value >> 4) & 7)
                                      << ", up " << ((value & 8) == 0)
                                      << ", shift " << AGE_LOG_DEC(value & 7)
                                      << ", master " << m_master_on)
    if (m_master_on)
    {
        update_state();
        m_nr10 = value;
        m_c1.write_nrX0(value);
    }
}

void age::gb_sound::write_nr11(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR11 = " << AGE_LOG_HEX8(value)
                                      << ", lc " << AGE_LOG_DEC((~value & 0x3F) + 1)
                                      << ", duty " << AGE_LOG_DEC(value >> 6)
                                      << ", master " << m_master_on)
    update_state();

    // length counter always writable for DMG
    if (m_master_on || !m_cgb)
    {
        m_c1.write_nrX1(value);
    }

    // wave pattern duty only changeable, if switched on
    if (m_master_on)
    {
        m_c1.set_duty_waveform(value);
        m_nr11 = value;
    }
}

void age::gb_sound::write_nr12(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR12 = " << AGE_LOG_HEX8(value)
                                      << ", period " << AGE_LOG_DEC(value & 7)
                                      << ", up " << ((value & 8) != 0)
                                      << ", volume " << AGE_LOG_DEC(value >> 4)
                                      << ", master " << m_master_on)
    if (m_master_on)
    {
        update_state();
        m_c1.write_nrX2(value);
    }
}

void age::gb_sound::write_nr13(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR13 = " << AGE_LOG_HEX8(value)
                                      << ", master " << m_master_on)
    if (m_master_on)
    {
        update_state();
        m_c1.set_low_frequency_bits(value);
    }
}

void age::gb_sound::write_nr14(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR14 = " << AGE_LOG_HEX8(value)
                                      << ", high freq " << AGE_LOG_DEC(value & 7)
                                      << ", lc " << ((value & 0x40) > 0)
                                      << ", init " << ((value & gb_nrX4_initialize) > 0)
                                      << ", master " << m_master_on)
    if (m_master_on)
    {
        update_state();
        m_c1.set_high_frequency_bits(value);
        m_c1.init_length_counter(value, should_dec_length_counter());

        if ((value & gb_nrX4_initialize) > 0)
        {
            m_c1.init_frequency_timer(m_clk_current_state, m_clock.is_double_speed());

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

        m_nr14 = value;
    }
}





//---------------------------------------------------------
//
//   Write channel 2 ports
//
//---------------------------------------------------------

void age::gb_sound::write_nr21(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR21 = " << AGE_LOG_HEX8(value)
                                      << ", lc " << AGE_LOG_DEC((~value & 0x3F) + 1)
                                      << ", duty " << AGE_LOG_DEC(value >> 6)
                                      << ", master " << m_master_on)
    update_state();

    // length counter always writable for DMG
    if (m_master_on || !m_cgb)
    {
        m_c2.write_nrX1(value);
    }

    // wave pattern duty only changeable, if switched on
    if (m_master_on)
    {
        m_c2.set_duty_waveform(value);
        m_nr21 = value;
    }
}

void age::gb_sound::write_nr22(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR22 = " << AGE_LOG_HEX8(value)
                                      << ", period " << AGE_LOG_DEC(value & 7)
                                      << ", up " << ((value & 8) != 0)
                                      << ", volume " << AGE_LOG_DEC(value >> 4)
                                      << ", master " << m_master_on)
    if (m_master_on)
    {
        update_state();
        m_c2.write_nrX2(value);
    }
}

void age::gb_sound::write_nr23(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR23 = " << AGE_LOG_HEX8(value)
                                      << ", master " << m_master_on)
    if (m_master_on)
    {
        update_state();
        m_c2.set_low_frequency_bits(value);
    }
}

void age::gb_sound::write_nr24(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR24 = " << AGE_LOG_HEX8(value)
                                      << ", high freq " << AGE_LOG_DEC(value & 7)
                                      << ", lc " << ((value & 0x40) > 0)
                                      << ", init " << ((value & gb_nrX4_initialize) > 0)
                                      << ", master " << m_master_on)
    if (m_master_on)
    {
        update_state();
        m_c2.set_high_frequency_bits(value);
        m_c2.init_length_counter(value, should_dec_length_counter());

        if ((value & gb_nrX4_initialize) > 0)
        {
            m_c2.init_frequency_timer(m_clk_current_state, m_clock.is_double_speed());

            bool deactivated = m_c2.init_volume_envelope(should_inc_period());
            if (!deactivated)
            {
                m_c2.activate();
            }
        }

        m_nr24 = value;
    }
}





//---------------------------------------------------------
//
//   Write channel 3 ports
//
//---------------------------------------------------------

void age::gb_sound::write_nr30(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR30 = " << AGE_LOG_HEX8(value)
                                      << ", master " << m_master_on)
    if (m_master_on)
    {
        m_nr30 = value;
        if ((m_nr30 & gb_sound_master_switch) == 0)
        {
            update_state();
            m_c3.deactivate();
        }
    }
}

void age::gb_sound::write_nr31(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR31 = " << AGE_LOG_HEX8(value)
                                      << ", master " << m_master_on)

    if (m_master_on || !m_cgb) // length counter always writable for DMG
    {
        update_state();
        m_c3.write_nrX1(value);
    }
}

void age::gb_sound::write_nr32(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR32 = " << AGE_LOG_HEX8(value)
                                      << ", master " << m_master_on)
    if (m_master_on)
    {
        update_state();
        m_nr32 = value;
        switch (m_nr32 & 0x60)
        {
            case 0x00: m_c3.set_volume(0); break;
            case 0x20: m_c3.set_volume(60); break;
            case 0x40: m_c3.set_volume(30); break;
            case 0x60: m_c3.set_volume(15); break;
        }
    }
}

void age::gb_sound::write_nr33(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR33 = " << AGE_LOG_HEX8(value)
                                      << ", master " << m_master_on)
    if (m_master_on)
    {
        update_state();
        m_c3.set_low_frequency_bits(value);
    }
}

void age::gb_sound::write_nr34(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR34 = " << AGE_LOG_HEX8(value)
                                      << ", master " << m_master_on)
    if (m_master_on)
    {
        update_state();
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
}





//---------------------------------------------------------
//
//   Write channel 4 ports
//
//---------------------------------------------------------

void age::gb_sound::write_nr41(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR41 = " << AGE_LOG_HEX8(value)
                                      << ", master " << m_master_on)

    if (m_master_on || !m_cgb) // length counter always writable for DMG
    {
        update_state();
        m_c4.write_nrX1(value);
    }
}

void age::gb_sound::write_nr42(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR42 = " << AGE_LOG_HEX8(value)
                                      << ", master " << m_master_on)
    if (m_master_on)
    {
        update_state();
        m_c4.write_nrX2(value);
    }
}

void age::gb_sound::write_nr43(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR43 = " << AGE_LOG_HEX8(value)
                                      << ", master " << m_master_on)
    if (m_master_on)
    {
        update_state();
        m_c4.write_nrX3(value);
    }
}

void age::gb_sound::write_nr44(uint8_t value)
{
    AGE_GB_CLOG_SOUND("write NR44 = " << AGE_LOG_HEX8(value)
                                      << ", master " << m_master_on)
    if (m_master_on)
    {
        update_state();
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
}
