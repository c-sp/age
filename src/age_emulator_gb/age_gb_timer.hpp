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

    int get_current_value() const;
    int get_cycle_offset(int for_counter_offset) const;

    void reset();
    void switch_double_speed_mode();
    void set_back_cycles(int offset);

private:

    const gb_core &m_core;
    int m_counter_origin = 0;
    int8_t m_cycle_shift = 2;
};



class gb_tima_counter
{
public:

    gb_tima_counter(gb_common_counter &counter);

    int get_current_value() const;
    int get_cycle_offset(int for_tima_offset) const;
    int get_trigger_bit(uint8_t for_tac) const;
    int get_past_tima_counter(uint8_t for_tima) const;

    void set_tima(int tima);
    void set_frequency(uint8_t tac);

private:

    static int8_t calculate_counter_shift(uint8_t for_tac);

    const gb_common_counter &m_counter;
    int m_tima_origin = 0;
    int8_t m_counter_shift = 2;
};



class gb_timer : public non_copyable
{
public:

    uint8_t read_div() const;
    uint8_t read_tima();
    uint8_t read_tma() const;
    uint8_t read_tac() const;

    void write_div(uint8_t value);
    void write_tima(uint8_t value);
    void write_tma(uint8_t value);
    void write_tac(uint8_t value);

    void timer_overflow();
    void switch_double_speed_mode();
    void set_back_cycles(int offset);

    gb_timer(gb_core &core);

private:

    int check_for_early_increment(int new_increment_bit);
    void schedule_timer_overflow();

    gb_core &m_core;
    gb_common_counter m_counter = {m_core};
    gb_tima_counter m_tima_counter = {m_counter};
    int m_last_overflow_counter = 0;
    bool m_tima_running = false;

    uint8_t m_tima = 0;
    uint8_t m_tma = 0;
    uint8_t m_tac = 0;
};

} // namespace age



#endif // AGE_GB_TIMER_HPP
