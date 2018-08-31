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

#include <ios> // std::hex

#include <age_debug.hpp>

#include "age_gb_timer.hpp"

#if 0
#define LOG(x) AGE_GB_CYCLE_LOG(x)
#else
#define LOG(x)
#endif

namespace age {

constexpr uint8_t gb_tac_start_timer = 0x04;

}





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

age::uint8_t age::gb_timer::read_tma() const
{
    return m_tma;
}

age::uint8_t age::gb_timer::read_tac() const
{
    return m_tac;
}

age::uint8_t age::gb_timer::read_tima()
{
    // if the timer is running we have to calculate the current TIMA value
    if (m_tima_running)
    {
        // get the current TIMA value based on the counter
        int tima = m_tima_counter.get_current_value();

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
        m_tima = tima & 0xFF;
    }

    // return the current TIMA value
    LOG("reading tima 0x" << std::hex << (int)m_tima);
    return m_tima;
}

age::uint8_t age::gb_timer::read_div() const
{
    return (m_counter.get_current_value() >> 6) & 0xFF;
}



//---------------------------------------------------------
//
//   i/o port writes.
//
//---------------------------------------------------------

void age::gb_timer::write_tma(uint8_t value)
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
    LOG("tma set to 0x" << std::hex << (int)m_tma);
}

void age::gb_timer::write_tima(uint8_t value)
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
            LOG("tima set to 0x" << std::hex << (int)value);
        }
    }

    // if the timer is not running, just store the value
    else
    {
        m_tima = value;
        LOG("tima set to 0x" << std::hex << (int)m_tima);
    }
}

void age::gb_timer::write_tac(uint8_t value)
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
    int tima = m_tima;

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
            int new_increment_bit = m_tima_counter.get_trigger_bit(value);
            tima = check_for_early_increment(new_increment_bit);
        }
    }

    // update timer configuration
    m_tac = value | 0xF8;
    LOG("tac set to 0x" << std::hex << (int)m_tac);

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

void age::gb_timer::write_div(uint8_t)
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
        int tima = check_for_early_increment(0);
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

void age::gb_timer::set_back_cycles(int offset)
{
    m_counter.set_back_cycles(offset);
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

int age::gb_timer::check_for_early_increment(int new_increment_bit)
{
    AGE_ASSERT(m_tima_running);

    // calculate the current TIMA value
    int tima = read_tima();

    // check for early increment
    // (the bit must change from high to low for that)
    int current_increment_bit = m_tima_counter.get_trigger_bit(m_tac);
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
    int tima = read_tima();

    // calculate the cycle offset
    int tima_offset = 0x100 - tima;
    int cycle_offset = m_tima_counter.get_cycle_offset(tima_offset);

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
