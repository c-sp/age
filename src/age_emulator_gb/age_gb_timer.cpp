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

#include "age_gb_timer.hpp"

#include <cassert>

namespace
{
    constexpr uint8_t tac_start_timer = 0x04;

} // namespace



age::gb_timer::gb_timer(const gb_device&      device,
                        const gb_clock&       clock,
                        gb_interrupt_trigger& interrupts,
                        gb_events&            events)
    : m_device(device),
      m_clock(clock),
      m_interrupts(interrupts),
      m_events(events)
{
}

age::uint8_t age::gb_timer::get_clock_shift() const
{
    // 00   (4096 Hz): clock cycle >> 10 (1024 clock cycles)
    // 01 (262144 Hz): clock cycle >>  4   (16 clock cycles)
    // 10  (65536 Hz): clock cycle >>  6   (64 clock cycles)
    // 11  (16384 Hz): clock cycle >>  8  (256 clock cycles)
    uint8_t clock_shift = 4 + (((m_tac - 1) & 0x03) << 1);

    // CGB double speed => faster increments
    clock_shift -= m_clock.is_double_speed() ? 1 : 0;

    assert(clock_shift <= 10);
    assert(clock_shift >= 3); // CGB double speed: up to 524288 Hz

    return clock_shift;
}



//---------------------------------------------------------
//
//   read timer registers
//
//---------------------------------------------------------

age::uint8_t age::gb_timer::read_tma() const
{
    log() << "read TMA == " << log_hex8(m_tma);
    return m_tma;
}



age::uint8_t age::gb_timer::read_tac() const
{
    log() << "read TAC == " << log_hex8(m_tac);
    return m_tac;
}



age::uint8_t age::gb_timer::read_tima()
{
    if (m_clk_timer_zero != gb_no_clock_cycle)
    {
        // calculate current TIMA value
        update_timer_state();

        // If there was a timer overflow on this very clock cycle,
        // TIMA equals zero.
        //
        // Mooneye GB tests:
        //      acceptance/timer/tima_reload
        if (m_clk_last_overflow == m_clock.get_clock_cycle())
        {
            log() << "read TIMA 0 due to recent timer overflow";
            return 0;
        }
    }

    log() << "read TIMA == " << log_hex8(m_tima);
    return m_tima;
}



//---------------------------------------------------------
//
//   write timer registers
//
//---------------------------------------------------------

void age::gb_timer::write_tma(uint8_t value)
{
    auto msg = log();
    msg << "write TMA = " << log_hex8(value);

    if (m_clk_timer_zero != gb_no_clock_cycle)
    {
        update_timer_state(); // update with old TMA

        // The new TMA is used immediately if a timer overflow
        // occurred at most one machine cycle ago.
        //
        // Mooneye GB tests:
        //      acceptance/timer/tma_write_reloading
        if (m_clk_last_overflow != gb_no_clock_cycle)
        {
            assert(m_clk_last_overflow <= m_clock.get_clock_cycle());
            int clks = m_clock.get_clock_cycle() - m_clk_last_overflow;

            if (clks <= m_clock.get_machine_cycle_clocks())
            {
                msg << "\n    * copied to TIMA due to recent timer overflow";
                m_tima = value;
                start_timer();
            }
        }
    }

    m_tma = value;
}



void age::gb_timer::write_tac(uint8_t value)
{
    auto msg = log();
    msg << "write TAC = " << log_hex8(value);

    m_tac                = value | 0xF8;
    bool start_new_timer = (m_tac & tac_start_timer) != 0;

    // timer off => maybe start it?
    if (m_clk_timer_zero == gb_no_clock_cycle)
    {
        if (start_new_timer)
        {
            start_timer();
        }
        return;
    }

    // timer active and to be stopped?
    if (!start_new_timer)
    {
        update_timer_state(); // update TIMA
        stop_timer();
        return;
    }

    // timer stays active,
    // the frequency may change though

    // When the increment triggering bit goes low due to
    // a timer frequency change,
    // the timer will be immediately incremented.
    //
    // Gambatte tests:
    //      tima/tc00_tc01_late_tc00_of_2

    int div_clock = m_clock.get_clock_cycle() + m_clock.get_div_offset();

    int old_trigger_bit = 1 << (m_clock_shift - 1);
    int old_bit         = div_clock & old_trigger_bit;

    int new_trigger_bit = 1 << (get_clock_shift() - 1);
    int new_bit         = div_clock & new_trigger_bit;

    if (old_bit && !new_bit)
    {
        // simulate immediate increment
        msg << "\n    * immediate TIMA increment by frequency change";
        m_clk_timer_zero -= old_trigger_bit * 2;
    }

    // update current TIMA
    // (including possible immediate increment, see above)
    update_timer_state();

    // restart timer
    // (required actually only if update_timer_state() did not overflow)
    start_timer();
}



void age::gb_timer::write_tima(uint8_t value)
{
    auto msg = log();
    msg << "write TIMA = " << log_hex8(value);

    // timer off => just write the value
    if (m_clk_timer_zero == gb_no_clock_cycle)
    {
        m_tima = value;
        return;
    }

    // update active timer
    update_timer_state();

    // TIMA write is ignored if the last timer overflow occurred
    // one machine cycle ago.
    //
    // Mooneye GB tests:
    //      acceptance/timer/tima_write_reloading
    int clk_current = m_clock.get_clock_cycle();
    int clk_ignore  = m_clk_last_overflow + m_clock.get_machine_cycle_clocks();
    if (clk_current == clk_ignore)
    {
        msg << "\n    * ignored due to recent timer overflow";
        return;
    }

    // Writing TIMA at the exact clock cycle it overflows prevents
    // the interrupt from being triggered.
    //
    // Gambatte tests:
    //      tima/tc01_late_tima_irq_1_dmg08_cgb04c_outE0
    //      tima/tc01_late_tima_irq_2_dmg08_cgb04c_outE4
    if (m_clk_last_overflow == clk_current)
    {
        // allow scheduling the timer interrupt event for the
        // next timer overflow
        m_clk_last_overflow = gb_no_clock_cycle;
    }

    m_tima = value;
    start_timer();
}
