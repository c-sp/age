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

#ifndef AGE_GB_SOUND_GENERATOR_DUTY_HPP
#define AGE_GB_SOUND_GENERATOR_DUTY_HPP

//!
//! \file
//!

#include <age_debug.hpp>
#include <age_types.hpp>

#include "age_gb_sound_channel.hpp"
#include "age_gb_sound_generate.hpp"

#include <array>



namespace age
{
    constexpr const std::array<age::uint8_array<8>, 4> gb_duty_waveforms
        = {{
            {{0, 0, 0, 0, 0, 0, 0, 15}},
            {{15, 0, 0, 0, 0, 0, 0, 15}},
            {{15, 0, 0, 0, 0, 15, 15, 15}},
            {{0, 15, 15, 15, 15, 15, 15, 0}},
        }};



#define DUTY_FREQ_TO_SAMPLES ((2048 - m_frequency_bits) << 1)

    class gb_duty_generator : public gb_sample_generator<gb_duty_generator>,
                              public gb_sound_channel
    {
    public:
        explicit gb_duty_generator(const gb_sound_logger* clock)
            : gb_sound_channel(clock)
        {
            set_frequency_bits(0);
            set_duty_waveform(0);
        }

        void set_low_frequency_bits(uint8_t nrX3)
        {
            int frequency_bits = (m_frequency_bits & ~0xFF) + nrX3;
            set_frequency_bits(frequency_bits);
        }

        void set_high_frequency_bits(uint8_t nrX4)
        {
            int frequency_bits = (m_frequency_bits & 0xFF) + ((nrX4 << 8) & 0x700);
            set_frequency_bits(frequency_bits);
        }

        void set_duty_waveform(uint8_t nrX1)
        {
            m_duty = nrX1 >> 6;
        }

        void init_frequency_timer(int current_clock_cycle, bool double_speed)
        {
            // frequency timer delay on channel initialization
            // (see test rom analysis)
            //! \todo test rom analysis for ch1_duty0_pos6_to_pos7_timing_ds_X
            int sample_delay = (double_speed && !(current_clock_cycle & 2)) ? 3 : 4;
            reset_frequency_timer(sample_delay);
        }

        void init_duty_waveform_position(int16_t frequency_bits, int sample_offset, uint8_t index)
        {
            set_frequency_bits(frequency_bits);

            int offset = sample_offset - DUTY_FREQ_TO_SAMPLES;
            reset_frequency_timer(offset);

            m_index = index;
        }

        uint8_t next_wave_sample()
        {
            ++m_index;
            m_index &= 7;

            uint8_t sample = gb_duty_waveforms[m_duty][m_index];
            return sample;
        }

    protected:
        [[nodiscard]] int16_t get_frequency_bits() const
        {
            return m_frequency_bits;
        }

        void set_frequency_bits(int frequency_bits)
        {
            AGE_ASSERT((frequency_bits >= 0) && (frequency_bits < 2048))
            m_frequency_bits = static_cast<int16_t>(frequency_bits);
            int samples      = DUTY_FREQ_TO_SAMPLES;
            set_frequency_timer_period(samples);
        }

    private:
        int16_t m_frequency_bits = 0;
        uint8_t m_index          = 0;
        uint8_t m_duty           = 0;
    };

} // namespace age



#endif // AGE_GB_SOUND_GENERATOR_DUTY_HPP
