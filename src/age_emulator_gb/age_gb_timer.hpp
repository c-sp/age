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

#ifndef AGE_GB_TIMER_HPP
#define AGE_GB_TIMER_HPP

//!
//! \file
//!

#include <age_types.hpp>

#include "common/age_gb_clock.hpp"
#include "common/age_gb_events.hpp"
#include "common/age_gb_interrupts.hpp"

#include "age_gb_div.hpp"



namespace age
{

class gb_timer
{
    AGE_DISABLE_COPY(gb_timer);

public:

    gb_timer(const gb_clock &clock,
             const gb_div &div,
             gb_interrupt_trigger &interrupts,
             gb_events &events);

    uint8_t read_tima();
    uint8_t read_tma() const;
    uint8_t read_tac() const;

    void write_tima(uint8_t value);
    void write_tma(uint8_t value);
    void write_tac(uint8_t value);

    void trigger_interrupt();
    void update_state();
    void on_div_reset(int old_div_offset);
    void set_back_clock(int clock_cycle_offset);

private:

    int get_clock_shift(int tac) const;
    bool update_timer_state();
    void start_timer();
    void stop_timer();
    void set_clk_timer_zero(int new_clk_timer_zero);

    const gb_clock &m_clock;
    const gb_div &m_div;
    gb_interrupt_trigger &m_interrupts;
    gb_events &m_events;

    int m_clock_shift = 0;
    int m_clk_timer_zero = gb_no_clock_cycle;
    int m_clk_last_overflow = gb_no_clock_cycle;

    uint8_t m_tima = 0;
    uint8_t m_tma = 0;
    uint8_t m_tac = 0xF8;
};

} // namespace age



#endif // AGE_GB_TIMER_HPP
