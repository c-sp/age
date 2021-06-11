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

#ifndef AGE_GB_SOUND_VOLUME_HPP
#define AGE_GB_SOUND_VOLUME_HPP

//!
//! \file
//!

#include <age_debug.hpp>
#include <age_types.hpp>



namespace age
{

    template<typename BaseClass>
    class gb_volume_envelope : public BaseClass
    {
    public:
        using BaseClass::BaseClass;

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
            bool is_silent = (m_nrX2 & 0xF8U) == 0;
            if (is_silent)
            {
                BaseClass::deactivate();
            }
            return is_silent;
        }

        void update_volume(int new_volume)
        {
            AGE_ASSERT((new_volume >= 0) && (new_volume < 0x10))
            m_volume = static_cast<int8_t>(new_volume);
            BaseClass::set_volume(m_volume * 4);
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

} // namespace age



#endif // AGE_GB_SOUND_UTILITIES_HPP
