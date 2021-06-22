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

#include "age_gb_serial.hpp"

namespace
{
    // DMG: 512 clock cycles for transferring one bit (8192 Bits/s)
    constexpr int sio_clock_shift = 9;

    constexpr uint8_t sc_start_transfer     = 0x80;
    constexpr uint8_t sc_shift_clock_switch = 0x02;
    constexpr uint8_t sc_terminal_selection = 0x01;

} // namespace



age::gb_serial::gb_serial(const gb_device&      device,
                          const gb_clock&       clock,
                          gb_interrupt_trigger& interrupts,
                          gb_events&            events)
    : m_device(device),
      m_clock(clock),
      m_interrupts(interrupts),
      m_events(events)
{
}



//---------------------------------------------------------
//
//   serial registers
//
//---------------------------------------------------------

age::uint8_t age::gb_serial::read_sb()
{
    // The bits received so far are visible during serial transfer.
    //
    // Gambatte tests:
    //      serial/start_wait_read_sb_1_dmg08_cgb04c_out7F
    //      serial/start_wait_read_sb_2_dmg08_cgb04c_outFF
    update_state();

    log() << "read SB == " << log_hex8(m_sb);
    return m_sb;
}

age::uint8_t age::gb_serial::read_sc() const
{
    // unused bits are always high
    uint8_t unused = m_device.is_cgb() ? 0x7C : 0x7E;

    // serial transfer currently in progress?
    uint8_t transfer_flag = (m_sio_state == gb_sio_state::no_transfer) ? 0 : sc_start_transfer;

    uint8_t result = m_sc & 0x03;
    result |= unused;
    result |= transfer_flag;

    log() << "read SC == " << log_hex8(result);
    return result;
}



void age::gb_serial::write_sb(uint8_t value)
{
    log() << "write SB = " << log_hex8(value)
          << ", current SB: " << log_hex8(m_sb)
          << ((m_sio_state == gb_sio_state::no_transfer) ? "" : "\n    * write ignored: transfer in progress");

    // serial transfer in progress -> writing prohibited
    m_sb = (m_sio_state == gb_sio_state::no_transfer) ? value : m_sb;
}

void age::gb_serial::write_sc(uint8_t value)
{
    log() << "write SC = " << log_hex8(value);
    m_sc = value;

    // start serial transfer
    if ((value & sc_start_transfer) != 0)
    {
        // start serial transfer with internal clock
        if ((value & sc_terminal_selection) != 0)
        {
            start_transfer(value);
        }

        // Using an external clock stops any serial transfer as we have no
        // transfer counterpart available.
        //
        // Gambatte tests:
        //      serial/start_wait_sc80_read_if_1_dmg08_cgb04c_outE0
        //      serial/start_wait_sc80_read_if_2_dmg08_cgb04c_outE8
        else
        {
            stop_transfer(gb_sio_state::transfer_external_clock);
        }
    }

    // Clearing SC bit 7 stops an ongoing transfer.
    //
    // Gambatte tests:
    //      serial/start_wait_stop_read_if_1_dmg08_cgb04c_outE0
    //      serial/start_wait_stop_read_if_2_dmg08_cgb04c_outE
    else
    {
        stop_transfer(gb_sio_state::no_transfer);
    }
}



//---------------------------------------------------------
//
//   serial transfer
//
//---------------------------------------------------------

void age::gb_serial::start_transfer(uint8_t value_sc)
{
    // A serial transfer is completed after a specific clock bit
    // (DMG: clock bit 7) has been going low for 16 times.
    // A serial transfer thus consists of 16 steps triggered by
    // that bit.
    //
    // Gambatte tests:
    //      serial/nopx1_start_wait_read_if_1_dmg08_cgb04c_outE0
    //      serial/nopx1_start_wait_read_if_2_dmg08_cgb04c_outE8
    //      serial/nopx2_start_wait_read_if_1_dmg08_cgb04c_outE0
    //      serial/nopx2_start_wait_read_if_2_dmg08_cgb04c_outE8

    bool fast_sio    = m_device.is_cgb() && ((value_sc & sc_shift_clock_switch) != 0);
    int  clock_shift = fast_sio ? (sio_clock_shift - 5) : sio_clock_shift;

    // adjust to CGB double speed
    clock_shift -= m_clock.is_double_speed();
    AGE_ASSERT(clock_shift >= 3)
    AGE_ASSERT(clock_shift <= 9)

    // number of clock cycles per serial transfer step
    int clks_per_step = 1 << (clock_shift - 1);

    // div-aligned clock
    auto current_clk     = m_clock.get_clock_cycle();
    int  clk_div_aligned = current_clk + m_clock.get_div_offset();

    // number of clock cycles until first serial transfer step
    int clks_into_step      = clk_div_aligned & (clks_per_step - 1);
    int clks_first_step     = clks_per_step - clks_into_step;
    int clks_until_finished = clks_first_step + (15 << (clock_shift - 1));

    AGE_ASSERT(clks_first_step > 0)
    AGE_ASSERT(clks_first_step <= clks_per_step)
    AGE_ASSERT(clks_until_finished > 0)
    AGE_ASSERT(clks_until_finished <= 8 << clock_shift)

    log() << "starting serial transfer"
          << "\n    * " << clks_per_step << " clock cycles per transferred bit"
          << "\n    * " << clks_first_step << " clock cycles until first transfer step"
          << "\n    * finished in " << log_in_clks(clks_until_finished, current_clk);

    m_sio_state       = gb_sio_state::transfer_internal_clock;
    m_sio_clock_shift = clock_shift;
    m_sio_clk_started = current_clk + clks_until_finished - (8 << clock_shift);
    m_sio_initial_sb  = m_sb;

    AGE_ASSERT(m_sio_clk_started <= current_clk)
    AGE_ASSERT(m_sio_clk_started != gb_no_clock_cycle)

    m_events.schedule_event(gb_event::serial_transfer_finished, clks_until_finished);
}



void age::gb_serial::update_state()
{
    // no ongoing serial transfer
    if (m_sio_state != gb_sio_state::transfer_internal_clock)
    {
        return;
    }
    AGE_ASSERT(m_sio_clk_started != gb_no_clock_cycle)
    AGE_ASSERT(m_sio_clk_started < m_clock.get_clock_cycle())

    // calculate the number of shifts since the transfer was started
    int clks_elapsed = m_clock.get_clock_cycle() - m_sio_clk_started;
    int shifts       = clks_elapsed >> m_sio_clock_shift;

    // Since there is no serial transfer counterpart
    // we always receive 0xFF.
    int tmp = m_sio_initial_sb * 0x100 + 0xFF;
    tmp >>= 8 - std::min(shifts, 8);
    m_sb = tmp & 0xFF;

    // transfer finished?
    if (shifts >= 8)
    {
        log() << "serial transfer finished";
        stop_transfer(gb_sio_state::no_transfer);
        AGE_ASSERT(m_sb == 0xFF)

        int clk_irq = m_sio_clk_started + (8 << m_sio_clock_shift);
        m_interrupts.trigger_interrupt(gb_interrupt::serial, clk_irq);
    }
}



void age::gb_serial::stop_transfer(gb_sio_state new_state)
{
    m_sio_state       = new_state;
    m_sio_clock_shift = 0;
    m_sio_clk_started = gb_no_clock_cycle;
    m_sio_initial_sb  = 0;
    m_events.remove_event(gb_event::serial_transfer_finished);
}



//---------------------------------------------------------
//
//   clock related
//
//---------------------------------------------------------

void age::gb_serial::set_back_clock(int clock_cycle_offset)
{
    gb_set_back_clock_cycle(m_sio_clk_started, clock_cycle_offset);
}



void age::gb_serial::after_div_reset()
{
    update_state(); // may finish active transfer

    // no transfer => nothing to do here
    if (m_sio_state != gb_sio_state::transfer_internal_clock)
    {
        log() << "serial transfer off at DIV reset";
        return;
    }
    // ongoing transfer => calculate new timing

    // number of clock cycles per serial transfer step
    int clks_per_step = 1 << (m_sio_clock_shift - 1);
    AGE_ASSERT(clks_per_step > 0)

    // calculate potential immediate serial transfer step by div reset
    auto reset_details = m_clock.get_div_reset_details(clks_per_step);

    int clk_current  = m_clock.get_clock_cycle();
    int clk_finished = m_sio_clk_started + (8 << m_sio_clock_shift);
    AGE_ASSERT(clk_finished > clk_current)
    clk_finished += reset_details.m_clk_adjust;
    AGE_ASSERT(clk_finished >= clk_current)

    log() << "serial transfer at DIV reset:"
          << "\n    * next step (old) in " << log_in_clks(reset_details.m_old_next_increment, clk_current)
          << "\n    * next step (new) in " << log_in_clks(reset_details.m_new_next_increment, clk_current)
          << "\n    * +/- remaining clock cycles: " << reset_details.m_clk_adjust
          << "\n    * finished on clock cycle " << clk_finished;

    m_sio_clk_started += reset_details.m_clk_adjust;
    m_events.schedule_event(gb_event::serial_transfer_finished, clk_finished - clk_current);
}
