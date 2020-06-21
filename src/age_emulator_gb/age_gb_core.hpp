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

#ifndef AGE_GB_CORE_HPP
#define AGE_GB_CORE_HPP

//!
//! \file
//!

#include <array>
#include <map>

#include <age_debug.hpp>
#include <age_types.hpp>
#include <emulator/age_gb_types.hpp>

#include "common/age_gb_device.hpp"
#include "common/age_gb_clock.hpp"
#include "common/age_gb_events.hpp"



namespace age
{



class gb_core
{
    AGE_DISABLE_COPY(gb_core);

public:

    gb_core(const gb_device &device, gb_clock &clock);

    void insert_event(int clock_cycle_offset, gb_event event);
    void remove_event(gb_event event);
    gb_event poll_event();
    int get_event_cycle(gb_event event) const;
    void set_back_clock(int clock_cycle_offset);

private:

    class gb_events
    {
    public:

        gb_events();

        int get_event_cycle(gb_event event) const;

        gb_event poll_event(int current_cycle);
        void insert_event(int for_cycle, gb_event event);

        void set_back_clock(int offset);

    private:

        // the following two members should be replaced by some better suited data structure(s)
        std::multimap<int, gb_event> m_events;
        std::array<int, to_integral(gb_event::none)> m_event_cycle;
    };

    const gb_device &m_device;
    gb_clock &m_clock;

    gb_events m_events;
};

} // namespace age



#endif // AGE_GB_CORE_HPP
