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

#include "age_gb_events.hpp"

#include <algorithm>



//---------------------------------------------------------
//
//   gb_sorted_events
//
//---------------------------------------------------------

age::gb_sorted_events::gb_sorted_events()
{
    std::for_each(begin(m_active_events),
                  end(m_active_events),
                  [&](auto& aev) {
                      aev = gb_no_clock_cycle;
                  });
}



void age::gb_sorted_events::schedule_event(gb_event event, int for_clock_cycle)
{
    AGE_ASSERT(for_clock_cycle >= 0)
    AGE_ASSERT(event != gb_event::none)

    int ev_idx = to_underlying(event);

    //! \todo optimize this, the vector is sorted

    // event already scheduled?
    if (m_active_events[ev_idx] != gb_no_clock_cycle)
    {
        for (auto it = begin(m_events); it != end(m_events); ++it)
        {
            if (it->m_struct.m_event == event)
            {
                it->m_struct.m_clock_cycle = for_clock_cycle;
                break;
            }
        }
    }

    // schedule new event
    else
    {
        scheduled_event ev{{.m_event = event, .m_clock_cycle = for_clock_cycle}};
        m_events.push_back(ev);
    }

    // sort descending so that we can pop_back() the next event
    std::sort(begin(m_events),
              end(m_events),
              [](const scheduled_event& a, const scheduled_event& b) {
                  return a.m_int > b.m_int;
              });
    m_active_events[ev_idx] = for_clock_cycle;
}



bool age::gb_sorted_events::remove_event(gb_event event)
{
    // is the event scheduled?
    uint8_t idx = to_underlying(event);
    if (m_active_events[idx] == gb_no_clock_cycle)
    {
        return false;
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
    return true;
}



int age::gb_sorted_events::get_event_cycle(gb_event event) const
{
    return m_active_events[to_underlying(event)];
}

int age::gb_sorted_events::get_next_event_cycle() const
{
    return m_events.empty()
        ? gb_no_clock_cycle
        : m_events.back().m_struct.m_clock_cycle;
}

age::size_t age::gb_sorted_events::get_events_scheduled() const
{
    return m_events.size();
}



age::gb_event age::gb_sorted_events::poll_next_event(int for_clock_cycle)
{
    // no event scheduled
    if (m_events.empty())
    {
        return gb_event::none;
    }

    // check event clock cycle
    if (for_clock_cycle < m_events.back().m_struct.m_clock_cycle)
    {
        return gb_event::none;
    }

    // poll event
    gb_event event = m_events.back().m_struct.m_event;
    m_events.pop_back();
    m_active_events[to_underlying(event)] = gb_no_clock_cycle;
    return event;
}



void age::gb_sorted_events::set_back_clock(int clock_cycle_offset)
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



//---------------------------------------------------------
//
//   gb_events
//
//---------------------------------------------------------

age::gb_events::gb_events(const gb_clock& clock)
    : m_clock(clock)
{
}



void age::gb_events::schedule_event(gb_event event, int clock_cycle_offset)
{
    int ev_cycle = m_clock.get_clock_cycle() + clock_cycle_offset;
    gb_sorted_events::schedule_event(event, ev_cycle);

    log() << "event " << log_dec(to_underlying(event))
          << " scheduled for clock cycle " << ev_cycle
          << ", " << get_events_scheduled() << " event(s) scheduled total";
}



void age::gb_events::remove_event(gb_event event)
{
    auto removed = gb_sorted_events::remove_event(event);
    if (removed)
    {
        log() << "event " << log_dec(to_underlying(event)) << " removed"
              << ", " << get_events_scheduled() << " event(s) still scheduled";
    }
}



int age::gb_events::get_event_cycle(gb_event event) const
{
    return gb_sorted_events::get_event_cycle(event);
}

int age::gb_events::get_next_event_cycle() const
{
    return gb_sorted_events::get_next_event_cycle();
}



age::gb_event age::gb_events::poll_event()
{
    auto event = gb_sorted_events::poll_next_event(m_clock.get_clock_cycle());
    if (event != gb_event::none)
    {
        log() << "event " << to_underlying(event) << " polled"
              << ", " << get_events_scheduled() << " event(s) still scheduled";
    }
    return event;
}



void age::gb_events::set_back_clock(int clock_cycle_offset)
{
    gb_sorted_events::set_back_clock(clock_cycle_offset);
}
