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

#ifndef AGE_GB_SOUND_UTILITIES_HPP
#define AGE_GB_SOUND_UTILITIES_HPP

//!
//! \file
//!

#include <algorithm> // std::min
#include <limits>

#include <age_debug.hpp>
#include <age_types.hpp>
#include <pcm/age_pcm_sample.hpp>



namespace age
{

constexpr int8_t gb_sample_cycle_shift = 1; // 2097152 samples per second for easier emulation (will be downsampled later on)
constexpr int32_t gb_cycles_per_sample = 1 << gb_sample_cycle_shift;
constexpr int32_t gb_cycle_sample_mask = ~(gb_cycles_per_sample - 1);

constexpr uint8 gb_nrX4_length_counter = 0x40;
constexpr uint8 gb_nrX4_initialize = 0x80;



template<typename TYPE>
class gb_sample_generator
{
public:

    int32_t get_cycles_per_sample() const
    {
        return m_cycles_per_sample;
    }

    int32_t get_cycles_next_sample() const
    {
        return m_cycles_next_sample;
    }

    void set_channel_multiplier(uint32 channel_multiplier)
    {
        AGE_ASSERT((channel_multiplier & 0xFFFF) <= 8);
        AGE_ASSERT((channel_multiplier >> 16) <= 8);
        m_channel_multiplier = channel_multiplier;
        calculate_sample();
    }

    void set_volume(uint8 volume)
    {
        AGE_ASSERT(volume <= 60);
        m_volume = volume;
        calculate_sample();
    }

    int32_t generate(pcm_vector &buffer, uint buffer_index, int32_t cycles_elapsed)
    {
        AGE_ASSERT(m_cycles_per_sample != 0);
        AGE_ASSERT((cycles_elapsed % gb_cycles_per_sample) == 0);

        int32_t last_sample_change = -1;

        for (int32_t cycles_remaining = cycles_elapsed; cycles_remaining > 0; )
        {
            // write one and the same sample until we have to calculate the next one
            int32_t cycles = std::min(cycles_remaining, m_cycles_next_sample) & gb_cycle_sample_mask;
            int32_t samples_to_write = cycles >> gb_sample_cycle_shift;
            AGE_ASSERT((cycles % gb_cycles_per_sample) == 0);

            for (uint max = buffer_index + samples_to_write; buffer_index < max; ++buffer_index)
            {
                buffer[buffer_index].m_stereo_sample += m_current_multiplied_sample;
            }

            cycles_remaining -= cycles;
            m_cycles_next_sample -= cycles;

            // check for next wave pattern sample
            if (m_cycles_next_sample < gb_cycles_per_sample)
            {
                m_cycles_next_sample += m_cycles_per_sample;
                last_sample_change = cycles_remaining;
                m_current_sample = static_cast<TYPE*>(this)->next_sample();
                calculate_sample();
            }
        }

        return last_sample_change;
    }

protected:

    void set_cycles_per_sample(int32_t cycles_per_sample)
    {
        AGE_ASSERT(cycles_per_sample > 0);
        AGE_ASSERT(cycles_per_sample >= gb_cycles_per_sample);
        AGE_ASSERT((cycles_per_sample % gb_cycles_per_sample) == 0);
        m_cycles_per_sample = cycles_per_sample;
    }

    void set_cycles_next_sample(int32_t cycles_next_sample)
    {
        m_cycles_next_sample = cycles_next_sample;
    }

private:

    void calculate_sample()
    {
        AGE_ASSERT(m_current_sample <= 15);
        //
        // divider:
        //
        //   / 15  ->  max amplitude
        //   / 32  ->  4 channels, s0x volume up to 8
        //   / 60  ->  combining volume 1-15 (channels 1,2,4) and volume 0, 1, 0.5, 0.25 (channel 3)
        //
        int32 value = (2 * m_current_sample - 15) * std::numeric_limits<int16>::max() * m_volume;
        value /= 15 * 32 * 60;
        m_current_multiplied_sample = value * m_channel_multiplier;
    }

    int32_t m_cycles_per_sample = 0;
    int32_t m_cycles_next_sample = 0;

    uint32 m_channel_multiplier = 0;
    uint8 m_volume = 15;

    uint8 m_current_sample = 0;
    uint32 m_current_multiplied_sample = 0;
};





template<typename TYPE>
class gb_volume_sweep : public TYPE
{
public:

    uint8 read_nrX2() const
    {
        return m_nrX2;
    }

    bool write_nrX2(uint8 nrX2)
    {
        // "zombie" update
        uint volume = m_volume;
        if ((m_period == 0) && (m_period_counter > 0))
        {
            ++volume;
        }
        else if (!m_sweep_up)
        {
            volume += 2;
        }

        if (m_sweep_up != ((nrX2 & 0x08) > 0))
        {
            volume = 16 - volume;
        }

        volume &= 0x0F;
        switch_volume(volume);

        // store new value
        m_nrX2 = nrX2;
        m_period = m_nrX2 & 0x07;
        m_sweep_up = (m_nrX2 & 0x08) > 0;

        return channel_off();
    }

    bool init_volume_sweep()
    {
        m_period_counter = (m_period == 0) ? 8 : m_period;
        switch_volume(m_nrX2 >> 4);
        return channel_off();
    }

    void sweep_volume()
    {
        if (m_period_counter > 0)
        {
            --m_period_counter;
            if (m_period_counter == 0)
            {
                if (m_period > 0)
                {
                    if (sweep())
                    {
                        m_period_counter = m_period;
                    }
                    // else m_period_counter remains unchanged (= 0) -> stop sweeping
                }
                // if we cannot sweep at the moment, retry later
                else
                {
                    m_period_counter = 8;
                }
            }
        }
    }

    void reset_volume_sweep()
    {
        // terminate any ongoing sweep
        m_period_counter = 0;
    }

private:

    bool channel_off() const
    {
        bool result = (m_nrX2 & 0xF8) == 0;
        return result;
    }

    void switch_volume(uint new_volume)
    {
        AGE_ASSERT(new_volume < 0x10);
        if (m_volume != new_volume)
        {
            m_volume = new_volume;
            TYPE::set_volume(m_volume * 4);
        }
    }

    bool sweep()
    {
        uint volume = m_sweep_up ? m_volume + 1 : m_volume - 1;

        bool adjust_volume = volume < 0x10;
        if (adjust_volume)
        {
            switch_volume(volume);
        }

        return adjust_volume;
    }

    uint8 m_nrX2 = 0;

    bool m_sweep_up = false;
    uint m_period = 0;
    uint m_period_counter = 0;
    uint m_volume = 0;
};





template<typename TYPE>
class gb_frequency_sweep : public TYPE
{
public:

    uint8 read_nrX0() const
    {
        return m_nrX0;
    }

    bool write_nrX0(uint8 nrX0)
    {
        m_nrX0 = nrX0 | 0x80;

        m_period = (nrX0 >> 4) & 7;
        m_shift = nrX0 & 7;
        m_sweep_up = (nrX0 & 8) == 0;

        bool deactivate = m_swept_down && m_sweep_up;
        return deactivate;
    }

    bool init_frequency_sweep()
    {
        m_swept_down = false;
        m_frequency_bits = TYPE::get_frequency_bits();

        // schedule event, if period or shift are set
        m_sweep_enabled = (m_period + m_shift) > 0;
        if (m_sweep_enabled)
        {
            set_period_counter();
        }

        // check for channel deactivation only, if shift is not zero
        bool deactivate = false;
        if (m_shift > 0)
        {
            uint frequency_bits = sweep_frequency_bits();
            deactivate = invalid_frequency_bits(frequency_bits);
        }

        return deactivate;
    }

    bool sweep_frequency()
    {
        bool deactivate = false;

        if (m_sweep_enabled)
        {
            AGE_ASSERT(m_period_counter > 0);
            --m_period_counter;
            if (m_period_counter == 0)
            {
                if (m_period > 0)
                {
                    uint frequency_bits = sweep_frequency_bits();
                    deactivate = invalid_frequency_bits(frequency_bits);

                    if (!deactivate && (m_shift > 0))
                    {
                        m_frequency_bits = frequency_bits;
                        TYPE::set_frequency_bits(m_frequency_bits);

                        uint frequency_bits = sweep_frequency_bits();
                        deactivate = invalid_frequency_bits(frequency_bits);
                    }
                }
                set_period_counter();
            }
        }

        return deactivate;
    }

private:

    bool invalid_frequency_bits(uint frequency_bits) const
    {
        return frequency_bits > 2047;
    }

    uint sweep_frequency_bits()
    {
        uint shifted = m_frequency_bits >> m_shift;
        uint result = m_frequency_bits;

        if (m_sweep_up)
        {
            result += shifted;
        }
        else
        {
            result -= shifted;
            m_swept_down = true;
        }

        return result;
    }

    void set_period_counter()
    {
        m_period_counter = (m_period == 0) ? 8 : m_period;
    }

    uint8 m_nrX0 = 0;

    uint m_frequency_bits = 0;
    uint m_period = 0;
    uint m_shift = 0;
    bool m_sweep_up = false;

    bool m_sweep_enabled = false;
    bool m_swept_down = false;
    uint m_period_counter = 0;
};





class gb_wave_generator : public gb_sample_generator<gb_wave_generator>
{
public:

    gb_wave_generator();
    gb_wave_generator(uint frequency_counter_shift, uint wave_pattern_index_mask);

    uint get_frequency_bits() const;
    uint get_wave_pattern_index() const;

    void set_frequency_bits(uint frequency_bits);
    void set_low_frequency_bits(uint8 nrX3);
    void set_high_frequency_bits(uint8 nrX4);

    void reset_wave_pattern_index();
    void set_wave_pattern_byte(uint offset, uint8 value);
    void set_wave_pattern_duty(uint8 nrX1);

    uint8 next_sample();

private:

    const uint m_frequency_counter_shift;
    const uint m_index_mask;

    uint m_frequency_bits = 0;
    uint m_index = 0;

    uint8_array<32> m_wave_pattern;
};





class gb_noise_generator : public gb_sample_generator<gb_noise_generator>
{
public:

    uint8 read_nrX3() const;

    void write_nrX3(uint8 nrX3);
    void init_generator();

    uint8 next_sample();

private:

    uint8 m_nrX3 = 0;
    bool m_7steps = false;
    bool m_allow_shift = true;
    uint16 m_lfsr = 0;
};





class gb_length_counter
{
public:

    gb_length_counter(uint counter_mask);

    void write_nrX1(uint8 nrX1);
    bool write_nrX4(uint8 nrX4, bool next_frame_sequencer_step_odd);
    bool cycle();

private:

    const uint m_counter_mask;

    bool m_counter_enabled = false;
    uint m_counter = 0;
};

} // namespace age



#endif // AGE_GB_SOUND_UTILITIES_HPP
