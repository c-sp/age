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

namespace
{

constexpr uint8_t tac_start_timer = 0x04;

}



//---------------------------------------------------------
//
//   constructor
//
//---------------------------------------------------------

age::gb_timer::gb_timer(const gb_clock &clock,
                        const gb_div &div,
                        gb_interrupt_trigger &interrupts,
                        gb_events &events)
    : m_clock(clock),
      m_div(div),
      m_interrupts(interrupts),
      m_events(events)
{
    write_tac(0);
}



//---------------------------------------------------------
//
//   i/o ports
//
//---------------------------------------------------------

age::uint8_t age::gb_timer::read_tma() const
{
    CLOG("read TMA " << AGE_LOG_HEX8(m_tma));
    return m_tma;
}

age::uint8_t age::gb_timer::read_tac() const
{
    CLOG("read TAC " << AGE_LOG_HEX8(m_tac));
    return m_tac;
}

age::uint8_t age::gb_timer::read_tima()
{
    // if the timer is running we have to calculate the current TIMA value
    update_state();

    bool just_overflowed = m_clk_last_overflow == m_clock.get_clock_cycle();
    uint8_t result = just_overflowed ? 0 : m_tima;

    CLOG("read TIMA " << AGE_LOG_HEX8(result)
         << (just_overflowed ? " (timer just overflowed)" : ""));

    return result;
}



void age::gb_timer::write_tma(uint8_t value)
{
    update_state();
    CLOG("write TMA " << AGE_LOG_HEX8(value));
    m_tma = value;

    // the new TMA is used immediately if a timer overflow occurred at most
    // 4 clock cycles ago (CGB double speed: 2 clock cycles)
    int last_mcycle = m_clock.get_clock_cycle() - m_clock.get_machine_cycle_clocks();

    if ((m_clk_last_overflow != gb_no_clock_cycle) && (m_clk_last_overflow >= last_mcycle))
    {
        set_tima(m_tma);
    }
}

void age::gb_timer::write_tac(uint8_t value)
{
    update_state();
    CLOG("write TAC " << AGE_LOG_HEX8(value));
    m_tac = value | 0xF8;

    if (m_tac & tac_start_timer)
    {
        start_timer();
    }
    else
    {
        stop_timer();
    }
}

void age::gb_timer::write_tima(uint8_t value)
{
    update_state();

    // TIMA write is ignored if a timer overflow occurred 4 clock cycles ago
    // (CGB double speed: 2 clock cycles)
    int last_mcycle = m_clock.get_clock_cycle() - m_clock.get_machine_cycle_clocks();
    if (m_clk_last_overflow == last_mcycle)
    {
        CLOG("write TIMA " << AGE_LOG_HEX8(value)
             << " ignored due to timer overflow");
        return;
    }

    CLOG("write TIMA " << AGE_LOG_HEX8(value)
         << ", old TIMA = " << AGE_LOG_HEX8(m_tima));
    set_tima(value);
}



//---------------------------------------------------------
//
//   start / stop / change timer
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

    CLOG("starting timer:");
    CLOG("    * clock shift " << clk_shift);
    CLOG("    * increment each " << clks_per_inc << " clock cycles");
    CLOG("    * div offset " << AGE_LOG_HEX16(m_div.get_div_offset()));
    CLOG("    * next increment in " << clks_next_inc << " clock cycles");
    CLOG("    * overflow in " << clks_until_overflow << " clock cycles");

    m_clk_shift = clk_shift;
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
    int clks_per_inc = 1 << m_clk_shift;
    int trigger_bit = clks_per_inc >> 1;

    int timer_clock = m_clock.get_clock_cycle() + m_div.get_div_offset();
    if (timer_clock & trigger_bit)
    {
        m_clk_timer_zero -= clks_per_inc;
        update_state(); //! \todo may this trigger an interrupt?
    }

    m_clk_shift = 0;
    m_clk_timer_zero = gb_no_clock_cycle;
    m_clk_last_overflow = gb_no_clock_cycle;
    m_events.remove_event(gb_event::timer_overflow);
}



//---------------------------------------------------------
//
//   update state for active timer
//
//---------------------------------------------------------

void age::gb_timer::update_state()
{
    if (m_clk_timer_zero == gb_no_clock_cycle)
    {
        return;
    }

    const int clk_current = m_clock.get_clock_cycle();
    const int clks_tima = clk_current - m_clk_timer_zero;
    const int tima = clks_tima >> m_clk_shift;

    // no timer overflow => just update TIMA
    if (tima < 0x100) {
        m_tima = tima & 0xFF;
        return;
    }
    // one or more timer overflows occurred

    // reload with current TMA after first overflow
    int tima_tma = tima - 0x100;
    m_tima = m_tma + (tima_tma % (0x100 - m_tma));

    // recalculate clock cycle for TIMA=0
    //! \todo call get_clock_shift to adjust to speed change?
    int clk_adjust = (tima - m_tima) << m_clk_shift;
    set_clk_timer_zero(m_clk_timer_zero + clk_adjust);

    CLOG("timer overflow:");
    CLOG("    * TIMA = " << AGE_LOG_HEX(tima));
    CLOG("    * setting TIMA = " << AGE_LOG_HEX8(m_tima)
         << " (TMA = " << AGE_LOG_HEX8(m_tma) << ")");
    CLOG("    * next overflow on clock cycle "
         << (m_clk_timer_zero + (0x100 << m_clk_shift)));

    // trigger interrupt
    m_interrupts.trigger_interrupt(gb_interrupt::timer);

    // calculate clock cycle of last overflow
    int clk_last_inc = clk_current - (clks_tima & ((1 << m_clk_shift) - 1));
    int incs_since_overflow = m_tima - m_tma;
    m_clk_last_overflow = clk_last_inc - (incs_since_overflow << m_clk_shift);
}



void age::gb_timer::on_div_reset(int old_div_offset)
{
    update_state();
    if (m_clk_timer_zero == gb_no_clock_cycle)
    {
        return;
    }

    // identify the least significant timer clock bit
    int clks_per_inc = 1 << m_clk_shift;

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

    CLOG("active timer at DIV reset:");
    CLOG("    * old clock bits " << AGE_LOG_HEX16(old_clock & 0xFFFF));
    CLOG("    * new clock bits " << AGE_LOG_HEX16(new_clock & 0xFFFF));
    CLOG("    * next increment (old) in " << old_next_inc << " clock cycles");
    CLOG("    * next increment (new) in " << new_next_inc << " clock cycles");
    CLOG("    * +/- clock cycles until overflow: " << clk_adjust);
    CLOG("    * overflow on clock cycle " << clk_overflow);

    set_clk_timer_zero(m_clk_timer_zero + clk_adjust);
}



void age::gb_timer::set_back_clock(int clock_cycle_offset)
{
    AGE_GB_SET_BACK_CLOCK(m_clk_timer_zero, clock_cycle_offset);
    AGE_GB_SET_BACK_CLOCK(m_clk_last_overflow, clock_cycle_offset);
}



int age::gb_timer::get_clock_shift(int tac) const
{
    // 00 (4096 Hz):   clock cycle >> 10  (1024 clock cycles)
    // 01 (262144 Hz): clock cycle >> 4   (16 clock cycles)
    // 10 (65536 Hz):  clock cycle >> 6   (64 clock cycles)
    // 11 (16384 Hz):  clock cycle >> 8   (256 clock cycles)
    int clock_shift = 4 + (((tac - 1) & 0x03) << 1);

    // CGB double speed => faster increments
    clock_shift -= m_clock.is_double_speed();

    // this number has only one bit set
    AGE_ASSERT(clock_shift <= 10);
    AGE_ASSERT(clock_shift >= 3); // CGB double speed: up to 524288 Hz
    return clock_shift;
}



void age::gb_timer::set_clk_timer_zero(int new_clk_timer_zero)
{
    AGE_ASSERT(new_clk_timer_zero != gb_no_clock_cycle);
    m_clk_timer_zero = new_clk_timer_zero;

    int clks_per_inc = 1 << m_clk_shift;
    int clk_overflow = m_clk_timer_zero + 0x100 * clks_per_inc;
    AGE_ASSERT(clk_overflow >= m_clock.get_clock_cycle());

    int clks_until_overflow = clk_overflow - m_clock.get_clock_cycle();
    m_events.schedule_event(gb_event::timer_overflow, clks_until_overflow);
}



void age::gb_timer::set_tima(int for_tima)
{
    // adjust timer, if it is active,
    if (m_clk_timer_zero != gb_no_clock_cycle)
    {
        // smaller TIMA => more clocks until overflow
        // (and vice versa)
        int diff_tima = for_tima - m_tima;
        int clk_adjust = -diff_tima << m_clk_shift;
        set_clk_timer_zero(m_clk_timer_zero + clk_adjust);

        CLOG("    * +/- TIMA: " << diff_tima);
        CLOG("    * +/- clock cycles until overflow: " << clk_adjust);
    }

    m_tima = for_tima & 0xFF;
}
