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

constexpr uint8_t gb_channel_bit(unsigned channel)
{
    return static_cast<uint8_t>(0x01 << channel);
}



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

    gb_core &m_core;
    const bool m_is_cgb;

    pcm_vector &m_samples;
    int m_last_sample_cycle = 0;
    int m_next_apu_event_cycle = 0;
    int8_t m_next_frame_sequencer_step = 0;
    bool m_delayed_disable_c1 = false;

    // channel control

    uint8_t m_nr50 = 0x77, m_nr51 = 0xF3, m_nr52 = 0x81;
    bool m_master_on = true;

    // channels

    std::array<gb_length_counter, 4> m_length_counter = {{ {0x3F}, {0x3F}, {0xFF}, {0x3F} }};

    uint8_t m_nr10 = 0, m_nr11 = 0x80, m_nr14 = 0;
    gb_frequency_sweep<gb_volume_envelope<gb_wave_generator>> m_c1;

    uint8_t m_nr21 = 0, m_nr24 = 0;
    gb_volume_envelope<gb_wave_generator> m_c2;

    uint8_t m_nr30 = 0, m_nr32 = 0, m_nr34 = 0;
    uint8_array<16> m_c3_wave_ram;
    gb_wave_generator m_c3 = {0, 31};

    uint8_t m_nr44 = 0;
    gb_volume_envelope<gb_noise_generator> m_c4;



    // channel template methods

    int apu_event(int at_cycle);
    void generate_samples(int until_cycle);
    void set_wave_ram_byte(unsigned offset, uint8_t value);

    template<unsigned channel>
    inline void init_channel()
    {
        uint8_t channel_bit = gb_channel_bit(channel);
        m_nr52 |= channel_bit;
        calculate_channel_multiplier<channel>();
    }

    template<unsigned channel>
    inline void deactivate_channel()
    {
        // AGE_GB_CYCLE_LOG("deactivate channel " << (channel + 1));
        uint8_t channel_bit = gb_channel_bit(channel);
        m_nr52 &= ~channel_bit;
        calculate_channel_multiplier<channel>();
    }

    template<unsigned channel>
    inline void length_counter_tick()
    {
        if (m_length_counter[channel].tick())
        {
            deactivate_channel<channel>();
        }
    }

    template<unsigned channel>
    inline void length_counter_write_nrX4(uint8_t value)
    {
        gb_length_counter &lc = m_length_counter[channel];
        bool last_fs_step_ticked_lc = m_next_frame_sequencer_step & 1;

        bool deactivate = lc.write_nrX4(value, last_fs_step_ticked_lc);
        if (deactivate)
        {
            deactivate_channel<channel>();
        }
    }

    template<unsigned channel>
    inline void length_counter_write_nrX1(uint8_t value)
    {
        // length counter always writable for DMG
        if (m_master_on || !m_is_cgb)
        {
            update_state();
            m_length_counter[channel].write_nrX1(value);
        }
    }

    template<unsigned channel>
    inline void calculate_channel_multiplier()
    {
        // calculate channel multiplier
        // (s01 is the right channel, s02 is the left channel)
        // this value includes:
        //     - s0x volume
        //     - channel active flag
        //     - channel to s0x routing
        int channel_bit = gb_channel_bit(channel);
        bool channel_active = (m_nr52 & channel_bit) != 0;

        int16_t volume_s01 = channel_active ? (m_nr50 & 7) + 1 : 0;
        int16_t volume_s02 = channel_active ? ((m_nr50 >> 4) & 7) + 1 : 0;

        volume_s01 = ((m_nr51 & channel_bit) > 0) ? volume_s01 : 0;
        volume_s02 = (((m_nr51 >> 4) & channel_bit) > 0) ? volume_s02 : 0;

        pcm_sample channel_multiplier = {volume_s02, volume_s01};
        set_channel_multiplier<channel>(channel_multiplier.m_stereo_sample);
    }

    template<unsigned>
    inline void set_channel_multiplier(uint32_t)
    {
        AGE_ASSERT(false); // implementation for each channel see below
    }
};

template<> inline void gb_sound::set_channel_multiplier<gb_channel_1>(uint32_t multiplier)
{
    m_c1.set_channel_multiplier(multiplier);
}

template<> inline void gb_sound::set_channel_multiplier<gb_channel_2>(uint32_t multiplier)
{
    m_c2.set_channel_multiplier(multiplier);
}

template<> inline void gb_sound::set_channel_multiplier<gb_channel_3>(uint32_t multiplier)
{
    m_c3.set_channel_multiplier(multiplier);
}

template<> inline void gb_sound::set_channel_multiplier<gb_channel_4>(uint32_t multiplier)
{
    m_c4.set_channel_multiplier(multiplier);
}

} // namespace age



#endif // AGE_GB_SOUND_HPP
