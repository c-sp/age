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

#ifndef AGE_GB_TIMER_HPP
#define AGE_GB_TIMER_HPP

//!
//! \file
//!

#include "age_gb_core.hpp"



namespace age
{

class gb_common_counter : public non_copyable
{
public:

    gb_common_counter(const gb_core &core);

    uint current_value() const;
    uint cycle_offset(uint for_counter_offset) const;

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

    uint current_value() const;
    uint counter_offset(uint for_tima_offset) const;
    uint counts_since_increment() const;

    void set_tima(uint tima);
    void set_frequency(uint tac);
    uint early_increment();

private:

    gb_common_counter &m_counter;

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

//    bool tima_check_unexpected_increment() const;
//    void tima_set_origin();
    uint calculate_current_tima();
    uint check_for_early_increment();
    void schedule_timer_overflow();

//    uint copy_tma(uint current_tima, uint current_cycle);
//    void calculate_tima_cycle_shift();
//    uint calculate_next_overflow_cycle();
//    uint refresh_last_update_cycle(uint current_cycle);
//    void schedule_tima_event();

    uint8 m_tima = 0;
    uint8 m_tma = 0;
    uint8 m_tac = 0;

    gb_core &m_core;
    gb_common_counter m_counter = {m_core};
    gb_tima_counter m_tima_counter = {m_counter};

//    uint m_div_cycle_offset = 0;
//    uint m_div_tick_cycle_shift = 8;

    bool m_tima_running = false;
//    uint m_tima_counter_shift = 0;
//    uint m_tima_origin = 0;
//    uint m_tima_tick_cycle_shift = 0;
//    uint m_tima_tick_cycle_shift_offset = 4;
//    uint m_tima_next_overflow = 0;
//    uint m_tima_last_update_cycle = 0;
//    uint m_tma_copy_cycle = gb_no_cycle;
};

} // namespace age



#endif // AGE_GB_TIMER_HPP
