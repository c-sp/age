//
// Copyright 2020 Christoph Sprenger
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



//---------------------------------------------------------
//
//   update state for current clock cycle
//
//---------------------------------------------------------

void age::gb_timer::trigger_interrupt()
{
    AGE_ASSERT(m_clk_timer_zero != gb_no_clock_cycle)

    // update the timer state in case the overflow has
    // not been handled yet
    bool interrupt_triggered = update_timer_state();
    AGE_ASSERT(m_clk_last_overflow != gb_no_clock_cycle)

    // While it does not really matter if we trigger the interrupt
    // twice on the same clock cycle,
    // it does look confusing while debugging.
    if (!interrupt_triggered)
    {
        int clk_irq = m_clk_last_overflow + m_clock.get_machine_cycle_clocks();
        m_interrupts.trigger_interrupt(gb_interrupt::timer, clk_irq);
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
    AGE_ASSERT(m_clock_shift > 0)
    AGE_ASSERT(m_clk_timer_zero != gb_no_clock_cycle)

    const int clk_current = m_clock.get_clock_cycle();
    const int clks_tima   = clk_current - m_clk_timer_zero;
    const int tima        = clks_tima >> m_clock_shift;

    // no timer overflow => just update TIMA
    if (tima < 0x100)
    {
        m_tima = tima & 0xFF;
        return false;
    }
    // one or more timer overflows occurred

    // reload with TMA after first overflow
    m_tima = m_tma + ((tima - 0x100) % (0x100 - m_tma));

    // calculate clock cycle of last overflow
    int clk_last_inc        = clk_current - (clks_tima & ((1 << m_clock_shift) - 1));
    int incs_since_overflow = m_tima - m_tma;
    m_clk_last_overflow     = clk_last_inc - (incs_since_overflow << m_clock_shift);
    AGE_ASSERT(m_clk_last_overflow <= clk_current);

    AGE_GB_CLOG_TIMER("timer overflow:")
    AGE_GB_CLOG_TIMER("    * last overflow on clock cycle " << m_clk_last_overflow)
    AGE_GB_CLOG_TIMER("    * TIMA == " << AGE_LOG_HEX(tima))
    AGE_GB_CLOG_TIMER("    * setting TIMA = " << AGE_LOG_HEX8(m_tima)
                                              << " (TMA == " << AGE_LOG_HEX8(m_tma) << ")")

    // re-initialize timer
    start_timer();

    // If the overflow happened at least one machine cycle ago,
    // we have to trigger an interrupt here as the timer interrupt
    // event is going to be re-scheduled.
    if (clk_current > m_clk_last_overflow)
    {
        int clk_irq = m_clk_last_overflow + m_clock.get_machine_cycle_clocks();
        m_interrupts.trigger_interrupt(gb_interrupt::timer, clk_irq);
        return true;
    }
    return false;
}



void age::gb_timer::set_back_clock(int clock_cycle_offset)
{
    gb_set_back_clock_cycle(m_clk_timer_zero, clock_cycle_offset);
    gb_set_back_clock_cycle(m_clk_last_overflow, clock_cycle_offset);
}



void age::gb_timer::after_speed_change()
{
    if (m_clk_timer_zero == gb_no_clock_cycle)
    {
        AGE_GB_CLOG_TIMER("timer off at speed change => nothing to do")
        return;
    }
    update_timer_state();

    // calculate the old timer overflow clock cycle
    int clk_overflow_old = m_clk_timer_zero + 0x100 * (1 << m_clock_shift);

    int clk_current         = m_clock.get_clock_cycle();
    int clks_until_overflow = clk_overflow_old - clk_current;
    AGE_ASSERT(clks_until_overflow > 0)

    // calculate the new timer overflow clock cycle
    clks_until_overflow = m_clock.is_double_speed()
                              ? clks_until_overflow >> 1  // switched to double speed
                              : clks_until_overflow << 1; // switched to single speed

    int clk_overflow_new = clk_current + clks_until_overflow;

    // update timer state
    m_clock_shift = get_clock_shift();
    set_clk_timer_zero(clk_overflow_new - 0x100 * (1 << m_clock_shift));

    AGE_GB_CLOG_TIMER("timer at speed change:")
    AGE_GB_CLOG_TIMER("    * TIMA == " << AGE_LOG_HEX8(m_tima))
    AGE_GB_CLOG_TIMER("    * timer overflow (old) at clock cycle " << clk_overflow_old)
    AGE_GB_CLOG_TIMER("    * timer overflow (new) at clock cycle " << clk_overflow_new)
}



void age::gb_timer::after_div_reset()
{
    if (m_clk_timer_zero == gb_no_clock_cycle)
    {
        AGE_GB_CLOG_TIMER("timer off at DIV reset => nothing to do")
        return;
    }
    update_timer_state();

    // identify the least significant timer clock bit
    int clks_per_inc = 1 << m_clock_shift;

    // calculate potential immediate timer increment by div reset
    auto reset_details = m_div.calculate_reset_details(clks_per_inc);

    AGE_ASSERT((m_clk_timer_zero + 0x100 * clks_per_inc) > m_clock.get_clock_cycle())
    set_clk_timer_zero(m_clk_timer_zero + reset_details.m_clk_adjust);
    AGE_ASSERT((m_clk_timer_zero + 0x100 * clks_per_inc) >= m_clock.get_clock_cycle())

    AGE_GB_CLOG_TIMER("timer at DIV reset:")
    AGE_GB_CLOG_TIMER("    * TIMA == " << AGE_LOG_HEX8(m_tima))
    AGE_GB_CLOG_TIMER("    * next increment (old) in " << reset_details.m_old_next_increment << " clock cycles ("
                                                       << (m_clock.get_clock_cycle() + reset_details.m_old_next_increment) << ")")
    AGE_GB_CLOG_TIMER("    * next increment (new) in " << reset_details.m_new_next_increment << " clock cycles ("
                                                       << (m_clock.get_clock_cycle() + reset_details.m_new_next_increment) << ")")
    AGE_GB_CLOG_TIMER("    * +/- clock cycles until overflow: " << reset_details.m_clk_adjust)
    AGE_GB_CLOG_TIMER("    * overflow on clock cycle " << (m_clk_timer_zero + 0x100 * clks_per_inc))
}



//---------------------------------------------------------
//
//   start / stop / adjust timer
//
//---------------------------------------------------------

void age::gb_timer::start_timer()
{
    // clock cycles per timer increment
    uint8_t clk_shift    = get_clock_shift();
    int     clks_per_inc = 1 << clk_shift;

    // align timer with DIV
    int current_clk     = m_clock.get_clock_cycle();
    int clk_div_aligned = current_clk + m_div.get_div_offset();

    // clock cycles until next increment
    int clks_next_inc = clks_per_inc - (clk_div_aligned % clks_per_inc);

    // clock cycles until timer overflow
    int overflow_incs       = 0x100 - m_tima;
    int clks_until_overflow = clks_next_inc + ((overflow_incs - 1) << clk_shift);

    AGE_GB_CLOG_TIMER("timer started/updated:")
    AGE_GB_CLOG_TIMER("    * clock shift " << AGE_LOG_DEC(clk_shift)
                                           << " (increment each " << clks_per_inc << " clock cycles)")
    AGE_GB_CLOG_TIMER("    * next increment in " << clks_next_inc << " clock cycles ("
                                                 << (current_clk + clks_next_inc) << ")")
    AGE_GB_CLOG_TIMER("    * overflow in " << clks_until_overflow << " clock cycles ("
                                           << (current_clk + clks_until_overflow) << ")")

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
    int trigger_bit  = clks_per_inc >> 1;

    int timer_clock = m_clock.get_clock_cycle() + m_div.get_div_offset();
    if (timer_clock & trigger_bit)
    {
        m_clk_timer_zero -= clks_per_inc;
        update_state(); //! \todo may this trigger an interrupt?
    }

    AGE_GB_CLOG_TIMER("timer stopped")
    m_clock_shift       = 0;
    m_clk_timer_zero    = gb_no_clock_cycle;
    m_clk_last_overflow = gb_no_clock_cycle;
    m_events.remove_event(gb_event::timer_interrupt);
}



void age::gb_timer::set_clk_timer_zero(int new_clk_timer_zero)
{
    AGE_ASSERT(new_clk_timer_zero != gb_no_clock_cycle)
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
        AGE_ASSERT(m_events.get_event_cycle(gb_event::timer_interrupt) != gb_no_clock_cycle)
        return;
    }

    // If this was called during gb_timer.trigger_interrupt(),
    // the interrupt event is not scheduled any more which is the
    // reasons for this deactivated assertion.
    // AGE_ASSERT(m_events.get_event_cycle(gb_event::timer_interrupt) != gb_no_clock_cycle)

    // Schedule the interrupt event if the last overflow
    // was more than 1 machine cycle ago.
    int clk_irq = m_clk_timer_zero + (0x100 << m_clock_shift) + m_clock.get_machine_cycle_clocks();
    AGE_ASSERT(clk_irq >= clk_current)

    int clks_irq = clk_irq - clk_current;
    m_events.schedule_event(gb_event::timer_interrupt, clks_irq);
}
