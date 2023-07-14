//
// © 2020 Christoph Sprenger <https://github.com/c-sp>
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

#include "age_gb_sound_channel.hpp"
#include "age_gb_sound_generate.hpp"

#include <age_types.hpp>

#include <array>
#include <cassert>



namespace age
{
    constexpr const std::array<age::uint8_array<8>, 4> gb_waveforms_duty
        = {{
            {{0, 0, 0, 0, 0, 0, 0, 1}},
            {{1, 0, 0, 0, 0, 0, 0, 1}},
            {{1, 0, 0, 0, 0, 1, 1, 1}},
            {{0, 1, 1, 1, 1, 1, 1, 0}},
        }};



    template<int ChannelId>
    class gb_duty_generator : public gb_sample_generator<gb_duty_generator<ChannelId>>,
                              public gb_sound_channel<ChannelId>
    {
    public:
        explicit gb_duty_generator(const gb_sound_logger* logger)
            : gb_sound_channel<ChannelId>(logger)
        {
            set_frequency_bits(0);
            set_waveform_duty(0);
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

        void set_waveform_duty(uint8_t nrX1)
        {
            m_duty = nrX1 >> 6;
            gb_sound_channel<ChannelId>::log() << "waveform duty " << log_dec(m_duty) << " activated";
            //! \todo set_current_pcm_amplitude()?
        }

        void init_frequency_timer(bool align_to_1mhz)
        {
            int sample_delay = (align_to_1mhz ? 4 : 3)
                               + (gb_sound_channel<ChannelId>::device().is_cgb_e_device() ? 1 : 0);

            // frequency timer delay reduced for already active channel
            // (based on SameSuite test roms)
            if (gb_sound_channel<ChannelId>::active())
            {
                sample_delay -= 2;
            }

            gb_sample_generator<gb_duty_generator<ChannelId>>::reset_frequency_timer(sample_delay);
        }

        void init_waveform_duty_position(int16_t frequency_bits, int sample_offset, uint8_t index)
        {
            set_frequency_bits(frequency_bits);

            auto offset = sample_offset - freq_to_samples();
            gb_sample_generator<gb_duty_generator<ChannelId>>::reset_frequency_timer(offset);

            m_duty_index = index;
            gb_sound_channel<ChannelId>::log() << "set waveform duty index = " << log_dec(m_duty_index);
        }

        void set_volume(uint8_t volume)
        {
            assert(volume <= 15);
            m_volume = volume;
            gb_sample_generator<gb_duty_generator<ChannelId>>::set_current_pcm_amplitude(m_current_duty_value ? m_volume : 0);
        }

        uint8_t next_pcm_amplitude()
        {
            ++m_duty_index;
            m_duty_index &= 7;
            m_current_duty_value = gb_waveforms_duty[m_duty][m_duty_index];
            return m_current_duty_value ? m_volume : 0;
        }

    protected:
        [[nodiscard]] int16_t get_frequency_bits() const
        {
            return m_frequency_bits;
        }

        void set_frequency_bits(int frequency_bits)
        {
            assert((frequency_bits >= 0) && (frequency_bits < 2048));
            m_frequency_bits = static_cast<int16_t>(frequency_bits);
            auto samples     = freq_to_samples();

            gb_sound_channel<ChannelId>::log() << "set frequency = " << log_hex16(m_frequency_bits);

            gb_sample_generator<gb_duty_generator<ChannelId>>::set_frequency_timer_period(samples);
        }

    private:
        [[nodiscard]] int freq_to_samples() const
        {
            return (2048 - m_frequency_bits) * 2;
        }

        int16_t m_frequency_bits     = 0;
        uint8_t m_duty               = 0;
        uint8_t m_duty_index         = 0;
        uint8_t m_current_duty_value = 0; //!< used to start with value 0 on every channel init (ignoring the first duty value)
        uint8_t m_volume             = 0;
    };

} // namespace age



#endif // AGE_GB_SOUND_GENERATOR_DUTY_HPP
