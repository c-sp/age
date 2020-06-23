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

#define CLOG(log) AGE_GB_CLOG(AGE_GB_CLOG_SERIAL)(log)

namespace
{

// DMG: 256 clock cycles for "bit in"
// DMG: 256 clock cycles for "bit out"
// DMG: 512 clock cycles for fully transferring one bit (8192 Bits/s)
constexpr int sio_clock_bit = 1 << 8;

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
    CLOG("read SB " << AGE_LOG_HEX8(m_sb));
    return m_sb;
}

age::uint8_t age::gb_serial::read_sc() const
{
    uint8_t result = m_sc & 0x03;

    // unused bits are always high
    result |= m_device.is_cgb() ? 0x7C : 0x7E;

    // serial transfer currently in progress?
    result |= (m_sio_state == gb_sio_state::no_transfer) ? 0 : sc_start_transfer;

    CLOG("read SC " << AGE_LOG_HEX8(result));
    return result;
}



void age::gb_serial::write_sb(uint8_t value)
{
    CLOG("write SB " << AGE_LOG_HEX8(value)
         << ", old SB was: " << AGE_LOG_HEX8(m_sb)
         << ((m_sio_state == gb_sio_state::no_transfer)
             ? ""
             : ", write ignored: transfer in progress!"));

    // serial transfer in progress -> writing prohibited
    m_sb = (m_sio_state == gb_sio_state::no_transfer) ? value : m_sb;
}

void age::gb_serial::write_sc(uint8_t value)
{
    CLOG("write SC " << AGE_LOG_HEX8(value));
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
    // (DMG: clock bit 8) has changed for 16 times.
    // The bit's initial state is irrelevant.
    //
    // Gambatte tests:
    //      serial/nopx1_start_wait_read_if_1_dmg08_cgb04c_outE0
    //      serial/nopx1_start_wait_read_if_2_dmg08_cgb04c_outE8
    //      serial/nopx2_start_wait_read_if_1_dmg08_cgb04c_outE0
    //      serial/nopx2_start_wait_read_if_2_dmg08_cgb04c_outE8

    bool high_frequency = m_device.is_cgb() && ((value_sc & sc_shift_clock_switch) != 0);

    // identify the clock bit
    int clock_bit = high_frequency
            ? (sio_clock_bit >> 5)
            : sio_clock_bit;

    // adjust that clock bit to the current CGB speed
    clock_bit = m_clock.is_double_speed()
            ? (clock_bit >> 1)
            : clock_bit;

    // the number of clock cycles per transferred bit
    int clks_per_bit = clock_bit << 1;

    // the number of clock cycles until the first bit change
    // (div-aligned)
    auto current_clk = m_clock.get_clock_cycle();
    int clk_div_aligned = current_clk + m_div.get_div_offset();
    int clks_first_switch = clock_bit - (clk_div_aligned % clock_bit);
    int clks_until_finished = clks_first_switch + 15 * clock_bit;

    AGE_ASSERT(clks_first_switch > 0);
    AGE_ASSERT(clks_first_switch <= clock_bit);
    AGE_ASSERT(clks_until_finished > 0);
    AGE_ASSERT(clks_until_finished <= 8 * clks_per_bit);

    CLOG("starting serial transfer:");
    CLOG("    * " << clks_per_bit << " clock cycles per transferred bit");
    CLOG("    * " << clks_first_switch << " clock cycles until first clock bit change");
    CLOG("    * div offset " << AGE_LOG_HEX16(m_div.get_div_offset()));
    CLOG("    * finishes on clock cycle " << (current_clk + clks_until_finished));

    m_sio_state = gb_sio_state::transfer_internal_clock;
    m_sio_clk_cycles_per_bit = clks_per_bit;
    m_sio_clk_started = current_clk + clks_until_finished - 8 * clks_per_bit;
    m_sio_initial_sb = m_sb;

    AGE_ASSERT(m_sio_clk_cycles_per_bit > 0);
    AGE_ASSERT((m_sio_clk_cycles_per_bit & (m_sio_clk_cycles_per_bit - 1)) == 0);
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
    int shifts = clks_elapsed / m_sio_clk_cycles_per_bit;

    // since there is no counterpart for serial transfer we always receive 0xFF
    int tmp = m_sio_initial_sb * 0x100 + 0xFF;
    tmp >>= 8 - std::min(shifts, 8);
    m_sb = tmp & 0xFF;

    // transfer finished?
    if (shifts >= 8)
    {
        CLOG("serial transfer finished");
        stop_transfer(gb_sio_state::no_transfer);
        AGE_ASSERT(m_sb == 0xFF);
        m_interrupts.trigger_interrupt(gb_interrupt::serial);
    }
}



void age::gb_serial::stop_transfer(gb_sio_state new_state)
{
    m_sio_state = new_state;
    m_sio_clk_cycles_per_bit = 0;
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

    // identify the clock bit changing 16 times during serial transfer
    int clock_bit = m_sio_clk_cycles_per_bit >> 1;
    AGE_ASSERT(clock_bit > 0);

    // identify the "trigger bit":
    // this clock bit goes low 16 times during serial transfer
    int trigger_bit = clock_bit >> 1;

    // calculate old and new "serial io clock" as we have to compare them
    // to check for the "trigger bit" going low due to DIV reset
    int current_clk = m_clock.get_clock_cycle();
    int old_clock = current_clk + old_div_offset;
    int new_clock = current_clk + m_div.get_div_offset();

    int old_next_change = clock_bit - (old_clock & (clock_bit - 1));
    int new_next_change = clock_bit - (new_clock & (clock_bit - 1));

    int old_trigger_bit = old_clock & trigger_bit;
    int new_trigger_bit = new_clock & trigger_bit;

    int clk_adjust = (old_trigger_bit && !new_trigger_bit)
            // trigger bit goes low
            //      => immediate serial transfer step
            //      => serial transfer time shortened
            ? -old_next_change
            // trigger bit not going low
            //      => serial transfer takes longer
            : new_next_change - old_next_change;

    int clk_finished = m_sio_clk_started + 8 * m_sio_clk_cycles_per_bit;
    AGE_ASSERT(clk_finished > current_clk);
    clk_finished += clk_adjust;
    AGE_ASSERT(clk_finished >= current_clk);

    CLOG("serial transfer during DIV reset:");
    CLOG("    * old clock bits " << AGE_LOG_HEX16(old_clock));
    CLOG("    * new clock bits " << AGE_LOG_HEX16(new_clock));
    CLOG("    * next bit change (old) in " << old_next_change << " clock cycles");
    CLOG("    * next bit change (new) in " << new_next_change << " clock cycles");
    CLOG("    * adjusting remaining clock cycles by " << clk_adjust);
    CLOG("    * finishes on clock cycle " << clk_finished);

    m_sio_clk_started += clk_adjust;
    m_events.schedule_event(gb_event::serial_transfer_finished, clk_finished - current_clk);
}
