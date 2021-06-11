//
// Copyright 2021 Christoph Sprenger
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

#ifndef AGE_GB_SOUND_CHANNEL_HPP
#define AGE_GB_SOUND_CHANNEL_HPP

//!
//! \file
//!

#include <age_types.hpp>

#include "../common/age_gb_clock.hpp"



namespace age
{

    // the only reason this class exists is to have individual logging for each channel
    class gb_sound_logger
    {
        AGE_DISABLE_COPY(gb_sound_logger);

    public:
        explicit gb_sound_logger(const gb_clock& clock, int clk_current_state)
            : m_clock(clock),
              m_clk_current_state(clk_current_state)
        {}

        // logging code is header-only to allow compile time optimization
        [[nodiscard]] gb_log_message_stream log() const
        {
            return m_clock.log(gb_log_category::lc_sound, m_clk_current_state);
        }

    protected:
        const gb_clock& m_clock;
        int             m_clk_current_state = 0;
    };



    template<int ChannelId>
    class gb_sound_channel
    {
    public:
        explicit gb_sound_channel(const gb_sound_logger* clock) : m_clock(clock) {}

        [[nodiscard]] bool active() const
        {
            return m_active;
        }

        [[nodiscard]] uint32_t get_multiplier() const
        {
            return m_active ? m_multiplier : 0;
        }

        void activate()
        {
            if (!m_active)
            {
                log() << "channel activated";
            }
            m_active = true;
        }

        void deactivate()
        {
            if (m_active)
            {
                log() << "channel deactivated";
            }
            m_active = false;
        }

        void set_multiplier(uint8_t nr50, uint8_t shifted_nr51)
        {
            // calculate channel multiplier
            // this value includes:
            //     - SOx volume
            //     - channel to SOx "routing"

            int volume_SO1 = (nr50 & 7) + 1;
            int volume_SO2 = ((nr50 >> 4) & 7) + 1;

            volume_SO1 *= shifted_nr51 & 1;
            volume_SO2 *= (shifted_nr51 >> 4) & 1;

            // SO1 is the right channel, SO2 is the left channel
            m_multiplier = pcm_sample(static_cast<int16_t>(volume_SO2), static_cast<int16_t>(volume_SO1)).m_stereo_sample;
        }

    protected:
        // logging code is header-only to allow compile time optimization
        [[nodiscard]] gb_log_message_stream log() const
        {
            auto log = m_clock->log();
            log << "(channel " << ChannelId << ") ";
            return log;
        }

    private:
        const gb_sound_logger* m_clock;

        bool     m_active     = false;
        uint32_t m_multiplier = 0;
    };

} // namespace age



#endif // AGE_GB_SOUND_CHANNEL_HPP