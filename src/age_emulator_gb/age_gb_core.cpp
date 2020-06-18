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

#include <age_debug.hpp>

#include "age_gb_core.hpp"

#if 0
#define LOG(x) AGE_GB_CLOCK_LOG(x)
#else
#define LOG(x)
#endif



age::gb_core::gb_core(const gb_device &device, gb_clock &clock)
    : m_device(device),
      m_clock(clock)
{
}



void age::gb_core::insert_event(int clock_cycle_offset, gb_event event)
{
    AGE_ASSERT(clock_cycle_offset >= 0);
    AGE_ASSERT(int_max - clock_cycle_offset >= m_clock.get_clock_cycle());
    m_events.insert_event(m_clock.get_clock_cycle() + clock_cycle_offset, event);
}

void age::gb_core::remove_event(gb_event event)
{
    m_events.insert_event(gb_no_clock_cycle, event);
}

age::gb_event age::gb_core::poll_event()
{
    return m_events.poll_event(m_clock.get_clock_cycle());
}

int age::gb_core::get_event_cycle(gb_event event) const
{
    return m_events.get_event_cycle(event);
}

void age::gb_core::set_back_clock(int clock_cycle_offset)
{
    m_events.set_back_clock(clock_cycle_offset);
}



bool age::gb_core::ongoing_dma() const
{
    return m_ongoing_dma;
}

void age::gb_core::start_dma()
{
    m_ongoing_dma = true;
}

void age::gb_core::finish_dma()
{
    m_ongoing_dma = false;
}



void age::gb_core::stop()
{
    if (m_device.is_cgb() && ((m_key1 & 0x01) > 0))
    {
        m_key1 ^= 0x81;
    }
    const bool double_speed = (m_key1 & 0x80) > 0;
    m_clock.set_double_speed(double_speed);
    LOG("double speed " << m_double_speed << ", machine cycles per cpu cycle " << m_machine_cycles_per_cpu_cycle);
    insert_event(0, gb_event::switch_double_speed);
}



age::uint8_t age::gb_core::read_key1() const
{
    return m_key1;
}

void age::gb_core::write_key1(uint8_t value)
{
    LOG(AGE_LOG_HEX(value));
    m_key1 = (m_key1 & 0xFE) | (value & 0x01);
}



//---------------------------------------------------------
//
//   gb_events class
//
//---------------------------------------------------------

age::gb_core::gb_events::gb_events()
{
    std::for_each(begin(m_event_cycle), end(m_event_cycle), [&](auto &elem)
    {
        elem = gb_no_clock_cycle;
    });
}

int age::gb_core::gb_events::get_event_cycle(gb_event event) const
{
    return m_event_cycle[to_integral(event)];
}

age::gb_event age::gb_core::gb_events::poll_event(int current_cycle)
{
    auto result = gb_event::none;

    auto itr = begin(m_events);
    if (itr != end(m_events))
    {
        if (itr->first <= current_cycle)
        {
            result = itr->second;
            m_events.erase(itr);
            m_event_cycle[to_integral(result)] = gb_no_clock_cycle;
        }
    }

    return result;
}

void age::gb_core::gb_events::insert_event(int for_cycle, gb_event event)
{
    auto event_id = to_integral(event);

    auto old_cycle = m_event_cycle[event_id];
    if (old_cycle != gb_no_clock_cycle)
    {
        auto range = m_events.equal_range(old_cycle);
        for (auto itr = range.first; itr != range.second; ++itr)
        {
            if (itr->second == event)
            {
                m_events.erase(itr);
                break;
            }
        }
    }

    if (for_cycle != gb_no_clock_cycle)
    {
        m_events.insert(std::pair<int, gb_event>(for_cycle, event));
    }
    m_event_cycle[event_id] = for_cycle;
}

void age::gb_core::gb_events::set_back_clock(int clock_cycle_offset)
{
    // find all scheduled events
    std::vector<gb_event> events_to_adjust;
    std::for_each(begin(m_events), end(m_events), [&](const auto &pair)
    {
        events_to_adjust.push_back(pair.second);
    });

    // re-schedule all found events
    std::for_each(begin(events_to_adjust), end(events_to_adjust), [&](const gb_event &event)
    {
        auto idx = to_integral(event);
        auto event_cycle = m_event_cycle[idx];
        AGE_GB_SET_BACK_CLOCK(event_cycle, clock_cycle_offset);
        insert_event(event_cycle, event);
    });
}
