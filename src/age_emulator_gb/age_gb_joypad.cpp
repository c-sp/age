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

#include <age_debug.hpp>

#include "age_gb_joypad.hpp"

#if 0
#define LOG(x) AGE_GB_CYCLE_LOG(x)
#else
#define LOG(x)
#endif



age::uint8 age::gb_joypad::read_p1() const
{
    return m_p1;
}

void age::gb_joypad::write_p1(uint8 byte)
{
    AGE_ASSERT(m_p14 <= 0x0F);
    AGE_ASSERT(m_p15 <= 0x0F);
    byte |= 0x0F; // set button-bits

    // p14 low?
    if ((byte & gb_p14) == 0)
    {
        byte &= 0xF0 | m_p14;
    }

    // p15 low?
    if ((byte & gb_p15) == 0)
    {
        byte &= 0xF0 | m_p15;
    }

    // interrupt: p10-p13 changed from high to low
    // (raise int for low-to-high too, since it happens on the Gameboy - apparently due to oscillation)
    uint raise_interrupt = (m_p1 ^ byte) & 0x0F;
    if (raise_interrupt > 0)
    {
        m_core.request_interrupt(gb_interrupt::joypad);
    }

    // save new value
    m_p1 = byte | 0xC0;
}



void age::gb_joypad::set_buttons_up(uint buttons)
{
    if (buttons != 0)
    {
        LOG(buttons);
        uint8 p14 = static_cast<uint8>(buttons & 0x0F);
        uint8 p15 = static_cast<uint8>((buttons >> 4) & 0x0F);
        m_p14 |= p14;
        m_p15 |= p15;
        write_p1(m_p1);
    }
}

void age::gb_joypad::set_buttons_down(uint buttons)
{
    if (buttons != 0)
    {
        LOG(buttons);
        uint8 p14 = static_cast<uint8>(buttons & 0x0F);
        uint8 p15 = static_cast<uint8>((buttons >> 4) & 0x0F);
        m_p14 &= ~p14;
        m_p15 &= ~p15;
        write_p1(m_p1);
    }
}



age::gb_joypad::gb_joypad(gb_core &core)
    : m_core(core)
{
}
