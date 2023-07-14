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

#ifndef AGE_GB_SOUND_LENGTH_COUNTER_HPP
#define AGE_GB_SOUND_LENGTH_COUNTER_HPP

//!
//! \file
//!

#include <age_types.hpp>

#include <cassert>



namespace age
{
    constexpr uint8_t gb_nrX4_initialize     = 0x80;
    constexpr uint8_t gb_nrX4_length_counter = 0x40;



    template<typename BaseClass>
    class gb_length_counter : public BaseClass
    {
    public:
        explicit gb_length_counter(uint8_t counter_mask, const gb_sound_logger* logger)
            : BaseClass(logger),
              m_counter_mask(counter_mask)
        {
            assert(m_counter_mask >= 0x3F);
        }

        void write_nrX1(uint8_t nrX1)
        {
            uint8_t invNrX1 = ~nrX1;
            m_counter       = (invNrX1 & m_counter_mask) + 1;
            BaseClass::log() << "set length counter = " << m_counter;
        }

        void init_length_counter(uint8_t nrX4, bool immediate_decrement)
        {
            int  decrement           = 0;
            bool new_counter_enabled = (nrX4 & gb_nrX4_length_counter) > 0;
            if (new_counter_enabled)
            {
                decrement = immediate_decrement ? 1 : 0;

                if (!m_counter_enabled && (m_counter > 0))
                {
                    m_counter -= decrement;
                    if (m_counter == 0)
                    {
                        BaseClass::deactivate();
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
                    BaseClass::log() << "decrement length counter (" << (m_counter + 1) << " => " << m_counter << ")";
                    if (m_counter == 0)
                    {
                        BaseClass::deactivate();
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



#endif // AGE_GB_SOUND_LENGTH_COUNTER_HPP
