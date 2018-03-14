//
// Copyright (c) 2010-2017 Christoph Sprenger
//
// This file is part of AGE ("Another Gameboy Emulator").
// <https://gitlab.com/csprenger/AGE>
//
// AGE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// AGE is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with AGE.  If not, see <http://www.gnu.org/licenses/>.
//

#include <algorithm>

#include <age_debug.hpp>

#include "age_gb_sound_utilities.hpp"





//---------------------------------------------------------
//
//   wave generator
//
//---------------------------------------------------------

age::gb_wave_generator::gb_wave_generator()
    : gb_wave_generator(2, 7)
{
}

age::gb_wave_generator::gb_wave_generator(uint frequency_counter_shift, uint index_mask)
    : m_frequency_counter_shift(frequency_counter_shift),
      m_index_mask(index_mask)
{
    std::fill(begin(m_wave_pattern), end(m_wave_pattern), 0);
}



age::uint age::gb_wave_generator::get_frequency_bits() const
{
    return m_frequency_bits;
}

age::uint age::gb_wave_generator::get_wave_pattern_index() const
{
    return m_index;
}



void age::gb_wave_generator::set_frequency_bits(uint frequency_bits)
{
    AGE_ASSERT(frequency_bits < 2048);
    m_frequency_bits = frequency_bits;
    uint cycles_per_wave_pattern_sample = (2048 - m_frequency_bits) << m_frequency_counter_shift;
    set_cycles_per_sample(cycles_per_wave_pattern_sample);
}

void age::gb_wave_generator::set_low_frequency_bits(uint8 nrX3)
{
    uint frequency_bits = (m_frequency_bits & ~0xFF) + nrX3;
    set_frequency_bits(frequency_bits);
}

void age::gb_wave_generator::set_high_frequency_bits(uint8 nrX4)
{
    uint frequency_bits = (m_frequency_bits & 0xFF) + ((nrX4 & 7) << 8);
    set_frequency_bits(frequency_bits);
}



void age::gb_wave_generator::reset_wave_pattern_index()
{
    AGE_ASSERT(m_index_mask == 31); // only for channel 3
    m_index = 0;
    // I don't know exactly why, but apparently the first wave pattern access
    // for channel 3 is delayed after init by 6 cycles
    // (found in gambatte source code)
    set_cycles_next_sample(get_cycles_per_sample() + 6);
}

void age::gb_wave_generator::set_wave_pattern_byte(uint offset, uint8 value)
{
    AGE_ASSERT(m_index_mask == 31); // only for channel 3
    AGE_ASSERT(offset < 16);
    uint8 first_sample = value >> 4;
    uint8 second_sample = value & 0x0F;

    offset <<= 1;
    m_wave_pattern[offset] = first_sample;
    m_wave_pattern[offset + 1] = second_sample;
}

void age::gb_wave_generator::set_wave_pattern_duty(uint8 nrX1)
{
    AGE_ASSERT(m_index_mask == 7); // only for channels 1 & 2
    uint duty = nrX1 >> 6;
    m_wave_pattern = gb_wave_pattern_duty[duty];
}



age::uint8 age::gb_wave_generator::next_sample()
{
    ++m_index;
    m_index &= m_index_mask;

    uint8 sample = m_wave_pattern[m_index];
    return sample;
}





//---------------------------------------------------------
//
//   noise generator
//
//---------------------------------------------------------

age::uint8 age::gb_noise_generator::read_nrX3() const
{
    return m_nrX3;
}



void age::gb_noise_generator::write_nrX3(uint8 nrX3)
{
    m_nrX3 = nrX3;

    m_7steps = (m_nrX3 & 0x08) > 0;
    m_allow_shift = m_nrX3 < 0xE0;

    uint ratio = m_nrX3 & 0x07;
    uint shift = (m_nrX3 >> 4) + 4;

    if (ratio == 0)
    {
        ratio = 1;
        --shift;
    }

    uint cycles = ratio << shift;
    set_cycles_per_sample(cycles);
}

void age::gb_noise_generator::init_generator()
{
    m_lfsr = 0x7FFF;
}



age::uint8 age::gb_noise_generator::next_sample()
{
    if (m_allow_shift)
    {
        uint16 shifted = m_lfsr >> 1;
        uint16 xor_bit = (shifted ^ m_lfsr) & 0x0001;

        m_lfsr = shifted + (xor_bit << 14);

        if (m_7steps)
        {
            m_lfsr &= ~0x40;
            m_lfsr += xor_bit << 6;
        }
    }

    uint8 sample = static_cast<uint8>(~m_lfsr & 1) * 15;
    return sample;
}





//---------------------------------------------------------
//
//   length counter
//
//---------------------------------------------------------

age::gb_length_counter::gb_length_counter(uint counter_mask)
    : m_counter_mask(counter_mask)
{
    AGE_ASSERT(m_counter_mask > 0);
    AGE_ASSERT(m_counter_mask < 256);
}



void age::gb_length_counter::write_nrX1(uint8 nrX1)
{
    m_counter = (~nrX1 & m_counter_mask) + 1;
}

bool age::gb_length_counter::write_nrX4(uint8 nrX4, bool next_frame_sequencer_step_odd)
{
    bool disable = false;

    // the length counter will immediately be decremented, if it is
    // enabled during the first half of a length period and the
    // counter did not already reach zero
    uint decrement = 0;

    bool new_counter_enabled = (nrX4 & gb_nrX4_length_counter) > 0;
    if (new_counter_enabled)
    {
        decrement = next_frame_sequencer_step_odd ? 1 : 0;

        if (!m_counter_enabled && (m_counter > 0))
        {
            m_counter -= decrement;
            disable = m_counter == 0;
        }
    }
    m_counter_enabled = new_counter_enabled;

    // store max-length counter on channel init, if current counter is zero
    if (((nrX4 & gb_nrX4_initialize) > 0) && (m_counter == 0))
    {
        m_counter = m_counter_mask + 1 - decrement;
    }

    return disable;
}

bool age::gb_length_counter::cycle()
{
    bool deactivate = false;

    if (m_counter_enabled)
    {
        if (m_counter > 0)
        {
            --m_counter;
            deactivate = (m_counter == 0);
        }
    }

    return deactivate;
}
