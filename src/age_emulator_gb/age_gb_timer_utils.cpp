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

#include "age_gb_timer.hpp"

#if 0
#define COUNTER_LOG(x) AGE_GB_CYCLE_LOG(x)
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

age::gb_common_counter::gb_common_counter(const gb_core &core)
    : m_core(core)
{
}

age::uint age::gb_common_counter::current_value() const
{
    uint counter = m_core.get_oscillation_cycle() >> m_cycle_shift;
    counter -= m_counter_origin;
    return counter;
}

age::uint age::gb_common_counter::cycle_offset(uint for_counter_offset) const
{
    AGE_ASSERT(for_counter_offset > 0);

    uint cycle = m_core.get_oscillation_cycle();

    uint counter = cycle >> m_cycle_shift;
    counter += for_counter_offset;

    uint offset = (counter << m_cycle_shift) - cycle;
    return offset;
}



void age::gb_common_counter::reset()
{
    m_counter_origin = m_core.get_oscillation_cycle() >> m_cycle_shift;
    COUNTER_LOG("DIV/TIMA counter reset, offset is " << m_counter_origin);
}

void age::gb_common_counter::switch_double_speed_mode()
{
    // adjust the internal counter cycle origin so that
    // the current counter value does not change when
    // switching between single and double speed

    uint counter_cycle_shift = m_core.is_double_speed() ? 1 : 2;
    if (counter_cycle_shift != m_cycle_shift)
    {
        uint counter = current_value();
        COUNTER_LOG("switching between speed modes, counter = " << counter << ", new shift = " << counter_cycle_shift);

        m_cycle_shift = counter_cycle_shift;
        reset();
        m_counter_origin -= counter;

        COUNTER_LOG("switched between speed modes, counter = " << current_value() << ", shift = " << m_cycle_shift);
    }
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

age::uint age::gb_tima_counter::current_value() const
{
    uint tima = m_counter.current_value() >> m_counter_shift;
    tima -= m_tima_origin;
    return tima;
}

age::uint age::gb_tima_counter::counter_offset(uint for_tima_offset) const
{
    AGE_ASSERT(for_tima_offset > 0);

    uint counter = m_counter.current_value();

    uint tima = counter >> m_counter_shift;
    tima += for_tima_offset;

    uint offset = (tima << m_counter_shift) - counter;
    return offset;
}

age::uint age::gb_tima_counter::counts_since_increment() const
{
    uint mask = (1 << m_counter_shift) - 1;
    uint masked = m_counter.current_value() & mask;
    return masked;
}



void age::gb_tima_counter::set_tima(uint tima)
{
    // by using a "tima origin" instead of a "counter origin" we
    // automatically ignore the lower counter bits
    // (the TIMA is incremented every time a specific bit of
    // the counter goes low)
    m_tima_origin = m_counter.current_value() >> m_counter_shift;
    m_tima_origin -= tima;

    TIMA_LOG("tima origin set to 0x" << std::hex << m_tima_origin);
}



void age::gb_tima_counter::set_frequency(uint tac)
{
    // preserve the current TIMA value during the frequency change
    uint tima = current_value();

    // calculate the number of bits the counter value has to
    // be shifted to get the TIMA value
    //
    //  00 (4096 Hz):      machine_cycle >> 10  ( / 1024)
    //  01 (262144 Hz):    machine_cycle >> 4   ( / 16)
    //  10 (65536 Hz):     machine_cycle >> 6   ( / 64)
    //  11 (16384 Hz):     machine_cycle >> 8   ( / 256)
    //  internal counter:  machine_cycle >> 2
    //
    //  (the above numbers are valid only when running at single speed)
    m_counter_shift = ((tac - 1) & 0x03) << 1;
    m_counter_shift += 2;

    AGE_ASSERT((m_counter_shift >= 2) && (m_counter_shift <= 8));

    set_tima(tima);
}



age::uint age::gb_tima_counter::early_increment()
{
    AGE_ASSERT(m_counter_shift > 1);

    uint counter = m_counter.current_value();
    uint half_tima = (counter >> (m_counter_shift - 1)) & 1;
    m_tima_origin -= half_tima;

    TIMA_LOG("early tima increment: " << half_tima);
    TIMA_LOG("tima origin = 0x" << std::hex << m_tima_origin);

    return current_value();
}
