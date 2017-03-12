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
#define LOG(x) AGE_GB_CYCLE_LOG(x)
#else
#define LOG(x)
#endif





//---------------------------------------------------------
//
//   constructor
//
//---------------------------------------------------------

age::gb_timer::gb_timer(gb_core &core)
    : m_core(core),
      m_counter(core)
{
    write_tac(0);
}



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
    // if the timer is running we have to calculate the current TIMA value
    if (m_tima_running)
    {
        // get the current TIMA value based on the counter
        uint tima = m_tima_counter.get_current_value();

        // handle overflow(s)
        if (tima >= 0x100)
        {
            tima = (tima - 0x100) % (0x100 - m_tma);
            tima += m_tma;

            AGE_ASSERT(tima < 0x100);
            AGE_ASSERT(tima >= m_tma); // only valid on overflow

            // propagate changes to m_tima_counter
            m_tima_counter.set_tima(tima);

            m_last_overflow_counter = m_tima_counter.get_past_tima_counter(m_tma);
            LOG("last overflow counter is " << m_last_overflow_counter
                << ", current counter is " << m_counter.get_current_value());

            // verified by mooneye-gb:
            //  loading the TIMA with TMA is delayed by 4 cycles during
            //  which the TIMA is zero
            //
            //      acceptance/timer/tima_reload
            //
            if ((tima == m_tma) && (m_last_overflow_counter == m_counter.get_current_value()))
            {
                // do NOT propagate this to m_tima_counter since it
                // is just a temporary value
                tima = 0;
            }
        }

        // save the current TIMA value
        AGE_ASSERT(tima < 0x100);
        m_tima = static_cast<uint8>(tima);
    }

    // return the current TIMA value
    LOG("reading tima 0x" << std::hex << (uint)m_tima);
    return m_tima;
}

age::uint8 age::gb_timer::read_div() const
{
    return m_counter.get_current_value() >> 6;
}



//---------------------------------------------------------
//
//   i/o port writes.
//
//---------------------------------------------------------

void age::gb_timer::write_tma(uint8 value)
{
    if (m_tima_running)
    {
        // check for pending TIMA overflows
        read_tima();

        // verified by mooneye-gb:
        //  the new TMA value is used immediately, if the TIMA
        //  is currently being reloaded.
        //
        //      acceptance/timer/tma_write_reloading
        //
        if ((m_counter.get_current_value() - m_last_overflow_counter) <= 1)
        {
            m_tima_counter.set_tima(value);
            schedule_timer_overflow();
        }
    }

    m_tma = value;
    LOG("tma set to 0x" << std::hex << (uint)m_tma);
}

void age::gb_timer::write_tima(uint8 value)
{
    if (m_tima_running)
    {
        // check for pending TIMA overflows
        read_tima();

        // verified by mooneye-gb:
        //  the write is ignored if the last TIMA increment happened
        //  one cycle before
        //
        //      acceptance/timer/tima_write_reloading
        //
        if ((m_counter.get_current_value() - m_last_overflow_counter) != 1)
        {
            m_tima_counter.set_tima(value);
            schedule_timer_overflow();
            LOG("tima set to 0x" << std::hex << (uint)value);
        }
    }

    // if the timer is not running, just store the value
    else
    {
        m_tima = value;
        LOG("tima set to 0x" << std::hex << (uint)m_tima);
    }
}

void age::gb_timer::write_tac(uint8 value)
{
    // verified by mooneye-gb & gambatte tests:
    //  A TIMA increment occurs when bit X of the counter goes low.
    //  Deactivating the timer while bit X is set has the same effect.
    //
    //      acceptance/timer/rapid_toggle
    //
    //      tima/tc00_late_stop_inc_1_outFE
    //      tima/tc00_late_stop_inc_2_outFF
    //      tima/tc00_late_stop_irq_2_outE4
    //      tima/tc00_late_stop_irq_1_outE0
    //      tima/tc00_late_stop_of_1_outFF
    //      tima/tc00_late_stop_of_2_outFE
    //
    // verified by gambatte test:
    //  When changing the timer frequency so that the "old" timer
    //  was incremented when bit X went low and the "new" timer
    //  is incremented when bit Y goes low, an immediate increment
    //  will happen for bit X currently being high and bit Y
    //  currently being low.
    //
    //      tima/tc00_tc01_late_tc00_of_2
    //

    bool new_tima_running = (value & gb_tac_start_timer) > 0;
    uint tima = m_tima;

    if (m_tima_running)
    {
        // deactivating the timer might trigger a TIMA increment
        if (!new_tima_running)
        {
            check_for_early_increment(0);
            read_tima(); // make sure m_tima is up to date
        }
        // changing the frequency might trigger a TIME increment
        else
        {
            uint new_increment_bit = m_tima_counter.get_trigger_bit(value);
            tima = check_for_early_increment(new_increment_bit);
        }
    }

    // update timer configuration
    m_tac = value | 0xF8;
    LOG("tac set to 0x" << std::hex << (uint)m_tac);

    m_tima_counter.set_frequency(m_tac);
    m_tima_running = new_tima_running;

    // if the timer is (still/now) running, propagate the current
    // TIMA value and schedule the timer overflow event
    if (m_tima_running)
    {
        AGE_ASSERT(tima <= 0x100);
        m_tima_counter.set_tima(tima);
        schedule_timer_overflow();
    }
    // if the timer is not running, cancel the timer overflow event
    else
    {
        LOG("cancelling timer overflow event");
        m_core.remove_event(gb_event::timer_overflow);
    }
}

void age::gb_timer::write_div(uint8)
{
    // verified by mooneye-gb tests:
    //  writing to DIV resets the internal counter that is used
    //  for incrementing DIV and TIMA
    //
    //      acceptance/timer/div_write
    //

    if (!m_tima_running)
    {
        m_counter.reset();
    }
    else
    {
        // verified by mooneye-gb tests:
        //  A TIMA increment occurrs when bit X of the counter goes low.
        //  Resetting the counter while bit X is set has the same effect
        //  since all bits are cleared.
        //
        //      acceptance/timer/tim00_div_trigger
        //      acceptance/timer/tim01_div_trigger
        //      acceptance/timer/tim10_div_trigger
        //      acceptance/timer/tim11_div_trigger
        //
        uint tima = check_for_early_increment(0);
        AGE_ASSERT(tima <= 0x100);

        // reset the counter and propagate the reset to m_tima_counter
        // (preserve the current TIMA value)
        m_counter.reset();
        m_tima_counter.set_tima(tima);

        // re-schedule the timer overflow event
        // (which may occur immediately if TIMA was incremented)
        schedule_timer_overflow();
    }
}



//---------------------------------------------------------
//
//   Timer utility methods
//
//---------------------------------------------------------

void age::gb_timer::timer_overflow()
{
    AGE_ASSERT(m_tima_running);

    // trigger the timer interrupt and schedule
    // the next timer overflow event
    m_core.request_interrupt(gb_interrupt::timer);
    schedule_timer_overflow();
}

void age::gb_timer::switch_double_speed_mode()
{
    // change the counter's speed
    m_counter.switch_double_speed_mode();

    // re-schedule the timer overflow event if the timer is running
    if (m_tima_running)
    {
        schedule_timer_overflow();
    }
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

age::uint age::gb_timer::check_for_early_increment(uint new_increment_bit)
{
    AGE_ASSERT(m_tima_running);

    // calculate the current TIMA value
    uint tima = read_tima();

    // check for early increment
    // (the bit must change from high to low for that)
    uint current_increment_bit = m_tima_counter.get_trigger_bit(m_tac);
    current_increment_bit &= ~new_increment_bit;

    AGE_ASSERT(current_increment_bit <= 1);
    AGE_ASSERT(new_increment_bit <= 1);

    if (current_increment_bit > 0)
    {
        ++tima;
        m_tima_counter.set_tima(tima);

        // trigger the interrupt on overflow
        AGE_ASSERT(tima <= 0x100);
        if (tima == 0x100)
        {
            m_core.request_interrupt(gb_interrupt::timer);
        }
    }

    // m_tima has to be updated after the overflow
    return tima;
}



void age::gb_timer::schedule_timer_overflow()
{
    AGE_ASSERT(m_tima_running);

    // calculate the current TIMA value
    uint tima = read_tima();

    // calculate the cycle offset
    uint tima_offset = 0x100 - tima;
    uint cycle_offset = m_tima_counter.get_cycle_offset(tima_offset);

    // verified by gambatte tests:
    //  the interrupt seems to be raised with a delay
    //  of one CPU cycle
    //
    //      tima/tc01_1stopstart_offset1_irq_2
    //      tima/tc01_1stopstart_offset2_irq_1
    //
    //! \todo this delay seems odd, but I cannot find any test rom disproving it and some gambatte tests do not work without it
    cycle_offset += m_core.get_machine_cycles_per_cpu_cycle();

    LOG("scheduling timer overflow event, cycle offset " << cycle_offset);
    m_core.insert_event(cycle_offset, gb_event::timer_overflow);
}
