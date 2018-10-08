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

#include <age_debug.hpp>
#include <age_types.hpp>
#include <pcm/age_pcm_sample.hpp>



namespace age
{

constexpr uint8_t gb_nrX4_initialize = 0x80;



template<typename TYPE>
class gb_sample_generator
{
public:

    int get_frequency_timer() const
    {
        return m_frequency_timer;
    }

    bool timer_reload_on_last_sample() const
    {
        return m_frequency_timer_just_reloaded;
    }

    void set_channel_multiplier(uint32_t channel_multiplier)
    {
        AGE_ASSERT((channel_multiplier & 0xFFFF) <= 8);
        AGE_ASSERT((channel_multiplier >> 16) <= 8);
        m_channel_multiplier = channel_multiplier;
        calculate_output_sample();
    }

    void set_volume(uint8_t volume)
    {
        AGE_ASSERT(volume <= 60);
        m_volume = volume;
        calculate_output_sample();
    }

    void generate_samples(pcm_vector &buffer, int buffer_index, int samples_to_generate)
    {
        AGE_ASSERT((buffer_index >= 0) && (samples_to_generate > 0));
        AGE_ASSERT(int_max - samples_to_generate >= buffer_index);
        AGE_ASSERT(m_frequency_timer_period > 0);
        AGE_ASSERT(m_frequency_timer >= 0);

        // we assume to generate at least one sample
        m_frequency_timer_just_reloaded = false;

        for (int samples_remaining = samples_to_generate; samples_remaining > 0; )
        {
            // write output samples until we have to retrieve the next wave sample
            int samples = std::min(samples_remaining, m_frequency_timer);

            for (int max = buffer_index + samples; buffer_index < max; ++buffer_index)
            {
                //! \todo result distorted by overflow from one sample into the other
                buffer[buffer_index].m_stereo_sample += m_output_sample;
            }

            samples_remaining -= samples;
            m_frequency_timer -= samples;

            // check for next wave input
            if (m_frequency_timer == 0)
            {
                m_frequency_timer_just_reloaded = true;
                m_frequency_timer = m_frequency_timer_period;
                m_wave_sample = static_cast<TYPE*>(this)->next_wave_sample();
                calculate_output_sample();
            }
        }

        bool just_reloaded = m_frequency_timer == m_frequency_timer_period;
        m_frequency_timer_just_reloaded &= just_reloaded;

        AGE_ASSERT(m_frequency_timer > 0);
    }

protected:

    void set_frequency_timer_period(int number_of_samples)
    {
        AGE_ASSERT(number_of_samples > 0);
        m_frequency_timer_period = number_of_samples;
    }

    void reset_frequency_timer(int sample_offset)
    {
        m_frequency_timer = m_frequency_timer_period + sample_offset;
    }

private:

    void calculate_output_sample()
    {
        AGE_ASSERT(m_wave_sample <= 15);
        AGE_ASSERT(m_volume <= 60);
        //
        // divider:
        //
        //   / 15  ->  max amplitude
        //   / 32  ->  4 channels, s0x volume up to 8
        //   / 60  ->  combining volume 1-15 (channels 1,2,4)
        //             and volume 0, 1, 0.5, 0.25 (channel 3)
        //
        int value = (2 * m_wave_sample * m_volume - 15 * 60) * int16_t_max;
        // int value = (2 * m_wave_sample - 15) * int16_t_max * m_volume;
        value /= 15 * 32 * 60;
        //! \todo result distorted by overflow from one sample into the other
        m_output_sample = value * m_channel_multiplier;
    }

    int m_frequency_timer_period = 0;
    int m_frequency_timer = 0;
    bool m_frequency_timer_just_reloaded = false;

    //!
    //! \brief the current wave sample
    //! (taken from duty waveform, wave ram or noise LFSR)
    //!
    uint8_t m_wave_sample = 0;
    uint32_t m_channel_multiplier = 0;
    uint8_t m_volume = 0;

    uint32_t m_output_sample = 0;
};





template<typename TYPE>
class gb_volume_envelope : public TYPE
{
public:

    uint8_t read_nrX2() const
    {
        return m_nrX2;
    }

    bool write_nrX2(uint8_t nrX2)
    {
        // "zombie" update
        int volume = m_volume;
        if ((m_period == 0) && (m_period_counter > 0))
        {
            ++volume;
        }
        else if (!m_increase_volume)
        {
            volume += 2;
        }

        if (m_increase_volume != ((nrX2 & 0x08) > 0))
        {
            volume = 16 - volume;
        }

        volume &= 0x0F;
        update_volume(volume);

        // store new value
        m_nrX2 = nrX2;
        m_period = m_nrX2 & 0x07;
        m_increase_volume = (m_nrX2 & 0x08) > 0;

        return channel_off();
    }

    bool init_volume_envelope(bool inc_period)
    {
        m_period_counter = (m_period == 0) ? 8 : m_period;
        m_period_counter += inc_period ? 1 : 0;
        update_volume(m_nrX2 >> 4);
        return channel_off();
    }

    void volume_envelope()
    {
        if (m_period_counter > 0)
        {
            --m_period_counter;
            if (m_period_counter == 0)
            {
                if (m_period > 0)
                {
                    if (adjust_volume())
                    {
                        m_period_counter = m_period;
                    }
                    // else m_period_counter unchanged (= 0) -> stop
                }
                // cannot adjust volume at the moment -> retry later
                else
                {
                    m_period_counter = 8;
                }
            }
        }
    }

private:

    bool channel_off() const
    {
        bool result = (m_nrX2 & 0xF8) == 0;
        return result;
    }

    void update_volume(int new_volume)
    {
        AGE_ASSERT((new_volume >= 0) && (new_volume < 0x10));
        if (m_volume != new_volume)
        {
            m_volume = new_volume;
            TYPE::set_volume(m_volume * 4);
        }
    }

    bool adjust_volume()
    {
        int volume = m_increase_volume ? m_volume + 1 : m_volume - 1;

        bool adjust_volume = (volume >= 0) && (volume < 0x10);
        if (adjust_volume)
        {
            update_volume(volume);
        }

        return adjust_volume;
    }

    uint8_t m_nrX2 = 0;

    bool m_increase_volume = false;
    int8_t m_period = 0;
    int8_t m_period_counter = 0;
    int8_t m_volume = 0;
};





template<typename TYPE>
class gb_frequency_sweep : public TYPE
{
public:

    bool write_nrX0(uint8_t nrX0)
    {
        m_period = (nrX0 >> 4) & 7;
        m_shift = nrX0 & 7;
        m_sweep_up = (nrX0 & 8) == 0;

        bool deactivate = m_swept_down && m_sweep_up;
        return deactivate;
    }

    bool init_frequency_sweep(bool skip_first_step)
    {
        m_skip_first_step = skip_first_step;

        m_swept_down = false;
        m_frequency_bits = TYPE::get_frequency_bits();

        // enable sweep if period or shift are set
        m_sweep_enabled = (m_period + m_shift) > 0;
        if (m_sweep_enabled)
        {
            set_period_counter();
        }

        // check for channel deactivation only, if shift is not zero
        bool deactivate = (m_shift > 0) && next_sweep_invalid();
        return deactivate;
    }

    bool sweep_frequency()
    {
        bool deactivate = false;

        if (m_sweep_enabled && !m_skip_first_step)
        {
            AGE_ASSERT(m_period_counter > 0);
            --m_period_counter;
            if (m_period_counter == 0)
            {
                if (m_period > 0)
                {
                    auto frequency_bits = sweep_frequency_bits();
                    deactivate = invalid_frequency(frequency_bits);

                    if (!deactivate && (m_shift > 0))
                    {
                        m_frequency_bits = frequency_bits;
                        TYPE::set_frequency_bits(frequency_bits);
                        deactivate = next_sweep_invalid();
                    }
                }
                set_period_counter();
            }
        }
        m_skip_first_step = false;

        return deactivate;
    }

private:

    bool next_sweep_invalid()
    {
        return invalid_frequency(sweep_frequency_bits());
    }

    bool invalid_frequency(int frequency_bits) const
    {
        return (frequency_bits < 0) || (frequency_bits > 2047);
    }

    int16_t sweep_frequency_bits()
    {
        m_swept_down = !m_sweep_up;

        AGE_ASSERT(!invalid_frequency(m_frequency_bits));
        int shifted = m_frequency_bits >> m_shift;
        int sweeped = m_frequency_bits + (m_sweep_up ? shifted : -shifted);

        AGE_ASSERT((sweeped >= int16_t_min) && (sweeped <= int16_t_max));
        return static_cast<int16_t>(sweeped);
    }

    void set_period_counter()
    {
        m_period_counter = (m_period == 0) ? 8 : m_period;
    }

    int16_t m_frequency_bits = 0;
    int8_t m_period = 0;
    int8_t m_shift = 0;
    bool m_sweep_up = true;

    bool m_skip_first_step = false;
    bool m_sweep_enabled = false;
    bool m_swept_down = false;
    int8_t m_period_counter = 0;
};





class gb_wave_generator : public gb_sample_generator<gb_wave_generator>
{
public:

    gb_wave_generator();
    gb_wave_generator(int8_t frequency_counter_shift, uint8_t wave_pattern_index_mask);

    uint8_t get_wave_pattern_index() const;

    void set_low_frequency_bits(uint8_t nrX3);
    void set_high_frequency_bits(uint8_t nrX4);

    void reset_wave_pattern_index();
    void set_wave_pattern_byte(unsigned offset, uint8_t value);
    void reset_duty_counter();
    void set_wave_pattern_duty(uint8_t nrX1);

    void init_duty_waveform_position(int16_t frequency_bits, int sample_offset, uint8_t index);

    uint8_t next_wave_sample();

protected:

    int16_t get_frequency_bits() const;

    void set_frequency_bits(int16_t frequency_bits);

private:

    int8_t m_frequency_counter_shift;
    int16_t m_frequency_bits = 0;

    uint8_t m_index_mask;
    uint8_t m_index = 0;

    uint8_array<32> m_wave_pattern;
};





class gb_noise_generator : public gb_sample_generator<gb_noise_generator>
{
public:

    gb_noise_generator();

    uint8_t read_nrX3() const;

    void write_nrX3(uint8_t nrX3);
    void init_generator();

    uint8_t next_wave_sample();

private:

    uint8_t m_nrX3 = 0;
    bool m_7steps = false;
    bool m_allow_shift = true;
    uint16_t m_lfsr = 0;
};





class gb_length_counter
{
public:

    gb_length_counter(uint8_t counter_mask);

    void write_nrX1(uint8_t nrX1);
    bool write_nrX4(uint8_t nrX4, bool last_fs_step_ticked_lc);
    bool tick();

private:

    const uint8_t m_counter_mask;

    int16_t m_counter = 0;
    bool m_counter_enabled = false;
};

} // namespace age



#endif // AGE_GB_SOUND_UTILITIES_HPP
