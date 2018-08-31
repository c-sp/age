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

#include "age_gb_sound.hpp"





//---------------------------------------------------------
//
//   Channel 1
//
//---------------------------------------------------------

void age::gb_sound::write_nr10(uint8_t value)
{
    if (m_master_on)
    {
        generate_samples();
        bool deactivate = m_c1.write_nrX0(value);
        if (deactivate)
        {
            deactivate_channel<gb_channel_1>();
        }
    }
}

void age::gb_sound::write_nr11(uint8_t value)
{
    // length counter always writable for DMG
    if (m_master_on || !m_is_cgb)
    {
        m_length_counter[gb_channel_1].write_nrX1(value);
    }
    // wave pattern duty only changeable, if switched on
    if (m_master_on)
    {
        generate_samples();
        m_c1.set_wave_pattern_duty(value);
        m_nr11 = value | 0x3F;
    }
}

void age::gb_sound::write_nr12(uint8_t value)
{
    if (m_master_on)
    {
        generate_samples();
        bool deactivate = m_c1.write_nrX2(value);
        if (deactivate)
        {
            deactivate_channel<gb_channel_1>();
        }
    }
}

void age::gb_sound::write_nr13(uint8_t value)
{
    if (m_master_on)
    {
        generate_samples();
        m_c1.set_low_frequency_bits(value);
    }
}

void age::gb_sound::write_nr14(uint8_t value)
{
    if (m_master_on)
    {
        generate_samples();
        m_c1.set_high_frequency_bits(value);

        bool counter_deactivate = m_length_counter[gb_channel_1].write_nrX4(value, m_next_frame_sequencer_step_odd);
        if (counter_deactivate)
        {
            deactivate_channel<gb_channel_1>();
        }

        if ((value & gb_nrX4_initialize) > 0)
        {
            bool sweep_deactivate = m_c1.init_frequency_sweep();
            bool volume_deactivate = m_c1.init_volume_sweep();

            if (sweep_deactivate || volume_deactivate)
            {
                deactivate_channel<gb_channel_1>();
            }
            else
            {
                activate_channel<gb_channel_1>();
            }
        }

        m_nr14 = value | 0xBF;
    }
}





//---------------------------------------------------------
//
//   Channel 2
//
//---------------------------------------------------------

void age::gb_sound::write_nr21(uint8_t value)
{
    // length counter always writable for DMG
    if (m_master_on || !m_is_cgb)
    {
        m_length_counter[gb_channel_2].write_nrX1(value);
    }
    // wave pattern duty only changeable, if switched on
    if (m_master_on)
    {
        generate_samples();
        m_c2.set_wave_pattern_duty(value);
        m_nr21 = value | 0x3F;
    }
}

void age::gb_sound::write_nr22(uint8_t value)
{
    if (m_master_on)
    {
        generate_samples();
        bool deactivate = m_c2.write_nrX2(value);
        if (deactivate)
        {
            deactivate_channel<gb_channel_2>();
        }
    }
}

void age::gb_sound::write_nr23(uint8_t value)
{
    if (m_master_on)
    {
        generate_samples();
        m_c2.set_low_frequency_bits(value);
    }
}

void age::gb_sound::write_nr24(uint8_t value)
{
    if (m_master_on)
    {
        generate_samples();
        m_c2.set_high_frequency_bits(value);

        bool counter_deactivate = m_length_counter[gb_channel_2].write_nrX4(value, m_next_frame_sequencer_step_odd);
        if (counter_deactivate)
        {
            deactivate_channel<gb_channel_2>();
        }

        if ((value & gb_nrX4_initialize) > 0)
        {
            bool volume_deactivated = m_c2.init_volume_sweep();
            if (volume_deactivated)
            {
                deactivate_channel<gb_channel_2>();
            }
            else
            {
                activate_channel<gb_channel_2>();
            }
        }

        m_nr24 = value | 0xBF;
    }
}





//---------------------------------------------------------
//
//   Channel 3
//
//---------------------------------------------------------

void age::gb_sound::write_nr30(uint8_t value)
{
    if (m_master_on)
    {
        m_nr30 = value | 0x7F;
        if ((m_nr30 & gb_master_switch) == 0)
        {
            generate_samples();
            deactivate_channel<gb_channel_3>();
        }
    }
}

void age::gb_sound::write_nr31(uint8_t value)
{
    // length counter always writable for DMG
    if (m_master_on || !m_is_cgb)
    {
        m_length_counter[gb_channel_3].write_nrX1(value);
    }
}

void age::gb_sound::write_nr32(uint8_t value)
{
    if (m_master_on)
    {
        generate_samples();
        m_nr32 = value | 0x9F;
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
    if (m_master_on)
    {
        generate_samples();
        m_c3.set_low_frequency_bits(value);
    }
}

void age::gb_sound::write_nr34(uint8_t value)
{
    if (m_master_on)
    {
        generate_samples();
        m_c3.set_high_frequency_bits(value);

        bool counter_deactivate = m_length_counter[gb_channel_3].write_nrX4(value, m_next_frame_sequencer_step_odd);
        if (counter_deactivate)
        {
            deactivate_channel<gb_channel_3>();
        }

        if ((value & m_nr30 & gb_nrX4_initialize) > 0)
        {
            activate_channel<gb_channel_3>();

            // DMG: if we're about to read a wave sample, wave pattern memory will be "scrambled"
            if (!m_is_cgb && (m_c3.get_cycles_next_sample() == 2))
            {
                size_t index = (m_c3.get_wave_pattern_index() + 1) & 31;
                index >>= 1;

                if (index < 4)
                {
                    set_wave_ram_byte(0, m_c3_wave_ram[index]);
                }
                else
                {
                    size_t offset = index & ~3;
                    for (size_t i = 0; i < 4; ++i)
                    {
                        set_wave_ram_byte(i, m_c3_wave_ram[i + offset]);
                    }
                }
            }

            // reset wave pattern index (restart wave playback)
            m_c3.reset_wave_pattern_index();
            m_c3_last_wave_access_cycle = m_core.get_oscillation_cycle() + m_c3.get_cycles_next_sample();
        }

        m_nr34 = value | 0xBF;
    }
}





//---------------------------------------------------------
//
//   Channel 4
//
//---------------------------------------------------------

void age::gb_sound::write_nr41(uint8_t value)
{
    // length counter always writable for DMG
    if (m_master_on || !m_is_cgb)
    {
        m_length_counter[gb_channel_4].write_nrX1(value);
    }
}

void age::gb_sound::write_nr42(uint8_t value)
{
    if (m_master_on)
    {
        generate_samples();
        bool deactivate = m_c4.write_nrX2(value);
        if (deactivate)
        {
            deactivate_channel<gb_channel_4>();
        }
    }
}

void age::gb_sound::write_nr43(uint8_t value)
{
    if (m_master_on)
    {
        generate_samples();
        m_c4.write_nrX3(value);
    }
}

void age::gb_sound::write_nr44(uint8_t value)
{
    if (m_master_on)
    {
        generate_samples();

        bool counter_deactivate = m_length_counter[gb_channel_4].write_nrX4(value, m_next_frame_sequencer_step_odd);
        if (counter_deactivate)
        {
            deactivate_channel<gb_channel_4>();
        }

        if ((value & gb_nrX4_initialize) > 0)
        {
            bool volume_deactivated = m_c4.init_volume_sweep();
            if (volume_deactivated)
            {
                deactivate_channel<gb_channel_4>();
            }
            else
            {
                activate_channel<gb_channel_4>();
                m_c4.init_generator();
            }
        }
        m_nr44 = value | 0xBF;
    }
}
