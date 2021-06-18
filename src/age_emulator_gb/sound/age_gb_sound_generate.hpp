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

    template<typename DerivedClass>
    class gb_sample_generator
    {
    public:
        void generate_samples(pcm_vector& buffer, int buffer_index, int samples_to_generate)
        {
            AGE_ASSERT((buffer_index >= 0) && (samples_to_generate > 0))
            AGE_ASSERT(int_max - samples_to_generate >= buffer_index)
            AGE_ASSERT(m_frequency_timer_period > 0)
            AGE_ASSERT(m_frequency_timer >= 0)

            uint32_t channel_multiplier = static_cast<DerivedClass*>(this)->get_multiplier();
            AGE_ASSERT((channel_multiplier & 0xFFFFU) <= 8)
            AGE_ASSERT((channel_multiplier >> 16) <= 8)

            for (int samples_remaining = samples_to_generate; samples_remaining > 0;)
            {
                // write output samples until we have to update the channel's PCM amplitude
                int samples = std::min(samples_remaining, m_frequency_timer);

                uint32_t output_sample = m_output_value * channel_multiplier;

                for (int max = buffer_index + samples; buffer_index < max; ++buffer_index)
                {
                    auto idx = static_cast<unsigned>(buffer_index);
                    buffer[idx].m_stereo_sample += output_sample;
                }

                samples_remaining -= samples;
                m_frequency_timer -= samples;

                // check for next wave input
                if (m_frequency_timer == 0)
                {
                    m_frequency_timer = m_frequency_timer_period;
                    set_current_pcm_amplitude(static_cast<DerivedClass*>(this)->next_pcm_amplitude());
                }
            }

            AGE_ASSERT(m_frequency_timer > 0)
        }

        void delay_one_sample()
        {
            AGE_ASSERT(m_frequency_timer > 0)
            m_frequency_timer++;
        }

        uint8_t get_current_pcm_amplitude()
        {
            AGE_ASSERT(m_current_pcm_amplitude <= 15)
            return m_current_pcm_amplitude;
        }

    protected:
        void set_frequency_timer_period(int number_of_samples)
        {
            AGE_ASSERT(number_of_samples > 0)
            m_frequency_timer_period = number_of_samples;
            static_cast<DerivedClass*>(this)->log() << "set frequency timer period = " << m_frequency_timer_period << " samples";
        }

        void reset_frequency_timer(int sample_offset)
        {
            m_frequency_timer = m_frequency_timer_period + sample_offset;
            static_cast<DerivedClass*>(this)->log() << "set frequency timer = " << m_frequency_timer
                                                    << " samples (period == " << m_frequency_timer_period << " samples)";
        }

        [[nodiscard]] int get_frequency_timer() const
        {
            return m_frequency_timer;
        }

        [[nodiscard]] bool frequency_timer_just_reloaded() const
        {
            return m_frequency_timer == m_frequency_timer_period;
        }

        void set_current_pcm_amplitude(uint8_t current_pcm_amplitude)
        {
            AGE_ASSERT(current_pcm_amplitude <= 15)
            m_current_pcm_amplitude = current_pcm_amplitude;

            // divider:
            //   / 15  ->  max channel PCM amplitude
            //   / 32  ->  4 channels, SOx volume up to 8
            int value = m_current_pcm_amplitude * int16_t_max;
            int output = value / (15 * 32);
            AGE_ASSERT(output >= 0);
            AGE_ASSERT(output <= int16_t_max / 32);

            m_output_value = static_cast<int16_t>(output);
        }

    private:
        int m_frequency_timer_period = 0;
        int m_frequency_timer        = 0;

        int16_t m_output_value = 0;

        //! \brief the channel's current PCM amplitude
        //! (combines duty waveform, wave ram or noise LFSR bits & channel volume)
        uint8_t m_current_pcm_amplitude = 0;
    };

} // namespace age



#endif // AGE_GB_SOUND_GENERATOR_HPP
