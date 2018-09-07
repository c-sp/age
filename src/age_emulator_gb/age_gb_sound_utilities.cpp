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

#include "age_gb_sound_utilities.hpp"



namespace
{

constexpr age::uint8_t gb_nrX4_length_counter = 0x40;

constexpr const std::array<age::uint8_array<32>, 4> gb_wave_pattern_duty =
{{
     {{  0, 0, 0, 0,  15, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 }},
     {{  0, 0, 0, 0,  15,15, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 }},
     {{  0, 0,15,15,  15,15, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 }},
     {{ 15,15,15,15,   0, 0,15,15,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 }},
 }};

}





//---------------------------------------------------------
//
//   wave generator
//
//---------------------------------------------------------

age::gb_wave_generator::gb_wave_generator()
    : gb_wave_generator(2, 7)
{
}

age::gb_wave_generator::gb_wave_generator(size_t frequency_counter_shift, uint8_t index_mask)
    : m_frequency_counter_shift(frequency_counter_shift),
      m_index_mask(index_mask)
{
    std::fill(begin(m_wave_pattern), end(m_wave_pattern), 0);
}



age::size_t age::gb_wave_generator::get_frequency_bits() const
{
    return m_frequency_bits;
}

age::uint8_t age::gb_wave_generator::get_wave_pattern_index() const
{
    return m_index;
}



void age::gb_wave_generator::set_frequency_bits(size_t frequency_bits)
{
    AGE_ASSERT(frequency_bits < 2048);
    m_frequency_bits = frequency_bits;
    size_t cycles_per_wave_pattern_sample = (2048 - m_frequency_bits) << m_frequency_counter_shift;
    set_cycles_per_sample(cycles_per_wave_pattern_sample);
}

void age::gb_wave_generator::set_low_frequency_bits(uint8_t nrX3)
{
    size_t frequency_bits = (m_frequency_bits & ~0xFF) + nrX3;
    set_frequency_bits(frequency_bits);
}

void age::gb_wave_generator::set_high_frequency_bits(uint8_t nrX4)
{
    size_t frequency_bits = (m_frequency_bits & 0xFF) + ((nrX4 & 7) << 8);
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

void age::gb_wave_generator::set_wave_pattern_byte(unsigned offset, uint8_t value)
{
    AGE_ASSERT(m_index_mask == 31); // only for channel 3
    AGE_ASSERT(offset < 16);
    uint8_t first_sample = value >> 4;
    uint8_t second_sample = value & 0x0F;

    offset <<= 1;
    m_wave_pattern[offset] = first_sample;
    m_wave_pattern[offset + 1] = second_sample;
}

void age::gb_wave_generator::set_wave_pattern_duty(uint8_t nrX1)
{
    AGE_ASSERT(m_index_mask == 7); // only for channels 1 & 2
    size_t duty = nrX1 >> 6;
    m_wave_pattern = gb_wave_pattern_duty[duty];
}



age::uint8_t age::gb_wave_generator::next_sample()
{
    ++m_index;
    m_index &= m_index_mask;

    uint8_t sample = m_wave_pattern[m_index];
    return sample;
}





//---------------------------------------------------------
//
//   noise generator
//
//---------------------------------------------------------

age::uint8_t age::gb_noise_generator::read_nrX3() const
{
    return m_nrX3;
}



void age::gb_noise_generator::write_nrX3(uint8_t nrX3)
{
    m_nrX3 = nrX3;

    m_7steps = (m_nrX3 & 0x08) > 0;
    m_allow_shift = m_nrX3 < 0xE0;

    int ratio = m_nrX3 & 0x07;
    int shift = (m_nrX3 >> 4) + 4;

    if (ratio == 0)
    {
        ratio = 1;
        --shift;
    }

    int cycles = ratio << shift;
    set_cycles_per_sample(cycles);
}

void age::gb_noise_generator::init_generator()
{
    m_lfsr = 0x7FFF;
}



age::uint8_t age::gb_noise_generator::next_sample()
{
    if (m_allow_shift)
    {
        int shifted = m_lfsr >> 1;
        int xor_bit = (shifted ^ m_lfsr) & 0x0001;

        m_lfsr = (shifted + (xor_bit << 14)) & 0xFFFF;

        if (m_7steps)
        {
            m_lfsr &= ~0x40;
            m_lfsr += xor_bit << 6;
        }
    }

    uint8_t sample = static_cast<uint8_t>(~m_lfsr & 1) * 15;
    return sample;
}





//---------------------------------------------------------
//
//   length counter
//
//---------------------------------------------------------

age::gb_length_counter::gb_length_counter(int16_t counter_mask)
    : m_counter_mask(counter_mask)
{
    AGE_ASSERT(m_counter_mask > 0);
    AGE_ASSERT(m_counter_mask < 256);
}



void age::gb_length_counter::write_nrX1(uint8_t nrX1)
{
    m_counter = (~nrX1 & m_counter_mask) + 1;
}

bool age::gb_length_counter::write_nrX4(uint8_t nrX4, bool next_frame_sequencer_step_odd)
{
    bool disable_channel = false;

    // the length counter will immediately be decremented, if it is
    // enabled during the first half of a length period and the
    // counter did not already reach zero
    int8_t decrement = 0;

    bool new_counter_enabled = (nrX4 & gb_nrX4_length_counter) > 0;
    if (new_counter_enabled)
    {
        decrement = next_frame_sequencer_step_odd ? 1 : 0;

        if (!m_counter_enabled && (m_counter > 0))
        {
            m_counter -= decrement;
            disable_channel = m_counter == 0;
        }
    }
    m_counter_enabled = new_counter_enabled;

    // store max-length counter on channel init, if current counter is zero
    if (((nrX4 & gb_nrX4_initialize) > 0) && (m_counter == 0))
    {
        m_counter = m_counter_mask + 1 - decrement;
    }

    return disable_channel;
}

bool age::gb_length_counter::tick()
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
