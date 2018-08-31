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

#include "age_gb_serial.hpp"

#if 0
#define LOG(x) AGE_GB_CYCLE_LOG(x)
#else
#define LOG(x)
#endif

namespace age {

// DMG: 256 cycles for "bit in"
// DMG: 256 cycles for "bit out"
// DMG: 512 cycles for fully transferring one bit (8192 Bits/s)
constexpr int gb_sio_shift_clock_bit = 1 << 8;

constexpr uint8_t gb_sc_start_transfer = 0x80;
constexpr uint8_t gb_sc_shift_clock_switch = 0x02;
constexpr uint8_t gb_sc_terminal_selection = 0x01;

}



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

age::uint8_t age::gb_serial::read_sb()
{
    //
    // verified by gambatte tests
    //
    // It seems the bits received so far are visible during serial transfer.
    //
    //      serial/start_wait_read_sb_1_dmg08_cgb04c_out7F
    //      serial/start_wait_read_sb_2_dmg08_cgb04c_outFF
    //
    if (m_sio_state == gb_sio_state::transfer_internal_clock)
    {
        transfer_update_sb();
    }
    LOG("0x" << std::hex << (int)m_sb << std::dec);
    return m_sb;
}

age::uint8_t age::gb_serial::read_sc() const
{
    uint8_t result = m_sc & 0x03;

    // unused bits are always high
    result |= m_core.is_cgb() ? 0x7C : 0x7E;

    // serial transfer currently in progress?
    result |= (m_sio_state == gb_sio_state::no_transfer) ? 0 : gb_sc_start_transfer;

    LOG("0x" << std::hex << (int)result << std::dec);
    return result;
}



void age::gb_serial::write_sb(uint8_t value)
{
    LOG("0x" << std::hex << (int)value << " (cur 0x" << (int)m_sb << std::dec
        << ((m_sio_state == gb_sio_state::no_transfer) ? ")" : "), ignored: transfer in progress!"));

    // serial transfer in progress -> writing prohibited
    m_sb = (m_sio_state == gb_sio_state::no_transfer) ? value : m_sb;
}

void age::gb_serial::write_sc(uint8_t value)
{
    LOG("0x" << std::hex << (int)value << std::dec);
    m_sc = value;

    // start serial transfer
    if ((value & gb_sc_start_transfer) != 0)
    {
        if ((value & gb_sc_terminal_selection) != 0)
        {
            m_sio_state = gb_sio_state::transfer_internal_clock;
            auto cycles_until_finished = transfer_init(value);
            m_core.insert_event(cycles_until_finished, gb_event::serial_transfer_finished);
        }

        //
        // verified by gambatte tests
        //
        // Using an external clock essentially stops the serial transfer
        // since we have no transfer counterpart available.
        //
        //      serial/start_wait_sc80_read_if_1_dmg08_cgb04c_outE0
        //      serial/start_wait_sc80_read_if_2_dmg08_cgb04c_outE8
        //
        else
        {
            m_sio_state = gb_sio_state::transfer_external_clock;
            m_core.remove_event(gb_event::serial_transfer_finished);
        }
    }

    //
    // verified by gambatte tests
    //
    // Clearing SC bit 7 seems to stop an ongoing transfer.
    //
    //      serial/start_wait_stop_read_if_1_dmg08_cgb04c_outE0
    //      serial/start_wait_stop_read_if_2_dmg08_cgb04c_outE
    //
    else
    {
        m_sio_state = gb_sio_state::no_transfer;
        m_core.remove_event(gb_event::serial_transfer_finished);
    }
}



void age::gb_serial::finish_transfer()
{
    LOG("serial transfer finished");
    m_sio_state = gb_sio_state::no_transfer;
    m_sio_last_receive_cycle = gb_no_cycle;

    // since there is no counterpart for serial transfer we always receive 0xFF
    m_sb = 0xFF;

    LOG("request serial transfer interrupt");
    m_core.request_interrupt(gb_interrupt::serial);
}

void age::gb_serial::set_back_cycles(int offset)
{
    AGE_GB_SET_BACK_CYCLES(m_sio_last_receive_cycle, offset);
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

int age::gb_serial::transfer_init(uint8_t value)
{
    bool high_frequency = m_core.is_cgb() && ((value & gb_sc_shift_clock_switch) != 0);

    int shift_clock_bit = high_frequency ? gb_sio_shift_clock_bit >> 5 : gb_sio_shift_clock_bit;
    shift_clock_bit = m_core.is_double_speed() ? shift_clock_bit >> 1 : shift_clock_bit;

    int cycles_per_bit = shift_clock_bit << 1;

    auto current_cycle = m_core.get_oscillation_cycle();

    //
    // verified by mooneye-gb tests
    //
    // The serial transfer clock is divided from the main clock,
    // thus the transfer does not start immediately.
    // Side note: when boot_sclk_align-dmgABCmgb starts the transfer
    //            bit 8 of the cycle counter is not set.
    //
    //      acceptance/serial/boot_sclk_align-dmgABCmgb
    //
    int cycles_into_sio_clock = current_cycle % cycles_per_bit;

    //
    // verified by gambatte tests
    //
    // Starting the serial transfer while bit 8 of the cycle counter (DMG)
    // is set will extend the transfer duration by half the cycles
    // required to transfer a single bit.
    //
    // My guess: Sending and receiving a single bit is done on different edges
    //           of the sio clock (DMG: cycle counter bit 8 going low/high).
    //           We must transmit bit 7 before shifting a new bit 0 into the register,
    //           otherwise bit 7 is lost.
    //           If receiving a bit is triggered by cycle counter bit 8 going low,
    //           starting a transfer with bit 8 being high would trigger a receive
    //           before the first transmission and thus lose bit 7.
    //           In this case it makes sense to skip the first receive which results
    //           in the transfer taking longer.
    //
    //      serial/nopx1_start_wait_read_if_1_dmg08_cgb04c_outE0
    //      serial/nopx1_start_wait_read_if_2_dmg08_cgb04c_outE8
    //      serial/nopx2_start_wait_read_if_1_dmg08_cgb04c_outE0
    //      serial/nopx2_start_wait_read_if_2_dmg08_cgb04c_outE8
    //
    int extended_cycles = ((current_cycle & shift_clock_bit) != 0) ? cycles_per_bit / 2 : 0;

    LOG("starting serial transfer, " << cycles_into_sio_clock << " cycles into sio clock, " << extended_cycles << " cycles extended");
    int cycles_until_finished = 8 * cycles_per_bit - cycles_into_sio_clock + extended_cycles;

    m_sio_cycles_per_bit = cycles_per_bit;
    m_sio_last_receive_cycle = current_cycle + cycles_until_finished - 8 * cycles_per_bit;
    AGE_ASSERT(m_sio_last_receive_cycle <= current_cycle);

    return cycles_until_finished;
}



void age::gb_serial::transfer_update_sb()
{
    AGE_ASSERT(m_sio_state == gb_sio_state::transfer_internal_clock);
    AGE_ASSERT(m_sio_cycles_per_bit > 0);
    AGE_ASSERT(m_sio_last_receive_cycle <= m_core.get_oscillation_cycle());

    int cycles_elapsed = m_core.get_oscillation_cycle() - m_sio_last_receive_cycle;
    int shifts = cycles_elapsed / m_sio_cycles_per_bit;
    AGE_ASSERT(shifts < 8);

    if (shifts > 0)
    {
        int tmp = m_sb * 0x100 + 0xFF;
        tmp >>= 8 - shifts;
        LOG("transfer in progress: updating SB from 0x" << std::hex << (int)m_sb << " to 0x" << tmp << std::dec << " (" << shifts << " shifts)");
        m_sb = tmp & 0xFF;

        m_sio_last_receive_cycle += shifts * m_sio_cycles_per_bit;
    }
}



//---------------------------------------------------------
//
//   object creation
//
//---------------------------------------------------------

age::gb_serial::gb_serial(gb_core &core)
    : m_core(core)
{
    write_sc(0);
}
