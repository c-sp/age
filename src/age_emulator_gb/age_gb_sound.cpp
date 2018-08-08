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



namespace age {

constexpr uint gb_frame_sequencer_cycle_shift = 13;

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
//   Read ports.
//
//---------------------------------------------------------

age::uint8 age::gb_sound::read_nr10() const { return m_c1.read_nrX0(); }
age::uint8 age::gb_sound::read_nr11() const { return m_nr11; }
age::uint8 age::gb_sound::read_nr12() const { return m_c1.read_nrX2(); }
age::uint8 age::gb_sound::read_nr13() const { return 0xFF;   }
age::uint8 age::gb_sound::read_nr14() const { return m_nr14; }

age::uint8 age::gb_sound::read_nr21() const { return m_nr21; }
age::uint8 age::gb_sound::read_nr22() const { return m_c2.read_nrX2(); }
age::uint8 age::gb_sound::read_nr23() const { return 0xFF;   }
age::uint8 age::gb_sound::read_nr24() const { return m_nr24; }

age::uint8 age::gb_sound::read_nr30() const { return m_nr30; }
age::uint8 age::gb_sound::read_nr31() const { return 0xFF;   }
age::uint8 age::gb_sound::read_nr32() const { return m_nr32; }
age::uint8 age::gb_sound::read_nr33() const { return 0xFF;   }
age::uint8 age::gb_sound::read_nr34() const { return m_nr34; }

age::uint8 age::gb_sound::read_nr41() const { return 0xFF;   }
age::uint8 age::gb_sound::read_nr42() const { return m_c4.read_nrX2(); }
age::uint8 age::gb_sound::read_nr43() const { return m_c4.read_nrX3(); }
age::uint8 age::gb_sound::read_nr44() const { return m_nr44; }

age::uint8 age::gb_sound::read_nr50() const { return m_nr50; }
age::uint8 age::gb_sound::read_nr51() const { return m_nr51; }
age::uint8 age::gb_sound::read_nr52() const { return m_nr52; }





//---------------------------------------------------------
//
//   Access wave pattern ram.
//
//---------------------------------------------------------

age::uint8 age::gb_sound::read_wave_ram(uint offset)
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

    uint8 result = m_c3_wave_ram[offset];
    return result;
}

void age::gb_sound::write_wave_ram(uint offset, uint8 value)
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
//   Write to control ports
//
//---------------------------------------------------------

void age::gb_sound::write_nr50(uint8 value)
{
    if (m_master_on)
    {
        generate_samples();
        m_nr50 = value;

        calculate_channel_multiplier<gb_channel_1>();
        calculate_channel_multiplier<gb_channel_2>();
        calculate_channel_multiplier<gb_channel_3>();
        calculate_channel_multiplier<gb_channel_4>();
    }
}

void age::gb_sound::write_nr51(uint8 value)
{
    if (m_master_on)
    {
        generate_samples();
        m_nr51 = value;

        calculate_channel_multiplier<gb_channel_1>();
        calculate_channel_multiplier<gb_channel_2>();
        calculate_channel_multiplier<gb_channel_3>();
        calculate_channel_multiplier<gb_channel_4>();
    }
}

void age::gb_sound::write_nr52(uint8 value)
{
    generate_samples();

    m_nr52 &= ~gb_master_switch;
    m_nr52 |= value & gb_master_switch;

    // sound switched off: reset all sound i/o ports
    bool new_master_on = m_nr52 >= gb_master_switch;
    if (m_master_on && !new_master_on)
    {
        write_nr10(0);
        write_nr11(0);
        write_nr12(0);
        write_nr13(0);
        write_nr14(0);

        write_nr21(0);
        write_nr22(0);
        write_nr23(0);
        write_nr24(0);

        write_nr30(0);
        write_nr31(0);
        write_nr32(0);
        write_nr33(0);
        write_nr34(0);

        write_nr41(0);
        write_nr42(0);
        write_nr43(0);
        write_nr44(0);

        write_nr50(0);
        write_nr51(0);
    }

    // sound switched on: reset frame sequencer
    else if (!m_master_on && new_master_on)
    {
        m_next_frame_sequencer_step = 0;
        m_next_frame_sequencer_step_odd = false;

        m_c1.reset_volume_sweep();
        m_c2.reset_volume_sweep();
        m_c4.reset_volume_sweep();
    }

    // set this after (!) ports were reset
    m_master_on = new_master_on;
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

    uint current_cycle = m_core.get_oscillation_cycle();
    uint cycle_offset = ((current_cycle >> gb_frame_sequencer_cycle_shift) + 1) << gb_frame_sequencer_cycle_shift;
    cycle_offset -= current_cycle;

    m_core.insert_event(cycle_offset, gb_event::sound_frame_sequencer);
}



void age::gb_sound::generate_samples()
{
    uint cycles_elapsed = (m_core.get_oscillation_cycle() - m_last_generate_samples_cycle) & gb_cycle_sample_mask;
    if (cycles_elapsed > 0)
    {
        m_last_generate_samples_cycle += cycles_elapsed;

        uint samples_to_generate = cycles_elapsed >> gb_sample_cycle_shift;
        AGE_ASSERT(samples_to_generate > 0);

        uint sample_index = m_samples.size();
        m_samples.resize(sample_index + samples_to_generate);

        if (m_master_on)
        {
            m_c1.generate(m_samples, sample_index, cycles_elapsed);
            m_c2.generate(m_samples, sample_index, cycles_elapsed);

            uint cycle_offset = m_c3.generate(m_samples, sample_index, cycles_elapsed);
            if (cycle_offset != uint_max)
            {
                AGE_ASSERT(cycle_offset <= cycles_elapsed);
                m_c3_last_wave_access_cycle = m_last_generate_samples_cycle - cycle_offset;
            }

            m_c4.generate(m_samples, sample_index, cycles_elapsed);
        }
    }
}



void age::gb_sound::set_back_cycles(uint offset)
{
    AGE_GB_SET_BACK_CYCLES(m_last_generate_samples_cycle, offset);
    AGE_GB_SET_BACK_CYCLES_OVERFLOW(m_c3_last_wave_access_cycle, offset);
}





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

void age::gb_sound::set_wave_ram_byte(uint offset, uint8 value)
{
    m_c3_wave_ram[offset] = value;
    m_c3.set_wave_pattern_byte(offset, value);
}





//---------------------------------------------------------
//
//   Object creation.
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
