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

#ifndef AGE_GB_SOUND_HPP
#define AGE_GB_SOUND_HPP

//!
//! \file
//!

#include <array>

#include <age_debug.hpp>
#include <age_types.hpp>
#include <pcm/age_pcm_sample.hpp>

#include "age_gb_core.hpp"
#include "age_gb_sound_utilities.hpp"



namespace age
{

constexpr unsigned gb_channel_1 = 0;
constexpr unsigned gb_channel_2 = 1;
constexpr unsigned gb_channel_3 = 2;
constexpr unsigned gb_channel_4 = 3;

constexpr const uint8_array<4> gb_channel_bit =
{{
     0x01, 0x02, 0x04, 0x08
 }};



class gb_sound
{
    AGE_DISABLE_COPY(gb_sound);

public:

    gb_sound(gb_core &core, pcm_vector &samples);

    uint8_t read_nr10() const;
    uint8_t read_nr11() const;
    uint8_t read_nr12() const;
    uint8_t read_nr13() const;
    uint8_t read_nr14() const;
    uint8_t read_nr21() const;
    uint8_t read_nr22() const;
    uint8_t read_nr23() const;
    uint8_t read_nr24() const;
    uint8_t read_nr30() const;
    uint8_t read_nr31() const;
    uint8_t read_nr32() const;
    uint8_t read_nr33() const;
    uint8_t read_nr34() const;
    uint8_t read_nr41() const;
    uint8_t read_nr42() const;
    uint8_t read_nr43() const;
    uint8_t read_nr44() const;
    uint8_t read_nr50() const;
    uint8_t read_nr51() const;
    uint8_t read_nr52();

    void write_nr10(uint8_t value);
    void write_nr11(uint8_t value);
    void write_nr12(uint8_t value);
    void write_nr13(uint8_t value);
    void write_nr14(uint8_t value);
    void write_nr21(uint8_t value);
    void write_nr22(uint8_t value);
    void write_nr23(uint8_t value);
    void write_nr24(uint8_t value);
    void write_nr30(uint8_t value);
    void write_nr31(uint8_t value);
    void write_nr32(uint8_t value);
    void write_nr33(uint8_t value);
    void write_nr34(uint8_t value);
    void write_nr41(uint8_t value);
    void write_nr42(uint8_t value);
    void write_nr43(uint8_t value);
    void write_nr44(uint8_t value);
    void write_nr50(uint8_t value);
    void write_nr51(uint8_t value);
    void write_nr52(uint8_t value);

    uint8_t read_wave_ram(unsigned offset);
    void write_wave_ram(unsigned offset, uint8_t value);

    void update_state();
    void set_back_cycles(int offset);



private:

    void frame_sequencer_step(int at_cycle);
    void generate_samples(int until_cycle);
    void set_wave_ram_byte(unsigned offset, uint8_t value);

    template<unsigned channel>
    void activate_channel()
    {
        uint8_t channel_bit = gb_channel_bit[channel];
        m_nr52 |= channel_bit;
        calculate_channel_multiplier<channel>();
    }

    template<unsigned channel>
    void deactivate_channel()
    {
        uint8_t channel_bit = gb_channel_bit[channel];
        m_nr52 &= ~channel_bit;
        calculate_channel_multiplier<channel>();
    }

    template<unsigned channel>
    void tick_length_counter()
    {
        if (m_length_counter[channel].tick())
        {
            deactivate_channel<channel>();
        }
    }

    template<unsigned channel>
    void calculate_channel_multiplier()
    {
        // calculate channel multiplier
        // (s01 is the right channel, s02 is the left channel)
        // this value includes:
        //     - s0x volume
        //     - channel active flag
        //     - channel to s0x routing
        uint8_t channel_bit = gb_channel_bit[channel];

        int16_t channel_s01 = ((m_nr52 & channel_bit) > 0) ? (m_nr50 & 7) + 1 : 0;
        int16_t channel_s02 = ((m_nr52 & channel_bit) > 0) ? ((m_nr50 >> 4) & 7) + 1 : 0;

        channel_s01 = ((m_nr51 & channel_bit) > 0) ? channel_s01 : 0;
        channel_s02 = (((m_nr51 >> 4) & channel_bit) > 0) ? channel_s02 : 0;

        pcm_sample channel_multiplier = {channel_s02, channel_s01};
        set_channel_multiplier<channel>(channel_multiplier.m_stereo_sample);
    }

    template<unsigned channel> void set_channel_multiplier(uint32_t)
    {
        AGE_ASSERT(false); // see implementations below this class
    }



    // common members

    gb_core &m_core;
    const bool m_is_cgb;

    pcm_vector &m_samples;
    int m_last_generate_samples_cycle = 0;
    int m_next_frame_sequencer_cycle = 0;
    int8_t m_next_frame_sequencer_step = 0;

    // channel control

    uint8_t m_nr50 = 0, m_nr51 = 0, m_nr52 = 0xF0;
    bool m_master_on = true;
    int16_t m_s01_volume = 0;
    int16_t m_s02_volume = 0;

    // channels

    std::array<gb_length_counter, 4> m_length_counter = {{ {0x3F}, {0x3F}, {0xFF}, {0x3F} }};

    uint8_t m_nr10 = 0, m_nr11 = 0, m_nr14 = 0;
    gb_frequency_sweep<gb_volume_sweep<gb_wave_generator>> m_c1;

    uint8_t m_nr21 = 0, m_nr24 = 0;
    gb_volume_sweep<gb_wave_generator> m_c2;

    uint8_t m_nr30 = 0, m_nr32 = 0, m_nr34 = 0;
    uint8_array<16> m_c3_wave_ram;
    gb_wave_generator m_c3 = {0, 31};

    uint8_t m_nr44 = 0;
    gb_volume_sweep<gb_noise_generator> m_c4;
};



template<> inline void gb_sound::set_channel_multiplier<0>(uint32_t multiplier)
{
    m_c1.set_channel_multiplier(multiplier);
}

template<> inline void gb_sound::set_channel_multiplier<1>(uint32_t multiplier)
{
    m_c2.set_channel_multiplier(multiplier);
}

template<> inline void gb_sound::set_channel_multiplier<2>(uint32_t multiplier)
{
    m_c3.set_channel_multiplier(multiplier);
}

template<> inline void gb_sound::set_channel_multiplier<3>(uint32_t multiplier)
{
    m_c4.set_channel_multiplier(multiplier);
}

} // namespace age



#endif // AGE_GB_SOUND_HPP
