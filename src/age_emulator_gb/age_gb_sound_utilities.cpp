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

constexpr const std::array<age::uint8_array<8>, 4> gb_duty_waveforms =
{{
     {{  0, 0, 0, 0,   0, 0, 0,15 }},
     {{ 15, 0, 0, 0,   0, 0, 0,15 }},
     {{ 15, 0, 0, 0,   0,15,15,15 }},
     {{  0,15,15,15,  15,15,15, 0 }},
 }};

}





//---------------------------------------------------------
//
//   sound channel common
//
//---------------------------------------------------------

bool age::gb_sound_channel::active() const
{
    return m_active;
}

age::uint32_t age::gb_sound_channel::get_multiplier() const
{
    return m_active ? m_multiplier : 0;
}



void age::gb_sound_channel::activate()
{
    m_active = true;
}

void age::gb_sound_channel::deactivate()
{
    m_active = false;
}

void age::gb_sound_channel::set_multiplier(uint8_t nr50, uint8_t shifted_nr51)
{
    // calculate channel multiplier
    // this value includes:
    //     - SOx volume
    //     - channel to SOx "routing"

    int16_t volume_SO1 = (nr50 & 7) + 1;
    int16_t volume_SO2 = ((nr50 >> 4) & 7) + 1;

    volume_SO1 *= shifted_nr51 & 1;
    volume_SO2 *= (shifted_nr51 >> 4) & 1;

    // SO1 is the right channel, SO2 is the left channel
    m_multiplier = pcm_sample(volume_SO2, volume_SO1).m_stereo_sample;
}





//---------------------------------------------------------
//
//   duty channel
//
//---------------------------------------------------------

age::gb_duty_source::gb_duty_source()
{
    set_frequency_bits(0);
    set_duty_waveform(0);
}



age::int16_t age::gb_duty_source::get_frequency_bits() const
{
    return m_frequency_bits;
}



#define DUTY_FREQ_TO_SAMPLES ((2048 - m_frequency_bits) << 1)

void age::gb_duty_source::set_frequency_bits(int16_t frequency_bits)
{
    AGE_ASSERT((frequency_bits >= 0) && (frequency_bits < 2048));
    m_frequency_bits = frequency_bits;
    int samples = DUTY_FREQ_TO_SAMPLES;
    set_frequency_timer_period(samples);
}

void age::gb_duty_source::set_low_frequency_bits(uint8_t nrX3)
{
    int16_t frequency_bits = (m_frequency_bits & ~0xFF) + nrX3;
    set_frequency_bits(frequency_bits);
}

void age::gb_duty_source::set_high_frequency_bits(uint8_t nrX4)
{
    int16_t frequency_bits = (m_frequency_bits & 0xFF) + ((nrX4 << 8) & 0x700);
    set_frequency_bits(frequency_bits);
}



void age::gb_duty_source::init_frequency_timer()
{
    reset_frequency_timer(4);
}

void age::gb_duty_source::set_duty_waveform(uint8_t nrX1)
{
    m_duty = nrX1 >> 6;
}

void age::gb_duty_source::init_duty_waveform_position(int16_t frequency_bits, int sample_offset, uint8_t index)
{
    set_frequency_bits(frequency_bits);

    int offset = sample_offset - DUTY_FREQ_TO_SAMPLES;
    reset_frequency_timer(offset);

    m_index = index;
}



age::uint8_t age::gb_duty_source::next_wave_sample()
{
    ++m_index;
    m_index &= 7;

    uint8_t sample = gb_duty_waveforms[m_duty][m_index];
    return sample;
}





//---------------------------------------------------------
//
//   wave channel
//
//---------------------------------------------------------

age::gb_wave_source::gb_wave_source()
{
    std::fill(begin(m_wave_pattern), end(m_wave_pattern), 0);
    calculate_frequency_timer_period();
}



bool age::gb_wave_source::next_sample_reads_wave_ram() const
{
    return get_frequency_timer() == 1;
}

bool age::gb_wave_source::wave_ram_just_read() const
{
    return m_wave_ram_just_read;
}

age::uint8_t age::gb_wave_source::get_wave_pattern_index() const
{
    return m_index;
}



void age::gb_wave_source::set_low_frequency_bits(uint8_t nrX3)
{
    m_frequency_low = nrX3;
    calculate_frequency_timer_period();
}

void age::gb_wave_source::set_high_frequency_bits(uint8_t nrX4)
{
    m_frequency_high = nrX4 & 7;
    calculate_frequency_timer_period();
}

void age::gb_wave_source::calculate_frequency_timer_period()
{
    int frequency = m_frequency_low + (m_frequency_high << 8);
    AGE_ASSERT((frequency >= 0) && (frequency < 2048));
    int samples = 2048 - frequency;
    set_frequency_timer_period(samples);
}



void age::gb_wave_source::init_wave_pattern_position()
{
    m_index = 0;
    reset_frequency_timer(3);
}

void age::gb_wave_source::set_wave_pattern_byte(unsigned offset, uint8_t value)
{
    uint8_t first_sample = value >> 4;
    uint8_t second_sample = value & 0x0F;

    AGE_ASSERT(offset < 16);
    offset <<= 1;
    m_wave_pattern[offset] = first_sample;
    m_wave_pattern[offset + 1] = second_sample;
}



age::uint8_t age::gb_wave_source::next_wave_sample()
{
    ++m_index;
    m_index &= 31;

    uint8_t sample = m_wave_pattern[m_index];
    m_wave_ram_just_read = true;

    return sample;
}

void age::gb_wave_source::generate_samples(pcm_vector &buffer, int buffer_index, int samples_to_generate)
{
    m_wave_ram_just_read = false;
    gb_sample_generator<gb_wave_source>::generate_samples(buffer, buffer_index, samples_to_generate);
    m_wave_ram_just_read &= frequency_timer_just_reloaded();
}





//---------------------------------------------------------
//
//   noise channel
//
//---------------------------------------------------------

age::gb_noise_source::gb_noise_source()
{
    write_nrX3(0);
}



age::uint8_t age::gb_noise_source::read_nrX3() const
{
    return m_nrX3;
}



void age::gb_noise_source::write_nrX3(uint8_t nrX3)
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

    int samples = ratio << (shift - 1);
    set_frequency_timer_period(samples);
}

void age::gb_noise_source::init_generator()
{
    m_lfsr = 0x7FFF;
}



age::uint8_t age::gb_noise_source::next_wave_sample()
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
