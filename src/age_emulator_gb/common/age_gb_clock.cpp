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

#include "age_gb_clock.hpp"



age::gb_clock::gb_clock(const gb_device& device)
{
    if (device.is_cgb())
    {
        // Gambatte tests:
        // div/start_inc_1_cgb_out1E
        // div/start_inc_2_cgb_out1F
        m_clock_cycle = 0x1F * 0x100;
        m_clock_cycle -= 96;
    }
    else if (device.is_cgb_hardware())
    {
        // Mooneye GB tests:
        // misc/boot_div-cgbABCDE
        m_clock_cycle = 0x27 * 0x100;
        m_clock_cycle -= 136;
    }
    else
    {
        // Mooneye GB tests:
        // acceptance/boot_div-dmgABCmgb
        //
        // Gambatte tests:
        // div/start_inc_1_dmg08_outAB
        // div/start_inc_2_dmg08_outAC
        m_clock_cycle = 0xAC * 0x100;
        m_clock_cycle -= 52;
    }
    AGE_GB_CLOG_CLOCK("clock initialized")
    AGE_ASSERT((m_clock_cycle % 4) == 0);
}



int age::gb_clock::get_clock_cycle() const
{
    AGE_ASSERT(m_clock_cycle >= 0)
    return m_clock_cycle;
}

age::int8_t age::gb_clock::get_machine_cycle_clocks() const
{
    return m_machine_cycle_clocks;
}

bool age::gb_clock::is_double_speed() const
{
    return m_machine_cycle_clocks == 2;
}



void age::gb_clock::tick_machine_cycle()
{
    m_clock_cycle += get_machine_cycle_clocks();
}

void age::gb_clock::tick_2_clock_cycles()
{
    m_clock_cycle += 2;
}

void age::gb_clock::set_back_clock(int clock_cycle_offset)
{
    AGE_GB_SET_BACK_CLOCK(m_clock_cycle, clock_cycle_offset)
}



bool age::gb_clock::trigger_speed_change()
{
    if ((m_key1 & 0x01U) == 0)
    {
        // no speed change requested
        return false;
    }

    // toggle double speed
    m_key1 ^= 0x81U;
    const bool double_speed = (m_key1 & 0x80U) > 0;
    m_machine_cycle_clocks  = double_speed ? 2 : 4;

    AGE_GB_CLOG_CLOCK("double speed "
                      << (double_speed ? "activated" : "deactivated"))
    return true;
}

age::uint8_t age::gb_clock::read_key1() const
{
    AGE_GB_CLOG_CLOCK("read key1 = " << AGE_LOG_HEX8(m_key1))
    return m_key1;
}

void age::gb_clock::write_key1(uint8_t value)
{
    m_key1 = (m_key1 & 0xfeU) | (value & 0x01U);
    AGE_GB_CLOG_CLOCK("write key1 = " << AGE_LOG_HEX8(m_key1))
}
