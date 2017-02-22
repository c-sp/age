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
#define LOG(x) DBG_GB_CYCLE_LOG(x)
#else
#define LOG(x)
#endif





//---------------------------------------------------------
//
//   i/o port reads
//
//---------------------------------------------------------

age::uint8 age::gb_timer::read_tma() const
{
    return m_tma;
}

age::uint8 age::gb_timer::read_tac() const
{
    return m_tac;
}

age::uint8 age::gb_timer::read_tima()
{
    // calculate current TIMA value, if the timer is running
    if (m_tima_running)
    {
        uint cycle = m_core.get_oscillation_cycle();

        // set TIMA to TMA, if necessary
        uint tima = copy_tma(m_tima, cycle);

        // calculate the number of ticks elapsed since the last tima calculation
        uint ticks = refresh_last_update_cycle(cycle);

        // if we had any ticks, check for overflow
        tima += ticks;
        while (tima > 0x100)
        {
            uint tmp = tima - 0x100 + m_tma;
            tima = tmp;
        }

        // TMA is copied after delay
        if (tima == 0x100)
        {
            LOG("cleared tima");
            tima = 0;
            m_tma_copy_cycle = m_tima_last_update_cycle + m_core.get_cycles_per_cpu_tick() - 1;
        }

        // set TIMA to TMA, if necessary
        tima = copy_tma(tima, cycle);

        // store new TIMA value
        m_tima = static_cast<uint8>(tima);
    }

    // just return the current TIMA value
    LOG("tima is " << (uint)m_tima);
    return m_tima;
}

age::uint8 age::gb_timer::read_div() const
{
    // calculate the current DIV value
    uint result = m_core.get_oscillation_cycle();
    result -= m_div_cycle_offset;
    result >>= m_div_tick_cycle_shift;
    return static_cast<uint8>(result);
}



//---------------------------------------------------------
//
//   i/o port writes.
//
//---------------------------------------------------------

void age::gb_timer::write_tma(uint8 value)
{
    //
    // verified by gambatte tests
    //
    // update TIMA value and (maybe) set TMA copy cycle to allow copying
    // the new TMA value set by this method (writing the TMA value at the
    // same time the TMA is copied will copy the new TMA value)
    //
    //      tc01_late_tima_tma_1_out11
    //      tc01_late_tima_tma_2_outF1
    //      tc01_late_tima_tma_3_out11
    //
    read_tima();
    m_tma = value;
    LOG("tma set to " << (uint)m_tma);
}

void age::gb_timer::write_tima(uint8 value)
{
    if (m_tima_running)
    {
        uint cycle = m_core.get_oscillation_cycle();

        //
        // verified by gambatte tests
        //
        // update TIMA value and (maybe) set TMA copy cycle
        // (setting the TMA copy cycle may overwrite the TIMA value set by
        // this method, but only if writing the TIMA occurs at the same time
        // TMA copying takes place)
        //
        //      tc01_late_tima_tma_1_out11
        //      tc01_late_tima_tma_2_outF1
        //      tc01_late_tima_tma_3_out11
        //
        read_tima();
        if (m_tma_copy_cycle + 1 != cycle)
        {
            m_tma_copy_cycle = gb_no_cycle;
        }

        // check for timer interrupt before we replace a potentially expired event
        if (cycle >= m_tima_next_overflow)
        {
            m_core.request_interrupt(gb_interrupt::timer);
        }

        // calculate new timer overflow cycle
        refresh_last_update_cycle(m_core.get_oscillation_cycle());
        m_tima = value;
        schedule_tima_event();
    }
    else
    {
        m_tima = value;
    }
}

void age::gb_timer::write_tac(uint8 value)
{
    uint cycle = m_core.get_oscillation_cycle();

    //
    // verified by gambatte tests
    //
    // If the TAC is written while the timer is already running and
    // we're currently in about the second half of it's period, the
    // timer will do an early increment (including overflow
    // interrupt, if necessary).
    //
    //      tc00_late_stop_inc_1_outFE
    //      tc00_late_stop_inc_2_outFF
    //      tc00_late_stop_irq_2_outE4
    //      tc00_late_stop_irq_1_outE0
    //      tc00_late_stop_of_1_outFF
    //      tc00_late_stop_of_2_outFE
    //
    AGE_ASSERT(m_tima_running || (m_tma_copy_cycle == gb_no_cycle));
    if (m_tima_running)
    {
        // update current TIMA
        read_tima();

        // perform timer "jump" by modifying cycle offsets
        uint offset = (1ul << (m_tima_tick_cycle_shift - 1)) + m_core.get_cycles_per_cpu_tick() - 1;
        m_tima_last_update_cycle -= offset;
        if (m_tma_copy_cycle != gb_no_cycle)
        {
            m_tma_copy_cycle -= offset;
        }

        AGE_ASSERT(m_tima_next_overflow != gb_no_cycle);
        m_tima_next_overflow -= offset;
        if (cycle >= m_tima_next_overflow)
        {
            m_core.request_interrupt(gb_interrupt::timer);
        }

        // recalculate TIMA with new cycle offsets
        read_tima();

        // switch off timer (may be switched on again below)
        m_core.remove_event(gb_event::timer_overflow);
        m_tma_copy_cycle = gb_no_cycle;
    }

    // store new TAC value
    m_tac = value | 0xF8;
    m_tima_running = (m_tac & gb_tac_start_timer) > 0;
    LOG("tac set to " << (uint)m_tac);

    // start timer, if requested
    if (m_tima_running)
    {
        calculate_tima_cycle_shift();
        m_tima_last_update_cycle = (cycle >> m_tima_tick_cycle_shift) << m_tima_tick_cycle_shift;
        schedule_tima_event();
    }
}

void age::gb_timer::write_div(uint8)
{
    //
    // verified by gambatte tests
    //
    // writing the DIV resets it to zero and restarts the counter
    //
    //      start_stop1_inc_1_out00
    //      start_stop1_inc_2_out01
    //      start_stop2_inc_1_out00
    //      start_stop2_inc_2_out01
    //
    m_div_cycle_offset = m_core.get_oscillation_cycle();
    LOG("DIV reset, 0-cycle is " << m_div_cycle_offset);
}



//---------------------------------------------------------
//
//   Timer utility methods
//
//---------------------------------------------------------

void age::gb_timer::timer_overflow()
{
    AGE_ASSERT(m_tima_running);
    AGE_ASSERT(m_core.get_oscillation_cycle() >= m_tima_next_overflow);

    m_core.request_interrupt(gb_interrupt::timer);
    m_tima_next_overflow += (0x100 - m_tma) << m_tima_tick_cycle_shift;

    uint cycle_offset = m_tima_next_overflow - m_core.get_oscillation_cycle();
    m_core.insert_event(cycle_offset, gb_event::timer_overflow);
}

void age::gb_timer::switch_double_speed_mode()
{
    bool switch_to_double_speed = m_core.is_double_speed();
    uint new_div_tick_cycle_shift = switch_to_double_speed ? 7 : 8;

    // only adjust values, if we really switched speed
    if (new_div_tick_cycle_shift != m_div_tick_cycle_shift)
    {
        uint cycle = m_core.get_oscillation_cycle();

        // adjust DIV offset to new CPU speed
        uint div_cycle_diff = cycle - m_div_cycle_offset;
        m_div_cycle_offset = cycle - (switch_to_double_speed ? div_cycle_diff >> 1 : div_cycle_diff << 1);
        m_div_tick_cycle_shift = new_div_tick_cycle_shift;

        // adjust TIMA offsets to new CPU speed
        read_tima();
        uint next_tick = m_tima_last_update_cycle + (1ul << m_tima_tick_cycle_shift);

        m_tima_tick_cycle_shift_offset = m_core.is_double_speed() ? 3 : 4;
        calculate_tima_cycle_shift();
        if (m_tima_running)
        {
            AGE_ASSERT(next_tick >= cycle);
            uint tima_cycle_diff = next_tick - cycle;

            next_tick = cycle + (switch_to_double_speed ? tima_cycle_diff >> 1 : tima_cycle_diff << 1);
            m_tima_last_update_cycle = next_tick - (1ul << m_tima_tick_cycle_shift);
            AGE_ASSERT(m_tima_last_update_cycle <= cycle);

            schedule_tima_event();
        }
    }
}



age::uint age::gb_timer::copy_tma(uint current_tima, uint current_cycle)
{
    AGE_ASSERT(m_tima_running);

    uint result = current_tima;
    if (current_cycle >= m_tma_copy_cycle)
    {
        result = m_tma;
        LOG("tima set to tma " << result);
        if (current_cycle >= m_tma_copy_cycle + m_core.get_cycles_per_cpu_tick())
        {
            m_tma_copy_cycle = gb_no_cycle;
            LOG("cleared tma copy cycle");
        }
    }
    return result;
}

void age::gb_timer::calculate_tima_cycle_shift()
{
    // when not running in CGB double speed mode:
    //      00 (4096 Hz):   machine_cycle >> 10  ( / 1024)
    //      01 (262144 Hz): machine_cycle >> 4   ( / 16)
    //      10 (65536 Hz):  machine_cycle >> 6   ( / 64)
    //      11 (16384 Hz):  machine_cycle >> 8   ( / 256)
    m_tima_tick_cycle_shift = ((m_tac - 1) & 0x03) << 1;
    m_tima_tick_cycle_shift += m_tima_tick_cycle_shift_offset;
}

age::uint age::gb_timer::refresh_last_update_cycle(uint current_cycle)
{
    AGE_ASSERT(m_tima_running);

    uint ticks = current_cycle;
    ticks -= m_tima_last_update_cycle;
    ticks >>= m_tima_tick_cycle_shift;
    m_tima_last_update_cycle += ticks << m_tima_tick_cycle_shift;
    return ticks;
}

void age::gb_timer::schedule_tima_event()
{
    AGE_ASSERT(m_tima_running);

    m_tima_next_overflow = m_tima_last_update_cycle + ((0x100 - m_tima) << m_tima_tick_cycle_shift) + m_core.get_cycles_per_cpu_tick() - 1;
    LOG("last tima update on cycle " << m_tima_last_update_cycle);
    LOG("next tima overflow on cycle " << m_tima_next_overflow);
    uint current_cycle = m_core.get_oscillation_cycle();
    AGE_ASSERT(m_tima_next_overflow > current_cycle);

    uint cycle_offset = 0;
    cycle_offset += m_tima_next_overflow - current_cycle;
    m_core.insert_event(cycle_offset, gb_event::timer_overflow);
}



//---------------------------------------------------------
//
//   constructor
//
//---------------------------------------------------------

age::gb_timer::gb_timer(gb_core &core)
    : m_core(core)
{
    write_tac(0);
}
