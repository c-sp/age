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

#include <algorithm>

#include "age_gb_events.hpp"



age::gb_events::gb_events(const gb_clock& clock)
    : m_clock(clock)
{
    std::for_each(begin(m_active_events),
                  end(m_active_events),
                  [&](auto& aev) {
                      aev = gb_no_clock_cycle;
                  });
}



void age::gb_events::schedule_event(gb_event event, int clock_cycle_offset)
{
    AGE_ASSERT(clock_cycle_offset >= 0)
    AGE_ASSERT(event != gb_event::none);

    int ev_idx   = to_underlying(event);
    int ev_cycle = m_clock.get_clock_cycle() + clock_cycle_offset;

    //! \todo optimize this, the vector is sorted

    // event already scheduled?
    if (m_active_events[ev_idx] != gb_no_clock_cycle)
    {
        for (auto it = begin(m_events); it != end(m_events); ++it)
        {
            if (it->m_struct.m_event == event)
            {
                it->m_struct.m_clock_cycle = ev_cycle;
                break;
            }
        }
    }

    // schedule new event
    else
    {
        scheduled_event ev{{.m_event = event, .m_clock_cycle = ev_cycle}};
        m_events.push_back(ev);
    }

    // sort descending so that we can pop_back() the next event
    std::sort(begin(m_events),
              end(m_events),
              [](const scheduled_event& a, const scheduled_event& b) {
                  return a.m_int > b.m_int;
              });
    m_active_events[ev_idx] = ev_cycle;

    log() << "event " << to_underlying(event) << " scheduled"
          << ", " << m_events.size() << " event(s) scheduled total";
}



void age::gb_events::remove_event(gb_event event)
{
    // is the event scheduled?
    uint8_t idx = to_underlying(event);
    if (m_active_events[idx] == gb_no_clock_cycle)
    {
        return;
    }

    // remove scheduled event
    for (auto it = begin(m_events); it != end(m_events); ++it)
    {
        if (it->m_struct.m_event == event)
        {
            m_events.erase(it);
            break;
        }
    }
    m_active_events[idx] = gb_no_clock_cycle;
    log() << "event " << to_underlying(event) << " removed"
          << ", " << m_events.size() << " event(s) still scheduled";
}



int age::gb_events::get_event_cycle(gb_event event) const
{
    return m_active_events[to_underlying(event)];
}



age::gb_event age::gb_events::poll_event()
{
    // no event scheduled
    if (m_events.empty())
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
    m_active_events[to_underlying(event)] = gb_no_clock_cycle;

    log() << "event " << to_underlying(event) << " polled"
          << ", " << m_events.size() << " event(s) still scheduled";

    return event;
}



void age::gb_events::set_back_clock(int clock_cycle_offset)
{
    std::for_each(begin(m_events),
                  end(m_events),
                  [&](auto& ev) {
                      gb_set_back_clock_cycle(ev.m_struct.m_clock_cycle, clock_cycle_offset);
                  });

    std::for_each(begin(m_active_events),
                  end(m_active_events),
                  [&](auto& ev) {
                      gb_set_back_clock_cycle(ev, clock_cycle_offset);
                  });
}
