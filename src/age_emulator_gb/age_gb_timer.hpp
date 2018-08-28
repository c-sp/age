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

    int32_t get_current_value() const;
    int32_t get_cycle_offset(int32_t for_counter_offset) const;

    void reset();
    void switch_double_speed_mode();
    void set_back_cycles(int32_t offset);

private:

    const gb_core &m_core;
    int32_t m_counter_origin = 0;
    int8_t m_cycle_shift = 2;
};



class gb_tima_counter
{
public:

    gb_tima_counter(gb_common_counter &counter);

    int32_t get_current_value() const;
    int32_t get_cycle_offset(int32_t for_tima_offset) const;
    int32_t get_trigger_bit(uint8 for_tac) const;
    int32_t get_past_tima_counter(uint8 for_tima) const;

    void set_tima(int32_t tima);
    void set_frequency(uint8 tac);

private:

    static int8_t calculate_counter_shift(uint8 for_tac);

    const gb_common_counter &m_counter;
    int32_t m_tima_origin = 0;
    int8_t m_counter_shift = 2;
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
    void set_back_cycles(int32_t offset);

    gb_timer(gb_core &core);

private:

    int32_t check_for_early_increment(int32_t new_increment_bit);
    void schedule_timer_overflow();

    uint8 m_tima = 0;
    uint8 m_tma = 0;
    uint8 m_tac = 0;

    gb_core &m_core;
    gb_common_counter m_counter = {m_core};
    gb_tima_counter m_tima_counter = {m_counter};
    bool m_tima_running = false;
    int32_t m_last_overflow_counter = 0;
};

} // namespace age



#endif // AGE_GB_TIMER_HPP
