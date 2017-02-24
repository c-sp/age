//
// Copyright (c) 2010-2017 Christoph Sprenger
//
// This file is part of AGE ("Another Gameboy Emulator").
// <https://gitlab.com/csprenger/AGE>
//
// AGE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// AGE is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with AGE.  If not, see <http://www.gnu.org/licenses/>.
//

#include "age_gb_serial.hpp"



//---------------------------------------------------------
//
//   i/o port reads & writes.
//
//---------------------------------------------------------

age::uint8 age::gb_serial::read_sb() const
{
    uint8 result;

    // SC7 set -> reading and writing prohibited (gb_document)
    if ((m_sc & gb_sc_start_transfer) == 0)
    {
        result = m_sb;
    }
    else
    {
        result = 0xFF;
    }

    return result;
}

age::uint8 age::gb_serial::read_sc() const
{
    return m_sc;
}

void age::gb_serial::write_sb(uint8 value)
{
    // SC7 set -> reading and writing prohibited (gb_document)
    if ((m_sc & gb_sc_start_transfer) == 0)
    {
        m_sb = value;
    }
}

void age::gb_serial::write_sc(uint8 value)
{
    // clearing the high bit manually is not possible
    // (I'm guessing that based on the information found in gb_document and gb-cpu-man)
    if ((m_sc & gb_sc_start_transfer) > 0)
    {
        value |= gb_sc_start_transfer;
    }

    // unused bits are always high
    value |= m_is_cgb ? 0x7C : 0x7E;

    // save value to SC
    m_sc = value;

    // trigger serial i/o transfer, if possible
    if (((m_sc & gb_sc_start_transfer) > 0) && (m_state == gb_serial_state::idle))
    {
        if ((m_sc && gb_sc_terminal_selection) > 0)
        {
            m_state = gb_serial_state::during_transfer_internal;
            m_cycles_left = gb_serial_transfer_cycles;

            if (m_is_cgb)
            {
                uint shift = 0;
                if ((m_sc & gb_sc_shift_clock) > 0)
                {
                    shift = 5; // /32
                }
                m_cycles_left >>= shift;
            }
        }
        else
        {
            m_state = gb_serial_state::waiting_for_external;
        }
    }
}



//---------------------------------------------------------
//
//   Serial i/o emulation.
//
//---------------------------------------------------------

void age::gb_serial::emulate(uint cycles_elapsed)
{
    // perform serial transfer with internal clock
    if (m_state == gb_serial_state::during_transfer_internal)
    {
        if (m_cycles_left <= cycles_elapsed)
        {
            m_sb = 0xFF; // reiceived nothing
            finish_transfer();
        }
        else
        {
            m_cycles_left -= cycles_elapsed;
        }
    }

    // perform serial transfer with external clock
    else if (m_state == gb_serial_state::during_transfer_external)
    {
        if (m_cycles_left <= cycles_elapsed)
        {
            m_sb = 0xFF; // reiceived nothing
            finish_transfer();
        }
        else
        {
            m_cycles_left -= cycles_elapsed;
        }
    }

    // do nothing, if we're idle or waiting for an external clock
}



void age::gb_serial::finish_transfer()
{
    m_sc &= ~gb_sc_start_transfer;
    //m_core.request_interrupt(gb_interrupt::serial); // breaks gator pinball (no joypad reaction any more)
    m_state = gb_serial_state::idle;
}



void age::gb_serial::switch_double_speed_mode()
{
}



//---------------------------------------------------------
//
//   Timer object creation.
//
//---------------------------------------------------------

age::gb_serial::gb_serial(gb_core &core)
    : m_core(core),
      m_is_cgb(m_core.is_cgb())
{
    write_sc(0);
}
