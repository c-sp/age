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

#include "age_gb_joypad.hpp"

#if 0
#define LOG(x) AGE_GB_CYCLE_LOG(x)
#else
#define LOG(x)
#endif

namespace age
{

constexpr uint8_t gb_p14 = 0x10;
constexpr uint8_t gb_p15 = 0x20;

}



age::uint8_t age::gb_joypad::read_p1() const
{
    return m_p1;
}

void age::gb_joypad::write_p1(uint8_t byte)
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
    int raise_interrupt = (m_p1 ^ byte) & 0x0F;
    if (raise_interrupt > 0)
    {
        m_core.request_interrupt(gb_interrupt::joypad);
    }

    // save new value
    m_p1 = byte | 0xC0;
}



void age::gb_joypad::set_buttons_up(int buttons)
{
    if (buttons != 0)
    {
        LOG(buttons);
        uint8_t p14 = buttons & 0x0F;
        uint8_t p15 = (buttons >> 4) & 0x0F;
        m_p14 |= p14;
        m_p15 |= p15;
        write_p1(m_p1);
    }
}

void age::gb_joypad::set_buttons_down(int buttons)
{
    if (buttons != 0)
    {
        LOG(buttons);
        uint8_t p14 = buttons & 0x0F;
        uint8_t p15 = (buttons >> 4) & 0x0F;
        m_p14 &= ~p14;
        m_p15 &= ~p15;
        write_p1(m_p1);
    }
}



age::gb_joypad::gb_joypad(gb_core &core)
    : m_core(core),
      m_p1(m_core.is_cgb_hardware() ? 0xFF : 0xCF)
{
}
