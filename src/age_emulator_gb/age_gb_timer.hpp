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

#ifndef AGE_GB_TIMER_HPP
#define AGE_GB_TIMER_HPP

//!
//! \file
//!

#include <age_non_copyable.hpp>
#include <age_types.hpp>

#include "age_gb_core.hpp"



namespace age
{

class gb_common_counter : public non_copyable
{
public:

    gb_common_counter(const gb_core &core);

    uint get_current_value() const;
    uint get_cycle_offset(uint for_counter_offset) const;

    void reset();
    void switch_double_speed_mode();

private:

    const gb_core &m_core;
    uint m_counter_origin = 0;
    uint m_cycle_shift = 2;
};



class gb_tima_counter
{
public:

    gb_tima_counter(gb_common_counter &counter);

    uint get_current_value() const;
    uint get_cycle_offset(uint for_tima_offset) const;
    uint get_trigger_bit(uint8 for_tac) const;
    uint get_past_tima_counter(uint8 for_tima) const;

    void set_tima(uint tima);
    void set_frequency(uint tac);

private:

    static uint calculate_counter_shift(uint8 for_tac);

    const gb_common_counter &m_counter;
    uint m_tima_origin = 0;
    uint m_counter_shift = 2;
};



class gb_timer : public non_copyable
{
public:

    uint8 read_div() const;
    uint8 read_tima();
    uint8 read_tma() const;
    uint8 read_tac() const;

    void write_div(uint8 value);
    void write_tima(uint8 value);
    void write_tma(uint8 value);
    void write_tac(uint8 value);

    void timer_overflow();
    void switch_double_speed_mode();

    gb_timer(gb_core &core);

private:

    uint check_for_early_increment(uint new_increment_bit);
    void schedule_timer_overflow();

    uint8 m_tima = 0;
    uint8 m_tma = 0;
    uint8 m_tac = 0;

    gb_core &m_core;
    gb_common_counter m_counter = {m_core};
    gb_tima_counter m_tima_counter = {m_counter};
    bool m_tima_running = false;
    uint m_last_overflow_counter = 0;
};

} // namespace age



#endif // AGE_GB_TIMER_HPP
