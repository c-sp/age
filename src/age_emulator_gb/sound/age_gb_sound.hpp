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

#ifndef AGE_GB_SOUND_HPP
#define AGE_GB_SOUND_HPP

//!
//! \file
//!

#include <age_debug.hpp>
#include <age_types.hpp>
#include <pcm/age_pcm_sample.hpp>

#include "../common/age_gb_clock.hpp"
#include "../common/age_gb_device.hpp"

#include "age_gb_sound_generate_duty.hpp"
#include "age_gb_sound_generate_noise.hpp"
#include "age_gb_sound_generate_wave.hpp"
#include "age_gb_sound_length_counter.hpp"
#include "age_gb_sound_sweep.hpp"
#include "age_gb_sound_volume.hpp"



namespace age
{
    constexpr int gb_apu_event_clock_cycles = 1 << 13;

    using gb_sound_channel1 = gb_length_counter<gb_volume_envelope<gb_frequency_sweep<gb_duty_generator>>>;
    using gb_sound_channel2 = gb_length_counter<gb_volume_envelope<gb_duty_generator>>;
    using gb_sound_channel3 = gb_length_counter<gb_wave_generator>;
    using gb_sound_channel4 = gb_length_counter<gb_volume_envelope<gb_noise_generator>>;



    class gb_sound : private gb_sound_logger
    {
        AGE_DISABLE_COPY(gb_sound);

    public:
        gb_sound(const gb_clock& clock, bool cgb_features, pcm_vector& samples);

        [[nodiscard]] uint8_t read_nr10() const;
        [[nodiscard]] uint8_t read_nr11() const;
        [[nodiscard]] uint8_t read_nr12() const;
        [[nodiscard]] uint8_t read_nr14() const;
        [[nodiscard]] uint8_t read_nr21() const;
        [[nodiscard]] uint8_t read_nr22() const;
        [[nodiscard]] uint8_t read_nr24() const;
        [[nodiscard]] uint8_t read_nr30() const;
        [[nodiscard]] uint8_t read_nr32() const;
        [[nodiscard]] uint8_t read_nr34() const;
        [[nodiscard]] uint8_t read_nr42() const;
        [[nodiscard]] uint8_t read_nr43() const;
        [[nodiscard]] uint8_t read_nr44() const;
        [[nodiscard]] uint8_t read_nr50() const;
        [[nodiscard]] uint8_t read_nr51() const;
        uint8_t               read_nr52();

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
        void    write_wave_ram(unsigned offset, uint8_t value);

        void update_state();
        void after_div_reset(bool during_stop);
        void after_speed_change();
        void set_back_clock(int clock_cycle_offset);



    private:
        [[nodiscard]] bool should_inc_period() const;
        [[nodiscard]] bool should_dec_length_counter() const;
        int                apu_event();
        void               generate_samples(int for_clk);
        void               set_wave_ram_byte(unsigned offset, uint8_t value);

        pcm_vector& m_samples;

        const bool m_cgb;
        int        m_clk_next_apu_event        = 0;
        int8_t     m_next_frame_sequencer_step = 0;
        bool       m_delayed_disable_c1        = false;
        bool       m_skip_frame_sequencer_step = false;

        // channel control

        uint8_t m_nr50 = 0x77, m_nr51 = 0xF3;
        bool    m_master_on = true;

        // channels

        uint8_t           m_nr10 = 0, m_nr11 = 0x80, m_nr14 = 0;
        gb_sound_channel1 m_c1{0x3F, this};

        uint8_t           m_nr21 = 0, m_nr24 = 0;
        gb_sound_channel2 m_c2{0x3F, this};

        uint8_t           m_nr30 = 0, m_nr32 = 0, m_nr34 = 0;
        uint8_array<16>   m_c3_wave_ram;
        gb_sound_channel3 m_c3{0xFF, this};

        uint8_t           m_nr44 = 0;
        gb_sound_channel4 m_c4{0x3F, this};
    };

} // namespace age



#endif // AGE_GB_SOUND_HPP
