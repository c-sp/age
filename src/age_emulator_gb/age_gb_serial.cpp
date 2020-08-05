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

#include "age_gb_serial.hpp"

namespace
{

// DMG: 512 clock cycles for transferring one bit (8192 Bits/s)
constexpr int sio_clock_shift = 9;

constexpr uint8_t sc_start_transfer = 0x80;
constexpr uint8_t sc_shift_clock_switch = 0x02;
constexpr uint8_t sc_terminal_selection = 0x01;

}



age::gb_serial::gb_serial(const gb_device &device,
                          const gb_clock &clock,
                          const gb_div &div,
                          gb_interrupt_trigger &interrupts,
                          gb_events &events)
    : m_device(device),
      m_clock(clock),
      m_div(div),
      m_interrupts(interrupts),
      m_events(events)
{
}



//---------------------------------------------------------
//
//   i/o ports
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
    AGE_GB_CLOG_SERIAL("read SB = " << AGE_LOG_HEX8(m_sb));
    return m_sb;
}

age::uint8_t age::gb_serial::read_sc() const
{
    uint8_t result = m_sc & 0x03;

    // unused bits are always high
    result |= m_device.is_cgb() ? 0x7C : 0x7E;

    // serial transfer currently in progress?
    result |= (m_sio_state == gb_sio_state::no_transfer) ? 0 : sc_start_transfer;

    AGE_GB_CLOG_SERIAL("read SC = " << AGE_LOG_HEX8(result));
    return result;
}



void age::gb_serial::write_sb(uint8_t value)
{
    AGE_GB_CLOG_SERIAL("write SB = " << AGE_LOG_HEX8(value)
                       << ", old SB was: " << AGE_LOG_HEX8(m_sb)
                       << ((m_sio_state == gb_sio_state::no_transfer)
                           ? ""
                           : ", write ignored: transfer in progress!"));

    // serial transfer in progress -> writing prohibited
    m_sb = (m_sio_state == gb_sio_state::no_transfer) ? value : m_sb;
}

void age::gb_serial::write_sc(uint8_t value)
{
    AGE_GB_CLOG_SERIAL("write SC = " << AGE_LOG_HEX8(value));
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

    bool fast_sio = m_device.is_cgb() && ((value_sc & sc_shift_clock_switch) != 0);
    int clock_shift = fast_sio ? (sio_clock_shift - 5) : sio_clock_shift;

    // adjust to CGB double speed
    clock_shift -= m_clock.is_double_speed();
    AGE_ASSERT(clock_shift >= 3);
    AGE_ASSERT(clock_shift <= 9);

    // number of clock cycles per serial transfer step
    int clks_per_step = 1 << (clock_shift - 1);

    // div-aligned clock
    auto current_clk = m_clock.get_clock_cycle();
    int clk_div_aligned = current_clk + m_div.get_div_offset();

    // number of clock cycles until first serial transfer step
    int clks_into_step = clk_div_aligned & (clks_per_step - 1);
    int clks_first_step = clks_per_step - clks_into_step;
    int clks_until_finished = clks_first_step + (15 << (clock_shift - 1));

    AGE_ASSERT(clks_first_step > 0);
    AGE_ASSERT(clks_first_step <= clks_per_step);
    AGE_ASSERT(clks_until_finished > 0);
    AGE_ASSERT(clks_until_finished <= 8 << clock_shift);

    AGE_GB_CLOG_SERIAL("starting serial transfer:");
    AGE_GB_CLOG_SERIAL("    * " << clks_per_step << " clock cycles per transferred bit");
    AGE_GB_CLOG_SERIAL("    * " << clks_first_step << " clock cycles until first step");
    AGE_GB_CLOG_SERIAL("    * finishes in " << clks_until_finished << " clock cycles ("
                       << (current_clk + clks_until_finished) << ")");

    m_sio_state = gb_sio_state::transfer_internal_clock;
    m_sio_clock_shift = clock_shift;
    m_sio_clk_started = current_clk + clks_until_finished - (8 << clock_shift);
    m_sio_initial_sb = m_sb;

    AGE_ASSERT(m_sio_clk_started <= current_clk);
    AGE_ASSERT(m_sio_clk_started != gb_no_clock_cycle);

    m_events.schedule_event(gb_event::serial_transfer_finished, clks_until_finished);
}



void age::gb_serial::update_state()
{
    // no ongoing serial transfer
    if (m_sio_state != gb_sio_state::transfer_internal_clock)
    {
        return;
    }
    AGE_ASSERT(m_sio_clk_started != gb_no_clock_cycle);
    AGE_ASSERT(m_sio_clk_started < m_clock.get_clock_cycle());

    // calculate the number of shifts since the transfer was started
    int clks_elapsed = m_clock.get_clock_cycle() - m_sio_clk_started;
    int shifts = clks_elapsed >> m_sio_clock_shift;

    // Since there is no serial transfer counterpart
    // we always receive 0xFF.
    int tmp = m_sio_initial_sb * 0x100 + 0xFF;
    tmp >>= 8 - std::min(shifts, 8);
    m_sb = tmp & 0xFF;

    // transfer finished?
    if (shifts >= 8)
    {
        AGE_GB_CLOG_SERIAL("serial transfer finished");
        stop_transfer(gb_sio_state::no_transfer);
        AGE_ASSERT(m_sb == 0xFF);

        int clk_irq = m_sio_clk_started + (8 << m_sio_clock_shift);
        m_interrupts.trigger_interrupt(gb_interrupt::serial, clk_irq);
    }
}



void age::gb_serial::stop_transfer(gb_sio_state new_state)
{
    m_sio_state = new_state;
    m_sio_clock_shift = 0;
    m_sio_clk_started = gb_no_clock_cycle;
    m_sio_initial_sb = 0;
    m_events.remove_event(gb_event::serial_transfer_finished);
}



//---------------------------------------------------------
//
//   clock related
//
//---------------------------------------------------------

void age::gb_serial::set_back_clock(int clock_cycle_offset)
{
    AGE_GB_SET_BACK_CLOCK(m_sio_clk_started, clock_cycle_offset);
}



void age::gb_serial::on_div_reset(int old_div_offset)
{
    update_state();

    // no transfer => nothing to do here
    if (m_sio_state != gb_sio_state::transfer_internal_clock)
    {
        return;
    }
    // ongoing transfer => calculate new timing

    // number of clock cycles per serial transfer step
    int clks_per_step = 1 << (m_sio_clock_shift - 1);
    AGE_ASSERT(clks_per_step > 0);

    // identify the "trigger bit":
    // this clock bit goes low 16 times during serial transfer
    int trigger_bit = 1 << (m_sio_clock_shift - 2);

    // calculate old and new "serial io clock" as we have to compare them
    // to check for the "trigger bit" going low due to DIV reset
    int current_clk = m_clock.get_clock_cycle();
    int old_clock = current_clk + old_div_offset;
    int new_clock = current_clk + m_div.get_div_offset();

    int old_next_step = clks_per_step - (old_clock & (clks_per_step - 1));
    int new_next_step = clks_per_step - (new_clock & (clks_per_step - 1));

    int old_trigger_bit = old_clock & trigger_bit;
    int new_trigger_bit = new_clock & trigger_bit;

    int clk_adjust = (old_trigger_bit && !new_trigger_bit)
            // trigger bit goes low
            //      => immediate serial transfer step
            //      => serial transfer time shortened
            ? -old_next_step
            // trigger bit not going low
            //      => serial transfer takes longer
            : new_next_step - old_next_step;

    int clk_finished = m_sio_clk_started + (8 << m_sio_clock_shift);
    AGE_ASSERT(clk_finished > current_clk);
    clk_finished += clk_adjust;
    AGE_ASSERT(clk_finished >= current_clk);

    AGE_GB_CLOG_SERIAL("serial transfer at DIV reset:");
    AGE_GB_CLOG_SERIAL("    * old lower clock bits: " << AGE_LOG_HEX16(old_clock & 0xFFFF));
    AGE_GB_CLOG_SERIAL("    * new lower clock bits: " << AGE_LOG_HEX16(new_clock & 0xFFFF));
    AGE_GB_CLOG_SERIAL("    * next step (old) in " << old_next_step << " clock cycles");
    AGE_GB_CLOG_SERIAL("    * next step (new) in " << new_next_step << " clock cycles");
    AGE_GB_CLOG_SERIAL("    * +/- remaining clock cycles: " << clk_adjust);
    AGE_GB_CLOG_SERIAL("    * finish on clock cycle " << clk_finished);

    m_sio_clk_started += clk_adjust;
    m_events.schedule_event(gb_event::serial_transfer_finished, clk_finished - current_clk);
}
