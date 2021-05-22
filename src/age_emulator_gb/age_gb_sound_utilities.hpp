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

#ifndef AGE_GB_SOUND_UTILITIES_HPP
#define AGE_GB_SOUND_UTILITIES_HPP

//!
//! \file
//!

#include <algorithm>   // std::min
#include <type_traits> // std::is_base_of

#include <age_debug.hpp>
#include <age_types.hpp>
#include <pcm/age_pcm_sample.hpp>



namespace age
{

    constexpr uint8_t gb_nrX4_initialize     = 0x80;
    constexpr uint8_t gb_nrX4_length_counter = 0x40;



    class gb_sound_channel
    {
    public:
        [[nodiscard]] bool     active() const;
        [[nodiscard]] uint32_t get_multiplier() const;

        void activate();
        void deactivate();
        void set_multiplier(uint8_t nr50, uint8_t shifted_nr51);

    private:
        bool     m_active     = false;
        uint32_t m_multiplier = 0;
    };





    template<typename TYPE>
    class gb_sample_generator
    {
    public:
        void generate_samples(pcm_vector& buffer, int buffer_index, int samples_to_generate)
        {
            AGE_ASSERT((buffer_index >= 0) && (samples_to_generate > 0))
            AGE_ASSERT(int_max - samples_to_generate >= buffer_index)
            AGE_ASSERT(m_frequency_timer_period > 0)
            AGE_ASSERT(m_frequency_timer >= 0)

            uint32_t channel_multiplier = static_cast<TYPE*>(this)->get_multiplier();
            AGE_ASSERT((channel_multiplier & 0xFFFF) <= 8)
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
                    m_wave_sample     = static_cast<TYPE*>(this)->next_wave_sample();
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





    class gb_duty_source : public gb_sample_generator<gb_duty_source>,
                           public gb_sound_channel
    {
    public:
        gb_duty_source();

        void set_low_frequency_bits(uint8_t nrX3);
        void set_high_frequency_bits(uint8_t nrX4);
        void set_duty_waveform(uint8_t nrX1);
        void init_frequency_timer(int current_clock_cycle, bool double_speed);

        void init_duty_waveform_position(int16_t frequency_bits, int sample_offset, uint8_t index);

        uint8_t next_wave_sample();

    protected:
        [[nodiscard]] int16_t get_frequency_bits() const;

        void set_frequency_bits(int frequency_bits);

    private:
        int16_t m_frequency_bits = 0;
        uint8_t m_index          = 0;
        uint8_t m_duty           = 0;
    };





    class gb_wave_source : public gb_sample_generator<gb_wave_source>,
                           public gb_sound_channel
    {
    public:
        gb_wave_source();

        [[nodiscard]] bool    next_sample_reads_wave_ram() const;
        [[nodiscard]] bool    wave_ram_just_read() const;
        [[nodiscard]] uint8_t get_wave_pattern_index() const;

        void set_low_frequency_bits(uint8_t nrX3);
        void set_high_frequency_bits(uint8_t nrX4);

        void init_wave_pattern_position();
        void set_wave_pattern_byte(unsigned offset, uint8_t value);

        uint8_t next_wave_sample();

        void generate_samples(pcm_vector& buffer, int buffer_index, int samples_to_generate);

    protected:
        void calculate_frequency_timer_period();

    private:
        uint8_array<32> m_wave_pattern;
        uint8_t         m_frequency_low      = 0;
        uint8_t         m_frequency_high     = 0;
        uint8_t         m_index              = 0;
        bool            m_wave_ram_just_read = false;
    };





    class gb_noise_source : public gb_sample_generator<gb_noise_source>,
                            public gb_sound_channel
    {
    public:
        gb_noise_source();

        [[nodiscard]] uint8_t read_nrX3() const;

        void write_nrX3(uint8_t nrX3);
        void init_generator();

        uint8_t next_wave_sample();

    private:
        uint8_t  m_nrX3        = 0;
        bool     m_7steps      = false;
        bool     m_allow_shift = true;
        uint16_t m_lfsr        = 0;
    };





    template<typename TYPE>
    class gb_volume_envelope : public TYPE
    {
        static_assert(
            std::is_base_of<gb_sound_channel, TYPE>::value,
            "gb_volume_envelope must derive from gb_sound_channel");

    public:
        [[nodiscard]] uint8_t read_nrX2() const
        {
            return m_nrX2;
        }

        void write_nrX2(uint8_t nrX2)
        {
            // "zombie" update
            int volume = static_cast<uint8_t>(m_volume);
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
            m_nrX2            = nrX2;
            m_period          = m_nrX2 & 0x07;
            m_increase_volume = (m_nrX2 & 0x08) > 0;

            deactivate_if_silent();
        }

        bool init_volume_envelope(bool inc_period)
        {
            m_period_counter = (m_period == 0) ? 8 : m_period;
            m_period_counter += inc_period ? 1 : 0;

            update_volume(m_nrX2 >> 4);

            return deactivate_if_silent();
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
        bool deactivate_if_silent()
        {
            bool is_silent = !(m_nrX2 & 0xF8);
            if (is_silent)
            {
                TYPE::deactivate();
            }
            return is_silent;
        }

        void update_volume(int new_volume)
        {
            AGE_ASSERT((new_volume >= 0) && (new_volume < 0x10))
            m_volume = static_cast<int8_t>(new_volume);
            TYPE::set_volume(m_volume * 4);
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

        bool   m_increase_volume = false;
        int8_t m_period          = 0;
        int8_t m_period_counter  = 0;
        int8_t m_volume          = 0;
    };





    template<typename TYPE>
    class gb_frequency_sweep : public TYPE
    {
        static_assert(
            std::is_base_of<gb_sound_channel, TYPE>::value,
            "gb_frequency_sweep must derive from gb_sound_channel");

    public:
        void write_nrX0(uint8_t nrX0)
        {
            m_period   = static_cast<int8_t>((nrX0 >> 4) & 7);
            m_shift    = static_cast<int8_t>(nrX0 & 7);
            m_sweep_up = (nrX0 & 8) == 0;

            if (m_swept_down && m_sweep_up)
            {
                gb_sound_channel::deactivate();
            }
        }

        bool init_frequency_sweep(bool skip_first_step)
        {
            m_skip_first_step = skip_first_step;

            m_swept_down     = false;
            m_frequency_bits = TYPE::get_frequency_bits();

            // enable sweep if period or shift are set
            m_sweep_enabled = (m_period + m_shift) > 0;
            if (m_sweep_enabled)
            {
                set_period_counter();
            }

            // check for channel deactivation only, if shift is not zero
            bool deactivate = (m_shift > 0) && next_sweep_invalid();
            if (deactivate)
            {
                gb_sound_channel::deactivate();
            }
            return deactivate;
        }

        bool sweep_frequency()
        {
            bool deactivate = false;

            if (m_sweep_enabled && !m_skip_first_step)
            {
                AGE_ASSERT(m_period_counter > 0)
                --m_period_counter;
                if (m_period_counter == 0)
                {
                    if (m_period > 0)
                    {
                        auto frequency_bits = sweep_frequency_bits();
                        deactivate          = invalid_frequency(frequency_bits);

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

        [[nodiscard]] bool invalid_frequency(int frequency_bits) const
        {
            return (frequency_bits < 0) || (frequency_bits > 2047);
        }

        int16_t sweep_frequency_bits()
        {
            m_swept_down = !m_sweep_up;

            AGE_ASSERT(!invalid_frequency(m_frequency_bits))
            int shifted = m_frequency_bits >> m_shift;
            int sweeped = m_frequency_bits + (m_sweep_up ? shifted : -shifted);

            AGE_ASSERT((sweeped >= int16_t_min) && (sweeped <= int16_t_max))
            return static_cast<int16_t>(sweeped);
        }

        void set_period_counter()
        {
            m_period_counter = (m_period == 0) ? 8 : m_period;
        }

        int16_t m_frequency_bits = 0;
        int8_t  m_period         = 0;
        int8_t  m_shift          = 0;
        bool    m_sweep_up       = true;

        bool   m_skip_first_step = false;
        bool   m_sweep_enabled   = false;
        bool   m_swept_down      = false;
        int8_t m_period_counter  = 0;
    };





    template<typename TYPE>
    class gb_length_counter : public TYPE
    {
        static_assert(
            std::is_base_of<gb_sound_channel, TYPE>::value,
            "gb_length_counter must derive from gb_sound_channel");

    public:
        explicit gb_length_counter(uint8_t counter_mask)
            : m_counter_mask(counter_mask)
        {
            AGE_ASSERT(m_counter_mask >= 0x3F)
        }

        void write_nrX1(uint8_t nrX1)
        {
            m_counter = (~nrX1 & m_counter_mask) + 1;
        }

        void init_length_counter(uint8_t nrX4, bool immediate_decrement)
        {
            int8_t decrement           = 0;
            bool   new_counter_enabled = (nrX4 & gb_nrX4_length_counter) > 0;
            if (new_counter_enabled)
            {
                decrement = immediate_decrement ? 1 : 0;

                if (!m_counter_enabled && (m_counter > 0))
                {
                    m_counter -= decrement;
                    if (m_counter == 0)
                    {
                        gb_sound_channel::deactivate();
                    }
                }
            }
            m_counter_enabled = new_counter_enabled;

            // store max-length counter on channel init, if current counter is zero
            if (((nrX4 & gb_nrX4_initialize) > 0) && (m_counter == 0))
            {
                m_counter = m_counter_mask + 1 - decrement;
            }
        }

        void decrement_length_counter()
        {
            if (m_counter_enabled)
            {
                if (m_counter > 0)
                {
                    --m_counter;
                    if (m_counter == 0)
                    {
                        gb_sound_channel::deactivate();
                    }
                }
            }
        }

    private:
        uint8_t m_counter_mask;
        int16_t m_counter         = 0;
        bool    m_counter_enabled = false;
    };

} // namespace age



#endif // AGE_GB_SOUND_UTILITIES_HPP
