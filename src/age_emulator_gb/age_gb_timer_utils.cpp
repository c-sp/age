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

#include <age_debug.hpp>

#include "age_gb_timer.hpp"

#if 0
#define COUNTER_LOG(x) AGE_GB_CLOCK_LOG(x)
#else
#define COUNTER_LOG(x)
#endif

#if 0
#define TIMA_LOG(x) AGE_LOG(x)
#else
#define TIMA_LOG(x)
#endif



//---------------------------------------------------------
//
//   common counter
//
//---------------------------------------------------------

age::gb_common_counter::gb_common_counter(const gb_clock &clock)
    : m_clock(clock)
{
}

int age::gb_common_counter::get_current_value() const
{
    int counter = m_clock.get_clock_cycle() >> m_clock_shift;
    counter -= m_counter_origin;
    return counter;
}

int age::gb_common_counter::get_clock_offset(int for_counter_offset) const
{
    AGE_ASSERT(for_counter_offset > 0);

    int clk = m_clock.get_clock_cycle();

    int counter = clk >> m_clock_shift;
    counter += for_counter_offset;

    int offset = (counter << m_clock_shift) - clk;
    return offset;
}



void age::gb_common_counter::reset()
{
    m_counter_origin = m_clock.get_clock_cycle() >> m_clock_shift;
    COUNTER_LOG("DIV/TIMA counter reset, offset is " << m_counter_origin);
}

void age::gb_common_counter::switch_double_speed_mode()
{
    // preserve the current counter value during speed change
    int counter = get_current_value();
    COUNTER_LOG("switching between speed modes, counter = " << counter << ", shift = " << m_clock_shift);

    m_clock_shift = m_clock.is_double_speed() ? 1 : 2;
    reset();
    m_counter_origin -= counter;

    COUNTER_LOG("switched between speed modes, counter = " << get_current_value() << ", shift = " << m_clock_shift);
}

void age::gb_common_counter::set_back_clock(int clock_cycle_offset)
{
    AGE_ASSERT(0 == (clock_cycle_offset & (m_clock_shift - 1)));
    int counter_offset = clock_cycle_offset >> m_clock_shift;
    m_counter_origin -= counter_offset;
}



//---------------------------------------------------------
//
//   tima
//
//---------------------------------------------------------

age::gb_tima_counter::gb_tima_counter(gb_common_counter &counter)
    : m_counter(counter)
{
}

int age::gb_tima_counter::get_current_value() const
{
    int tima = m_counter.get_current_value() >> m_counter_shift;
    tima -= m_tima_origin;
    return tima;
}

int age::gb_tima_counter::get_clock_offset(int for_tima_offset) const
{
    AGE_ASSERT(for_tima_offset > 0);

    int counter = m_counter.get_current_value();
    int tima = counter >> m_counter_shift;
    tima += for_tima_offset;
    int counter_offset = (tima << m_counter_shift) - counter;

    int clock_offset = m_counter.get_clock_offset(counter_offset);
    return clock_offset;
}

int age::gb_tima_counter::get_trigger_bit(uint8_t for_tac) const
{
    int counter = m_counter.get_current_value();
    int shift = calculate_counter_shift(for_tac);
    int increment_bit = 1 & (counter >> (shift - 1));

    return increment_bit;
}

int age::gb_tima_counter::get_past_tima_counter(uint8_t for_tima) const
{
    AGE_ASSERT(for_tima <= get_current_value());

    int tima = m_tima_origin + for_tima;
    int counter = tima << m_counter_shift;

    return counter;
}



void age::gb_tima_counter::set_tima(int tima)
{
    // by using a "tima origin" instead of a "counter origin" we
    // automatically ignore the lower counter bits
    // (the TIMA is incremented every time a specific bit of
    // the counter goes low)
    m_tima_origin = m_counter.get_current_value() >> m_counter_shift;
    m_tima_origin -= tima;

    TIMA_LOG("tima origin set to " << AGE_LOG_HEX(m_tima_origin));
}



void age::gb_tima_counter::set_frequency(uint8_t tac)
{
    // preserve the current TIMA value during the frequency change
    int tima = get_current_value();
    m_counter_shift = calculate_counter_shift(tac);
    set_tima(tima);
}



age::int8_t age::gb_tima_counter::calculate_counter_shift(uint8_t for_tac)
{
    // calculate the number of bits the counter value has to
    // be shifted to get the TIMA value
    //
    //  00 (4096 Hz):      clock cycle >> 10  ( / 1024)
    //  01 (262144 Hz):    clock cycle >> 4   ( / 16)
    //  10 (65536 Hz):     clock cycle >> 6   ( / 64)
    //  11 (16384 Hz):     clock cycle >> 8   ( / 256)
    //  internal counter:  clock cycle >> 2   ( / 4)
    //
    //  (the above numbers are valid only when running at single speed)
    int counter_shift = ((for_tac - 1) & 0x03) << 1;
    counter_shift += 2;

    AGE_ASSERT((counter_shift >= 2) && (counter_shift <= 8));
    return counter_shift & 0x0F;
}
