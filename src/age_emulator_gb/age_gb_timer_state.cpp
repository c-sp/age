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
    AGE_ASSERT(m_clk_last_overflow <= clk_current)

    log() << "timer overflow on clock cycle " << m_clk_last_overflow
          << "\n    * TIMA == " << log_hex8(tima)
          << "\n    * setting TIMA = " << log_hex8(m_tima) << " (TMA == " << log_hex8(m_tma) << ")";

    bool interrupt_triggered = false;

    // trigger interrupt, if
    //  * the overflow happened at least one machine cycle ago
    //  * or multiple overflows occurred
    if (clk_current > m_clk_last_overflow)
    {
        int clk_irq = m_clk_last_overflow + m_clock.get_machine_cycle_clocks();
        m_interrupts.trigger_interrupt(gb_interrupt::timer, clk_irq);
        interrupt_triggered = true;
    }
    else if ((clk_current == m_clk_last_overflow) && (tima > 0x100))
    {
        m_interrupts.trigger_interrupt(gb_interrupt::timer, 0); // we don't care for the actual clock cycle any more
        interrupt_triggered = true;
        m_events.schedule_event(gb_event::timer_interrupt, m_clock.get_machine_cycle_clocks());
    }

    // re-initialize timer
    start_timer();

    return interrupt_triggered;
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
        log() << "timer off at speed change";
        return;
    }
    // DIV has been reset
    AGE_ASSERT(((m_clock.get_clock_cycle() + m_clock.get_div_offset()) % 65536) == 0)
    // state is up to date
    AGE_ASSERT(((m_clock.get_clock_cycle() - m_clk_timer_zero) >> m_clock_shift) == m_tima)

    // calculate the old timer overflow clock cycle
    int clk_overflow_old = m_clk_timer_zero + 0x100 * (1 << m_clock_shift);

    int clk_current         = m_clock.get_clock_cycle();
    int clks_until_overflow = clk_overflow_old - clk_current;
    AGE_ASSERT(clks_until_overflow > 0)

    // calculate the new timer overflow clock cycle
    clks_until_overflow = m_clock.is_double_speed()
                              ? clks_until_overflow / 2  // switched to double speed
                              : clks_until_overflow * 2; // switched to single speed

    int clk_overflow_new = clk_current + clks_until_overflow;

    auto msg = log();
    msg << "timer at speed change:"
        << "\n    * TIMA == " << log_hex8(m_tima)
        << "\n    * next increment (old) in " << log_in_clks(1 << m_clock_shift, m_clock.get_clock_cycle());

    // update timer state
    m_clock_shift = get_clock_shift();
    set_clk_timer_zero(clk_overflow_new - 0x100 * (1 << m_clock_shift));

    msg << "\n    * next increment (new) in " << log_in_clks(1 << m_clock_shift, m_clock.get_clock_cycle())
        << "\n    * timer overflow (old) at clock cycle " << clk_overflow_old
        << "\n    * timer overflow (new) at clock cycle " << clk_overflow_new;
}



void age::gb_timer::after_div_reset(bool during_stop)
{
    if (m_clk_timer_zero == gb_no_clock_cycle)
    {
        log() << "timer off at DIV reset";
        return;
    }
    // DIV has been reset
    AGE_ASSERT(((m_clock.get_clock_cycle() + m_clock.get_div_offset()) % 65536) == 0)
    // state is up to date
    AGE_ASSERT(((m_clock.get_clock_cycle() - m_clk_timer_zero) >> m_clock_shift) == m_tima)

    // identify the least significant timer clock bit
    int clks_per_inc = 1 << m_clock_shift;

    // calculate potential immediate timer increment by div reset
    auto reset_details = m_clock.get_div_reset_details(clks_per_inc);

    auto msg = log();
    msg << "timer at DIV reset:";

    // speed change glitch:
    // no immediate action by div reset on the exact first machine cycle for some timers
    if (during_stop && (reset_details.m_clk_adjust == -clks_per_inc / 2))
    {
        auto glitch = m_device.is_cgb_e_device()
                          ? (m_tac & 0x03) != 1
                          : (m_tac & 0x03) == 0;
        if (glitch)
        {
            msg << "\n    * speed change glitch: immediate increment by DIV reset not on this machine cycle";
            AGE_ASSERT(reset_details.m_new_next_increment == -reset_details.m_clk_adjust * 2)
            reset_details.m_clk_adjust = -reset_details.m_clk_adjust;
        }
    }

    AGE_ASSERT((m_clk_timer_zero + 0x100 * clks_per_inc) > m_clock.get_clock_cycle())
    set_clk_timer_zero(m_clk_timer_zero + reset_details.m_clk_adjust);
    AGE_ASSERT((m_clk_timer_zero + 0x100 * clks_per_inc) >= m_clock.get_clock_cycle())

    msg << "\n    * TIMA (old) == " << log_hex8(m_tima)
        << "\n    * TIMA (new) == " << log_hex8((m_clock.get_clock_cycle() - m_clk_timer_zero) >> m_clock_shift)
        << "\n    * next increment (old) in " << log_in_clks(reset_details.m_old_next_increment, m_clock.get_clock_cycle())
        << "\n    * next increment (new) in " << log_in_clks(reset_details.m_new_next_increment, m_clock.get_clock_cycle())
        << "\n    * +/- clock cycles until overflow: " << reset_details.m_clk_adjust
        << "\n    * overflow on clock cycle " << (m_clk_timer_zero + 0x100 * clks_per_inc);
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
    int clk_div_aligned = current_clk + m_clock.get_div_offset();

    // clock cycles until next increment
    int clks_next_inc = clks_per_inc - (clk_div_aligned % clks_per_inc);

    // clock cycles until timer overflow
    int overflow_incs       = 0x100 - m_tima;
    int clks_until_overflow = clks_next_inc + ((overflow_incs - 1) << clk_shift);

    log() << "timer started/updated"
          << "\n    * clock shift " << log_dec(clk_shift) << " (increment each " << clks_per_inc << " clock cycles)"
          << "\n    * next increment in " << log_in_clks(clks_next_inc, current_clk)
          << "\n    * overflow in " << log_in_clks(clks_until_overflow, current_clk);

    m_clock_shift = clk_shift;
    set_clk_timer_zero(current_clk + clks_until_overflow - 0x100 * clks_per_inc);
}



void age::gb_timer::stop_timer()
{
    // timer off?
    if (m_clk_timer_zero == gb_no_clock_cycle)
    {
        return;
    }

    // Stop the active timer.
    // Stopping the timer while the clock bit triggering timer increments is
    // high causes an immediate timer increment.
    int clks_per_inc = 1 << m_clock_shift;
    int trigger_bit  = clks_per_inc / 2;

    int timer_clock = m_clock.get_clock_cycle() + m_clock.get_div_offset();
    if (timer_clock & trigger_bit)
    {
        m_clk_timer_zero -= clks_per_inc;
        update_state(); //! \todo may this trigger an interrupt?
    }

    log() << "timer stopped";
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
