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

#ifndef AGE_SPEED_CALCULATOR_HPP
#define AGE_SPEED_CALCULATOR_HPP

//!
//! \file
//!

#include <array>

#include <age_types.hpp>



namespace age
{

template<uint _history_size>
class speed_calculator
{
    static_assert(_history_size > 0, "history size must be > 0");

public:

    speed_calculator()
    {
        clear();
    }

    void clear()
    {
        m_next_value = 0;
        m_sum = {0, 0};
        m_history.fill(m_sum);
    }

    void add_value(uint64 ticks, uint64 realtime_ticks)
    {
        if ((ticks > 0) && (realtime_ticks > 0))
        {
            // subtract old value from sum
            m_sum -= m_history[m_next_value];

            // add new value
            sample s = {ticks, realtime_ticks};
            m_history[m_next_value] = s;
            m_sum += s;

            // update counter
            ++m_next_value;
            if (m_next_value >= m_history.size())
            {
                m_next_value = 0;
            }
        }
    }

    uint get_speed_percent(uint64 reference_ticks, uint64 reference_realtime_ticks)
    {
        uint result;

        if (m_sum.m_ticks > 0)
        {
            uint64 t = m_sum.m_ticks * reference_realtime_ticks;
            uint64 rt = m_sum.m_realtime_ticks * reference_ticks;
            result = static_cast<uint>(t * 1000 / rt);

            // convert per-mille to percent by rounding to the nearest whole number
            uint mod = result % 10;
            result = result / 10 + ((mod >= 5) ? 1 : 0);
        }
        else
        {
            result = 0;
        }

        return result;
    }

private:

    struct sample
    {
        sample()
            : sample(0, 0)
        {}

        sample(uint64 ticks, uint64 realtime_ticks)
            : m_ticks(ticks),
              m_realtime_ticks(realtime_ticks)
        {}

        sample& operator+=(const sample &other)
        {
            m_ticks += other.m_ticks;
            m_realtime_ticks += other.m_realtime_ticks;
            return *this;
        }

        sample& operator-=(const sample &other)
        {
            m_ticks -= other.m_ticks;
            m_realtime_ticks -= other.m_realtime_ticks;
            return *this;
        }

        uint64 m_ticks;
        uint64 m_realtime_ticks;
    };

    uint m_next_value = 0;
    sample m_sum;
    std::array<sample, _history_size> m_history;
};

} // namespace age



#endif // AGE_SPEED_CALCULATOR_HPP
