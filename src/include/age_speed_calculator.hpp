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
