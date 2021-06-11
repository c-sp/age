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

#ifndef AGE_GB_SOUND_GENERATE_WAVE_HPP
#define AGE_GB_SOUND_GENERATE_WAVE_HPP

//!
//! \file
//!

#include <age_debug.hpp>
#include <age_types.hpp>

#include "age_gb_sound_channel.hpp"
#include "age_gb_sound_generate.hpp"

#include <algorithm>



namespace age
{

    template<int ChannelId>
    class gb_wave_generator : public gb_sample_generator<gb_wave_generator<ChannelId>>,
                              public gb_sound_channel<ChannelId>
    {
    public:
        explicit gb_wave_generator(const gb_sound_logger* clock)
            : gb_sound_channel<ChannelId>(clock)
        {
            std::fill(begin(m_wave_pattern), end(m_wave_pattern), 0);
            calculate_frequency_timer_period();
        }

        [[nodiscard]] bool next_sample_reads_wave_ram() const
        {
            return gb_sample_generator<gb_wave_generator<ChannelId>>::get_frequency_timer() == 1;
        }

        [[nodiscard]] bool wave_ram_just_read() const
        {
            return m_wave_ram_just_read;
        }

        [[nodiscard]] uint8_t get_wave_pattern_index() const
        {
            return m_index;
        }



        void set_low_frequency_bits(uint8_t nrX3)
        {
            m_frequency_low = nrX3;
            calculate_frequency_timer_period();
        }

        void set_high_frequency_bits(uint8_t nrX4)
        {
            m_frequency_high = nrX4 & 7;
            calculate_frequency_timer_period();
        }



        void init_wave_pattern_position()
        {
            m_index = 0;
            gb_sample_generator<gb_wave_generator<ChannelId>>::reset_frequency_timer(3);
        }

        void set_wave_pattern_byte(unsigned offset, uint8_t value)
        {
            uint8_t first_sample  = value >> 4;
            uint8_t second_sample = value & 0x0F;

            AGE_ASSERT(offset < 16)
            offset <<= 1;
            m_wave_pattern[offset]     = first_sample;
            m_wave_pattern[offset + 1] = second_sample;
        }



        uint8_t next_wave_sample()
        {
            ++m_index;
            m_index &= 31;

            uint8_t sample       = m_wave_pattern[m_index];
            m_wave_ram_just_read = true;

            return sample;
        }

        void generate_samples(pcm_vector& buffer, int buffer_index, int samples_to_generate)
        {
            m_wave_ram_just_read = false;
            gb_sample_generator<gb_wave_generator>::generate_samples(buffer, buffer_index, samples_to_generate);
            m_wave_ram_just_read &= gb_sample_generator<gb_wave_generator<ChannelId>>::frequency_timer_just_reloaded();
        }



    protected:
        void calculate_frequency_timer_period()
        {
            int frequency = m_frequency_low + (m_frequency_high << 8);
            AGE_ASSERT((frequency >= 0) && (frequency < 2048))
            int samples = 2048 - frequency;
            gb_sample_generator<gb_wave_generator<ChannelId>>::set_frequency_timer_period(samples);
        }

    private:
        uint8_array<32> m_wave_pattern{};
        uint8_t         m_frequency_low      = 0;
        uint8_t         m_frequency_high     = 0;
        uint8_t         m_index              = 0;
        bool            m_wave_ram_just_read = false;
    };

} // namespace age



#endif // AGE_GB_SOUND_GENERATE_WAVE_HPP
