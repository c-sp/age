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

#ifndef AGE_GB_EVENTS_HPP
#define AGE_GB_EVENTS_HPP

//!
//! \file
//!

#include <age_types.hpp>

#include "age_gb_clock.hpp"



namespace age
{

enum class gb_event : int
{
    lcd_interrupt_vblank = 0,
    lcd_interrupt_lyc = 1,
    lcd_interrupt_mode2 = 2,
    lcd_interrupt_mode0 = 3,
    serial_transfer_finished = 4,
    timer_interrupt = 5,

    start_hdma = 6,
    start_oam_dma = 7,

    none = 8 // must be the last value
};



class gb_events
{
    AGE_DISABLE_COPY(gb_events);
public:

    gb_events(const gb_clock &clock);

    void schedule_event(gb_event event, int clock_cycle_offset);
    void remove_event(gb_event event);
    int get_event_cycle(gb_event event) const;
    gb_event poll_event();
    void set_back_clock(int clock_cycle_offset);

private:

    union scheduled_event
    {
        struct
        {
            gb_event m_event;
            int m_clock_cycle;
        } m_struct;
        int64_t m_int;
    };

    const gb_clock &m_clock;

    std::array<int, to_integral(gb_event::none)> m_active_events;

    static_assert(sizeof(int) == 4, "gb_events requires int to be exactly 32 bits wide");
    std::vector<scheduled_event> m_events;
};

} // namespace age



#endif // AGE_GB_EVENTS_HPP
