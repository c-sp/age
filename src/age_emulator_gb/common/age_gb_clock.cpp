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

#include "age_gb_clock.hpp"

#if AGE_GB_CLOG_CLOCK
#define CLOG(log) AGE_LOG("clock " << get_clock_cycle() << ": " << log)
#else
#define CLOG(log)
#endif



age::gb_clock::gb_clock(const gb_device &device)
{
    if (device.is_cgb_hardware())
    {
        // Gambatte tests:
        // div/start_inc_1_cgb_out1E
        // div/start_inc_2_cgb_out1F
        m_clock_cycle = 0x1F * 0x100;
        m_clock_cycle -= 96;
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
    CLOG("clock initialized");
    AGE_ASSERT((m_clock_cycle % 4) == 0);
}



int age::gb_clock::get_clock_cycle() const
{
    AGE_ASSERT(m_clock_cycle >= 0);
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
    AGE_GB_SET_BACK_CLOCK(m_clock_cycle, clock_cycle_offset);
}



bool age::gb_clock::trigger_speed_change()
{
    if ((m_key1 & 0x01) == 0)
    {
        // no speed change requested
        return false;
    }

    // toggle double speed
    m_key1 ^= 0x81;
    const bool double_speed = (m_key1 & 0x80) > 0;
    m_machine_cycle_clocks = double_speed ? 2 : 4;

    CLOG("double speed " << (double_speed ? "activated" : "deactivated"));
    return true;
}

age::uint8_t age::gb_clock::read_key1() const
{
    CLOG("read key1 = " << AGE_LOG_HEX8(m_key1));
    return m_key1;
}

void age::gb_clock::write_key1(uint8_t value)
{
    m_key1 = (m_key1 & 0xFE) | (value & 0x01);
    CLOG("write key1 = " << AGE_LOG_HEX8(m_key1));
}
