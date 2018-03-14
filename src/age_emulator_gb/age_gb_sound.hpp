//
// Copyright (c) 2010-2017 Christoph Sprenger
//
// This file is part of AGE ("Another Gameboy Emulator").
// <https://gitlab.com/csprenger/AGE>
//
// AGE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// AGE is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with AGE.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AGE_GB_SOUND_HPP
#define AGE_GB_SOUND_HPP

//!
//! \file
//!

#include <array>

#include <age_pcm_sample.hpp>
#include <age_debug.hpp>
#include <age_non_copyable.hpp>
#include <age_types.hpp>

#include "age_gb_core.hpp"
#include "age_gb_sound_utilities.hpp"



namespace age
{

class gb_sound : public non_copyable
{
public:

    gb_sound(gb_core &core, pcm_vector &samples);

    uint8 read_nr10() const;
    uint8 read_nr11() const;
    uint8 read_nr12() const;
    uint8 read_nr13() const;
    uint8 read_nr14() const;
    uint8 read_nr21() const;
    uint8 read_nr22() const;
    uint8 read_nr23() const;
    uint8 read_nr24() const;
    uint8 read_nr30() const;
    uint8 read_nr31() const;
    uint8 read_nr32() const;
    uint8 read_nr33() const;
    uint8 read_nr34() const;
    uint8 read_nr41() const;
    uint8 read_nr42() const;
    uint8 read_nr43() const;
    uint8 read_nr44() const;
    uint8 read_nr50() const;
    uint8 read_nr51() const;
    uint8 read_nr52() const;

    void write_nr10(uint8 value);
    void write_nr11(uint8 value);
    void write_nr12(uint8 value);
    void write_nr13(uint8 value);
    void write_nr14(uint8 value);
    void write_nr21(uint8 value);
    void write_nr22(uint8 value);
    void write_nr23(uint8 value);
    void write_nr24(uint8 value);
    void write_nr30(uint8 value);
    void write_nr31(uint8 value);
    void write_nr32(uint8 value);
    void write_nr33(uint8 value);
    void write_nr34(uint8 value);
    void write_nr41(uint8 value);
    void write_nr42(uint8 value);
    void write_nr43(uint8 value);
    void write_nr44(uint8 value);
    void write_nr50(uint8 value);
    void write_nr51(uint8 value);
    void write_nr52(uint8 value);

    uint8 read_wave_ram(uint offset);
    void write_wave_ram(uint offset, uint8 value);

    void frame_sequencer_cycle();
    void generate_samples();



private:

    template<uint channel>
    void activate_channel()
    {
        uint8 channel_bit = gb_channel_bit[channel];
        m_nr52 |= channel_bit;
        calculate_channel_multiplier<channel>();
    }

    template<uint channel>
    void deactivate_channel()
    {
        uint8 channel_bit = gb_channel_bit[channel];
        m_nr52 &= ~channel_bit;
        calculate_channel_multiplier<channel>();
    }

    template<uint channel>
    void cycle_length_counter()
    {
        if (m_length_counter[channel].cycle())
        {
            generate_samples();
            deactivate_channel<channel>();
        }
    }

    template<uint channel>
    void calculate_channel_multiplier()
    {
        // calculate channel multiplier
        // (s01 is the right channel, s02 is the left channel)
        // this value includes:
        //     - s0x volume
        //     - channel active flag
        //     - channel to s0x routing
        uint8 channel_bit = gb_channel_bit[channel];

        int16 channel_s01 = ((m_nr52 & channel_bit) > 0) ? (m_nr50 & 7) + 1 : 0;
        int16 channel_s02 = ((m_nr52 & channel_bit) > 0) ? ((m_nr50 >> 4) & 7) + 1 : 0;

        channel_s01 = ((m_nr51 & channel_bit) > 0) ? channel_s01 : 0;
        channel_s02 = (((m_nr51 >> 4) & channel_bit) > 0) ? channel_s02 : 0;

        pcm_sample channel_multiplier = {channel_s02, channel_s01};
        set_channel_multiplier<channel>(channel_multiplier.m_stereo_sample);
    }

    template<uint channel> void set_channel_multiplier(uint32)
    {
        AGE_ASSERT(false); // see implementations below this class
    }

    void set_wave_ram_byte(uint offset, uint8 value);



    // common members

    gb_core &m_core;
    const bool m_is_cgb;

    uint m_last_generate_samples_cycle = 0;
    pcm_vector &m_samples;

    uint m_next_frame_sequencer_step = 0;
    bool m_next_frame_sequencer_step_odd = false;

    // channel control

    uint8 m_nr50 = 0, m_nr51 = 0, m_nr52 = 0xF0;
    bool m_master_on = true;
    int16 m_s01_volume = 0;
    int16 m_s02_volume = 0;

    // channels

    std::array<gb_length_counter, 4> m_length_counter = {{ {0x3F}, {0x3F}, {0xFF}, {0x3F} }};

    uint8 m_nr11 = 0, m_nr14 = 0;
    gb_frequency_sweep<gb_volume_sweep<gb_wave_generator>> m_c1;

    uint8 m_nr21 = 0, m_nr24 = 0;
    gb_volume_sweep<gb_wave_generator> m_c2;

    uint8 m_nr30 = 0, m_nr32 = 0, m_nr34 = 0;
    uint8_array<16> m_c3_wave_ram;
    uint m_c3_last_wave_access_cycle = 0;
    gb_wave_generator m_c3 = {1, 31};

    uint8 m_nr44 = 0;
    gb_volume_sweep<gb_noise_generator> m_c4;
};



template<> inline void gb_sound::set_channel_multiplier<0>(uint32 multiplier)
{
    m_c1.set_channel_multiplier(multiplier);
}

template<> inline void gb_sound::set_channel_multiplier<1>(uint32 multiplier)
{
    m_c2.set_channel_multiplier(multiplier);
}

template<> inline void gb_sound::set_channel_multiplier<2>(uint32 multiplier)
{
    m_c3.set_channel_multiplier(multiplier);
}

template<> inline void gb_sound::set_channel_multiplier<3>(uint32 multiplier)
{
    m_c4.set_channel_multiplier(multiplier);
}

} // namespace age



#endif // AGE_GB_SOUND_HPP
