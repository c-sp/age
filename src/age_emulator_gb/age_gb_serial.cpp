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

// 256 cycles for "bit in"
// 256 cycles for "bit out"
constexpr uint gb_sio_shift_clock_bit = 1 << 8;
// 512 cycles for fully transferring one bit (8192 Bits/s)
constexpr uint gb_sio_cycles_per_bit = gb_sio_shift_clock_bit << 1;

constexpr uint8 gb_sc_start_transfer = 0x80;
constexpr uint8 gb_sc_terminal_selection = 0x01;

}



//---------------------------------------------------------
//
//   i/o port reads & writes
//
//---------------------------------------------------------

age::uint8 age::gb_serial::read_sb() const
{
    // SC7 set -> reading and writing prohibited
    return ((m_sc & gb_sc_start_transfer) == 0) ? m_sb : 0xFF;
}

age::uint8 age::gb_serial::read_sc() const
{
    return m_sc;
}



void age::gb_serial::write_sb(uint8 value)
{
    LOG("0x" << std::hex << (uint)value << " (cur 0x" << (uint)m_sb << std::dec
        << ( ((m_sc & gb_sc_start_transfer) == 0) ? ")" : "), transfer in progress!" ));

    // SC7 set -> reading and writing prohibited
    if ((m_sc & gb_sc_start_transfer) == 0)
    {
        m_sb = value;
    }
}

void age::gb_serial::write_sc(uint8 value)
{
    LOG("0x" << std::hex << (uint)value << std::dec);

    // clearing the high bit manually is not possible
    if ((m_sc & gb_sc_start_transfer) > 0)
    {
        value |= gb_sc_start_transfer;
    }

    // unused bits are always high
    value |= m_core.is_cgb() ? 0x7C : 0x7E;

    // save value to SC
    m_sc = value;

    // start serial transfer
    if ((m_sc & gb_sc_start_transfer) > 0)
    {
        // start transfer only when using the internal clock
        if ((m_sc & gb_sc_terminal_selection) > 0)
        {
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
            uint cycles_into_sio_clock = m_core.get_oscillation_cycle() % gb_sio_cycles_per_bit;

            //
            // verified by gambatte tests
            //
            // Starting the serial transfer while bit 8 of the cycle counter
            // is set will extend the transfer duration by half the cycles
            // required to transfer a single bit.
            //
            //      serial/nopx1_start_wait_read_if_1_dmg08_cgb04c_outE0
            //      serial/nopx1_start_wait_read_if_2_dmg08_cgb04c_outE8
            //      serial/nopx2_start_wait_read_if_1_dmg08_cgb04c_outE0
            //      serial/nopx2_start_wait_read_if_2_dmg08_cgb04c_outE8
            //
            uint extended_cycles = ((m_core.get_oscillation_cycle() & gb_sio_shift_clock_bit) > 0) ? gb_sio_cycles_per_bit / 2 : 0;

            LOG("starting serial transfer, " << cycles_into_sio_clock << " cycles into sio clock, " << extended_cycles << " cycles extended");
            uint transfer_duration_cycles = 8 * gb_sio_cycles_per_bit - cycles_into_sio_clock + extended_cycles;

            m_core.insert_event(transfer_duration_cycles, gb_event::serial_transfer_finished);
        }

        //
        // verified by gambatte tests
        //
        // Since we have no external clock any ongoing transfer is essentially
        // aborted when switching to external clock.
        //
        //      serial/start_wait_sc80_read_if_1_dmg08_cgb04c_outE0
        //      serial/start_wait_sc80_read_if_2_dmg08_cgb04c_outE8
        //
        else
        {
            m_core.remove_event(gb_event::serial_transfer_finished);
        }
    }
}



//---------------------------------------------------------
//
//   Serial i/o emulation
//
//---------------------------------------------------------

void age::gb_serial::finish_transfer()
{
    LOG("serial transfer finished");
    m_sc &= ~gb_sc_start_transfer;

    LOG("request serial transfer interrupt");
    m_core.request_interrupt(gb_interrupt::serial); // breaks gator pinball (no joypad reaction any more)
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
