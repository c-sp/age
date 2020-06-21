//
// Copyright 2019 Christoph Sprenger
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

#include <algorithm>

#include "age_gb_events.hpp"



age::gb_events::gb_events(const gb_clock &clock)
    : m_clock(clock)
{
}



void age::gb_events::schedule_event(gb_event event, int event_clock_cycle)
{
    AGE_ASSERT(event_clock_cycle >= m_clock.get_clock_cycle());
    AGE_ASSERT(event != gb_event::none);

    scheduled_event ev{{.m_event = event, .m_clock_cycle = event_clock_cycle}};

    //! \todo optimize this, the vector is sorted
    m_events.push_back(ev);
    // sort descending so that we can pop_back() the next event
    std::sort(
        m_events.begin(),
        m_events.end(),
        [](const scheduled_event &a, const scheduled_event &b){
            return a.m_int > b.m_int;
        }
    );
}



void age::gb_events::remove_event(gb_event event)
{
    // is the event scheduled?
    uint8_t idx = to_integral(event);
    if (m_active_events[idx] == gb_no_clock_cycle)
    {
        return;
    }

    // remove scheduled event
    for (auto it = m_events.begin(); it != m_events.end(); ++it)
    {
        if (it->m_struct.m_event == event)
        {
            m_events.erase(it);
            break;
        }
    }
    m_active_events[idx] = gb_no_clock_cycle;
}



int age::gb_events::get_event_cycle(gb_event event) const
{
    return m_active_events[to_integral(event)];
}



age::gb_event age::gb_events::poll_event()
{
    // no event scheduled
    if (!m_events.size())
    {
        return gb_event::none;
    }

    // check event clock cycle
    int current_clk = m_clock.get_clock_cycle();
    if (current_clk < m_events.back().m_struct.m_clock_cycle)
    {
        return gb_event::none;
    }

    // poll event
    gb_event event = m_events.back().m_struct.m_event;
    m_events.pop_back();
    return event;
}



void age::gb_events::set_back_clock(int clock_cycle_offset)
{
    for (auto it = m_events.begin(); it != m_events.end(); ++it)
    {
        AGE_GB_SET_BACK_CLOCK(it->m_struct.m_clock_cycle, clock_cycle_offset);
    }
    for (auto it = m_active_events.begin(); it != m_active_events.end(); ++it)
    {
        AGE_GB_SET_BACK_CLOCK(*it, clock_cycle_offset);
    }
}
