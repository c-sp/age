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

#define CLOG(log) AGE_GB_CLOG(AGE_GB_CLOG_TIMER)(log)



//---------------------------------------------------------
//
//   update state for current clock cycle
//
//---------------------------------------------------------

void age::gb_timer::trigger_interrupt()
{
    AGE_ASSERT(m_clk_timer_zero != gb_no_clock_cycle);

    // update the timer state in case the overflow has
    // not been handled yet
    bool interrupt_triggered = update_timer_state();
    AGE_ASSERT(m_clk_last_overflow != gb_no_clock_cycle);

    // While it does not really matter if we trigger the interrupt
    // twice on the same clock cycle,
    // it does look confusing while debugging.
    if (!interrupt_triggered)
    {
        m_interrupts.trigger_interrupt(gb_interrupt::timer);
        // schedule next interrupt event
        set_clk_timer_zero(m_clk_timer_zero);
    }
}



void age::gb_timer::update_state()
{
    if (m_clk_timer_zero != gb_no_clock_cycle)
    {
        update_timer_state();
    }
}

bool age::gb_timer::update_timer_state()
{
    AGE_ASSERT(m_clock_shift > 0);
    AGE_ASSERT(m_clk_timer_zero != gb_no_clock_cycle);

    const int clk_current = m_clock.get_clock_cycle();
    const int clks_tima = clk_current - m_clk_timer_zero;
    const int tima = clks_tima >> m_clock_shift;

    // no timer overflow => just update TIMA
    if (tima < 0x100) {
        m_tima = tima & 0xFF;
        return false;
    }
    // one or more timer overflows occurred

    // reload with TMA after first overflow
    m_tima = m_tma + ((tima - 0x100) % (0x100 - m_tma));

    // calculate clock cycle of last overflow
    int clk_last_inc = clk_current - (clks_tima & ((1 << m_clock_shift) - 1));
    int incs_since_overflow = m_tima - m_tma;
    m_clk_last_overflow = clk_last_inc - (incs_since_overflow << m_clock_shift);
    AGE_ASSERT(m_clk_last_overflow <= clk_current);

    AGE_GB_CLOG_TIMER("timer overflow:");
    AGE_GB_CLOG_TIMER("    * last overflow on clock cycle " << m_clk_last_overflow);
    AGE_GB_CLOG_TIMER("    * TIMA = " << AGE_LOG_HEX(tima));
    AGE_GB_CLOG_TIMER("    * setting TIMA = " << AGE_LOG_HEX8(m_tima)
                      << " (TMA = " << AGE_LOG_HEX8(m_tma) << ")");

    // re-initialize timer
    start_timer();

    // If the overflow happened at least one machine cycle ago,
    // we have to trigger an interrupt here as the timer interrupt
    // event is going to be re-scheduled.
    if (clk_current > m_clk_last_overflow)
    {
        m_interrupts.trigger_interrupt(gb_interrupt::timer);
        return true;
    }
    return false;
}



void age::gb_timer::on_div_reset(int old_div_offset)
{
    if (m_clk_timer_zero == gb_no_clock_cycle)
    {
        return;
    }
    update_timer_state();

    // identify the least significant timer clock bit
    int clks_per_inc = 1 << m_clock_shift;

    // identify the clock bit that triggers a timer increment
    // when going low
    int trigger_bit = clks_per_inc >> 1;
    AGE_ASSERT(trigger_bit > 0);

    // calculate old and new div-aligned clock
    int current_clk = m_clock.get_clock_cycle();
    int old_clock = current_clk + old_div_offset;
    int new_clock = current_clk + m_div.get_div_offset();

    // calculate the number of clock cycles until the next
    // timer increment for both clocks
    int old_next_inc = clks_per_inc - (old_clock & (clks_per_inc - 1));
    int new_next_inc = clks_per_inc - (new_clock & (clks_per_inc - 1));

    int old_trigger_bit = old_clock & trigger_bit;
    int new_trigger_bit = new_clock & trigger_bit;

    int clk_adjust = (old_trigger_bit && !new_trigger_bit)
            // trigger bit goes low
            //      => immediate timer increment
            //      => time-to-overflow shortened
            ? -old_next_inc
            // trigger bit not going low
            //      => time-to-overflow increased
            : new_next_inc - old_next_inc;

    int clk_overflow = m_clk_timer_zero + 0x100 * clks_per_inc;
    AGE_ASSERT(clk_overflow > current_clk);
    clk_overflow += clk_adjust;
    AGE_ASSERT(clk_overflow >= current_clk);

    AGE_GB_CLOG_TIMER("timer at DIV reset:");
    AGE_GB_CLOG_TIMER("    * old clock bits " << AGE_LOG_HEX16(old_clock & 0xFFFF));
    AGE_GB_CLOG_TIMER("    * new clock bits " << AGE_LOG_HEX16(new_clock & 0xFFFF));
    AGE_GB_CLOG_TIMER("    * next increment (old) in " << old_next_inc << " clock cycles");
    AGE_GB_CLOG_TIMER("    * next increment (new) in " << new_next_inc << " clock cycles");
    AGE_GB_CLOG_TIMER("    * +/- clock cycles until overflow: " << clk_adjust);
    AGE_GB_CLOG_TIMER("    * overflow on clock cycle " << clk_overflow);

    set_clk_timer_zero(m_clk_timer_zero + clk_adjust);
}



void age::gb_timer::set_back_clock(int clock_cycle_offset)
{
    AGE_GB_SET_BACK_CLOCK(m_clk_timer_zero, clock_cycle_offset);
    AGE_GB_SET_BACK_CLOCK(m_clk_last_overflow, clock_cycle_offset);
}



//---------------------------------------------------------
//
//   start / stop / adjust timer
//
//---------------------------------------------------------

void age::gb_timer::start_timer()
{
    // clock cycles per timer increment
    int clk_shift = get_clock_shift(m_tac);
    int clks_per_inc = 1 << clk_shift;

    // align timer with DIV
    int current_clk = m_clock.get_clock_cycle();
    int clk_div_aligned = current_clk + m_div.get_div_offset();

    // clock cycles until next increment
    int clks_next_inc = clks_per_inc - (clk_div_aligned % clks_per_inc);

    // clock cycles until timer overflow
    int overflow_incs = 0x100 - m_tima;
    int clks_until_overflow = clks_next_inc + ((overflow_incs - 1) << clk_shift);

    AGE_GB_CLOG_TIMER("timer started/updated:");
    AGE_GB_CLOG_TIMER("    * clock shift " << clk_shift
                      << " (increment each " << clks_per_inc << " clock cycles)");
    AGE_GB_CLOG_TIMER("    * next increment in " << clks_next_inc << " clock cycles ("
                      << (current_clk + clks_next_inc) << ")");
    AGE_GB_CLOG_TIMER("    * overflow in " << clks_until_overflow << " clock cycles ("
                      << (current_clk + clks_until_overflow) << ")");

    m_clock_shift = clk_shift;
    set_clk_timer_zero(current_clk + clks_until_overflow - 0x100 * clks_per_inc);
}



void age::gb_timer::stop_timer()
{
    // timer not active?
    if (m_clk_timer_zero == gb_no_clock_cycle)
    {
        return;
    }

    // Stop the active timer.
    // Stopping the timer while the clock bit triggering timer increments is
    // high causes an immediate timer increment.
    int clks_per_inc = 1 << m_clock_shift;
    int trigger_bit = clks_per_inc >> 1;

    int timer_clock = m_clock.get_clock_cycle() + m_div.get_div_offset();
    if (timer_clock & trigger_bit)
    {
        m_clk_timer_zero -= clks_per_inc;
        update_state(); //! \todo may this trigger an interrupt?
    }

    AGE_GB_CLOG_TIMER("timer stopped");
    m_clock_shift = 0;
    m_clk_timer_zero = gb_no_clock_cycle;
    m_clk_last_overflow = gb_no_clock_cycle;
    m_events.remove_event(gb_event::timer_interrupt);
}



void age::gb_timer::set_clk_timer_zero(int new_clk_timer_zero)
{
    AGE_ASSERT(new_clk_timer_zero != gb_no_clock_cycle);
    m_clk_timer_zero = new_clk_timer_zero;

    // The interrupt is raised with a delay of one machine cycle.
    //
    // Gambatte tests:
    //      tima/tc01_1stopstart_offset1_irq_2
    //      tima/tc01_1stopstart_offset2_irq_1
    //
    // Don't overwrite an interrupt event that has not been
    // handled yet.
    int clk_current = m_clock.get_clock_cycle();

    if (m_clk_last_overflow == clk_current)
    {
        AGE_ASSERT(m_events.get_event_cycle(gb_event::timer_interrupt) != gb_no_clock_cycle);
        return;
    }

    // If this was called during gb_timer.trigger_interrupt(),
    // the interrupt event is not scheduled any more which is the
    // reasons for the missing assertion.

    // Schedule the interrupt event if the last overflow
    // was more than 1 machine cycle ago.
    int clk_irq = m_clk_timer_zero + (0x100 << m_clock_shift) + m_clock.get_machine_cycle_clocks();
    AGE_ASSERT(clk_irq >= clk_current);

    int clks_irq = clk_irq - clk_current;
    m_events.schedule_event(gb_event::timer_interrupt, clks_irq);
}
