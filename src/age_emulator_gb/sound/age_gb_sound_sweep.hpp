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

#ifndef AGE_GB_SOUND_SWEEP_HPP
#define AGE_GB_SOUND_SWEEP_HPP

//!
//! \file
//!

#include <age_types.hpp>

#include <cassert>



namespace age
{

    template<typename BaseClass>
    class gb_frequency_sweep : public BaseClass
    {
    public:
        using BaseClass::BaseClass;

        void write_nrX0(uint8_t nrX0)
        {
            m_period   = (nrX0 >> 4) & 7;
            m_shift    = nrX0 & 7;
            m_sweep_up = (nrX0 & 8) == 0;

            auto msg = BaseClass::log();
            msg << "configure sweep:"
                << "\n    * period = " << log_hex8(m_period)
                << "\n    * shift = " << log_hex8(m_shift)
                << "\n    * sweep " << (m_sweep_up ? "up" : "down");

            if (m_swept_down && m_sweep_up)
            {
                msg << "\n    * deactivating channel: swept down => sweep up";
                BaseClass::deactivate();
            }
        }

        bool init_frequency_sweep(bool skip_first_step)
        {
            m_skip_first_step = skip_first_step;

            m_swept_down     = false;
            m_frequency_bits = BaseClass::get_frequency_bits();

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
                BaseClass::deactivate();
            }
            return deactivate;
        }

        bool sweep_frequency()
        {
            bool deactivate = false;

            if (m_sweep_enabled && !m_skip_first_step)
            {
                assert(m_period_counter > 0);
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
                            BaseClass::set_frequency_bits(frequency_bits);
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

            assert(!invalid_frequency(m_frequency_bits));
            int shifted = m_frequency_bits >> m_shift;
            int result  = m_frequency_bits + (m_sweep_up ? shifted : -shifted);

            assert((result >= int16_t_min) && (result <= int16_t_max));
            return static_cast<int16_t>(result);
        }

        void set_period_counter()
        {
            m_period_counter = (m_period == 0) ? 8 : m_period;
        }

        int16_t m_frequency_bits = 0;
        uint8_t m_period         = 0;
        uint8_t m_shift          = 0;
        bool    m_sweep_up       = true;

        bool    m_skip_first_step = false;
        bool    m_sweep_enabled   = false;
        bool    m_swept_down      = false;
        uint8_t m_period_counter  = 0;
    };

} // namespace age



#endif // AGE_GB_SOUND_SWEEP_HPP
