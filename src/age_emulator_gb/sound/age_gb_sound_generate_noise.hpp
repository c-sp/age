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

#ifndef AGE_GB_SOUND_NOISE_HPP
#define AGE_GB_SOUND_NOISE_HPP

//!
//! \file
//!

#include "age_gb_sound_channel.hpp"
#include "age_gb_sound_generate.hpp"

#include <age_types.hpp>

#include <cassert>



namespace age
{

    template<int ChannelId>
    class gb_noise_generator : public gb_sample_generator<gb_noise_generator<ChannelId>>,
                               public gb_sound_channel<ChannelId>
    {
    public:
        explicit gb_noise_generator(const gb_sound_logger* logger)
            : gb_sound_channel<ChannelId>(logger)
        {
            write_nrX3(0);
        }

        [[nodiscard]] uint8_t read_nrX3() const
        {
            return m_nrX3;
        }



        void write_nrX3(uint8_t nrX3)
        {
            m_nrX3 = nrX3;

            m_7steps      = (m_nrX3 & 0x08) > 0;
            m_allow_shift = m_nrX3 < 0xE0;

            int ratio = m_nrX3 & 0x07;
            int shift = (m_nrX3 >> 4) + 4;

            if (ratio == 0)
            {
                ratio = 1;
                --shift;
            }

            gb_sound_channel<ChannelId>::log() << "set noise ratio = " << ratio << ", shift = " << shift;

            int samples = ratio << (shift - 1);
            gb_sample_generator<gb_noise_generator<ChannelId>>::set_frequency_timer_period(samples);
        }

        void init_generator()
        {
            m_lfsr = 0x7FFF;
            gb_sound_channel<ChannelId>::log() << "initialize noise lfsr = " << log_hex16(m_lfsr);
        }



        void set_volume(uint8_t volume)
        {
            assert(volume <= 15);
            m_volume = volume;
            gb_sample_generator<gb_noise_generator<ChannelId>>::set_current_pcm_amplitude(get_current_noise_value());
        }

        uint8_t next_pcm_amplitude()
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

            return get_current_noise_value();
        }

    private:
        uint8_t get_current_noise_value()
        {
            int current_bit = m_lfsr & 1;
            return current_bit ? 0 : m_volume; // 0 => high, 1 => low
        }

        uint16_t m_lfsr        = 0;
        uint8_t  m_volume      = 0;
        uint8_t  m_nrX3        = 0;
        bool     m_7steps      = false;
        bool     m_allow_shift = true;
    };

} // namespace age



#endif // AGE_GB_SOUND_NOISE_HPP
