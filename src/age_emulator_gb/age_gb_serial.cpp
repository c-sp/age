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

constexpr uint gb_serial_transfer_cycles_per_bit = gb_machine_cycles_per_second / 8192; // bit transfer with 8192 Hz

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
    // SC7 set -> reading and writing prohibited
    if ((m_sc & gb_sc_start_transfer) == 0)
    {
        m_sb = value;
    }
}

void age::gb_serial::write_sc(uint8 value)
{
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
        if ((m_sc & gb_sc_terminal_selection) > 0)
        {
            //
            // verified by mooneye-gb tests
            //
            // The serial transfer clock is divided from the main clock,
            // thus the transfer does not start immediately.
            //
            //      acceptance/serial/boot_sclk_align-dmgABCmgb
            //
            uint cycles_mod = m_core.get_oscillation_cycle() % gb_serial_transfer_cycles_per_bit;
            LOG("starting serial transfer, " << cycles_mod << " cycles into serial transfer clock");

            uint transfer_duration_cycles = 8 * gb_serial_transfer_cycles_per_bit - cycles_mod;
            m_core.insert_event(transfer_duration_cycles, gb_event::serial_transfer_finished);
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
