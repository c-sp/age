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

#ifndef AGE_GB_SOUND_GENERATOR_HPP
#define AGE_GB_SOUND_GENERATOR_HPP

//!
//! \file
//!

#include <age_debug.hpp>
#include <age_types.hpp>

#include "age_gb_sound_channel.hpp"

#include <algorithm> // std::min



namespace age
{

    template<typename DERIVED_TYPE>
    class gb_sample_generator
    {
    public:
        void generate_samples(pcm_vector& buffer, int buffer_index, int samples_to_generate)
        {
            AGE_ASSERT((buffer_index >= 0) && (samples_to_generate > 0))
            AGE_ASSERT(int_max - samples_to_generate >= buffer_index)
            AGE_ASSERT(m_frequency_timer_period > 0)
            AGE_ASSERT(m_frequency_timer >= 0)

            uint32_t channel_multiplier = static_cast<DERIVED_TYPE*>(this)->get_multiplier();
            AGE_ASSERT((channel_multiplier & 0xFFFFU) <= 8)
            AGE_ASSERT((channel_multiplier >> 16) <= 8)

            for (int samples_remaining = samples_to_generate; samples_remaining > 0;)
            {
                // write output samples until we have to retrieve the next wave sample
                int samples = std::min(samples_remaining, m_frequency_timer);

                // the result might be slightly distorted by overflow from
                // lower sample into upper sample
                // (ignored since it's just +1/-1)
                uint32_t output_sample = m_output_value * channel_multiplier;

                for (int max = buffer_index + samples; buffer_index < max; ++buffer_index)
                {
                    auto idx = static_cast<unsigned>(buffer_index);

                    // result might be slightly distorted by overflow from
                    // lower sample into upper sample
                    // (ignored since it's just +1/-1)
                    buffer[idx].m_stereo_sample += output_sample;
                }

                samples_remaining -= samples;
                m_frequency_timer -= samples;

                // check for next wave input
                if (m_frequency_timer == 0)
                {
                    m_frequency_timer = m_frequency_timer_period;
                    m_wave_sample     = static_cast<DERIVED_TYPE*>(this)->next_wave_sample();
                    calculate_output_sample();
                }
            }

            AGE_ASSERT(m_frequency_timer > 0)
        }

        void set_volume(uint8_t volume)
        {
            AGE_ASSERT(volume <= 60)
            m_volume = volume;
            calculate_output_sample();
        }

    protected:
        void set_frequency_timer_period(int number_of_samples)
        {
            AGE_ASSERT(number_of_samples > 0)
            m_frequency_timer_period = number_of_samples;
        }

        void reset_frequency_timer(int sample_offset)
        {
            m_frequency_timer = m_frequency_timer_period + sample_offset;
        }

        [[nodiscard]] int get_frequency_timer() const
        {
            return m_frequency_timer;
        }

        [[nodiscard]] bool frequency_timer_just_reloaded() const
        {
            return m_frequency_timer == m_frequency_timer_period;
        }

    private:
        void calculate_output_sample()
        {
            AGE_ASSERT(m_wave_sample <= 15)
            AGE_ASSERT(m_volume <= 60)

            int value = m_wave_sample * m_volume * int16_t_max;
            //
            // divider:
            //
            //   / 15  ->  max amplitude
            //   / 32  ->  4 channels, SOx volume up to 8
            //   / 60  ->  combining volume 1-15 (channels 1,2,4)
            //             and volume 0, 1, 0.5, 0.25 (channel 3)
            //
            int output     = value / (15 * 32 * 60); // 15 * 32 * 60 = 28800
            m_output_value = static_cast<int16_t>(output);
        }

        int m_frequency_timer_period = 0;
        int m_frequency_timer        = 0;

        //!
        //! \brief the current wave sample
        //! (taken from duty waveform, wave ram or noise LFSR)
        //!
        uint8_t m_wave_sample = 0;
        uint8_t m_volume      = 0;

        int16_t m_output_value = 0;
    };

} // namespace age



#endif // AGE_GB_SOUND_GENERATOR_HPP
