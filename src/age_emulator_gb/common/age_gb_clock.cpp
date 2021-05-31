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



void age::gb_set_back_clock_cycle(int& clock_cycle, int cycle_offset)
{
    if (clock_cycle == gb_no_clock_cycle)
    {
        return;
    }

    AGE_ASSERT(cycle_offset >= gb_clock_cycles_per_second)
    AGE_ASSERT((cycle_offset % gb_clock_cycles_per_second) == 0)

    AGE_ASSERT(clock_cycle >= cycle_offset)
    clock_cycle -= cycle_offset;
}



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
    AGE_GB_CLOG_CLOCK("clock initialized to " << AGE_LOG_HEX(m_clock_cycle))
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
    AGE_GB_CLOG_CLOCK("set back clock to " << (m_clock_cycle - clock_cycle_offset)
                                           << " (-" << clock_cycle_offset << ")")

    gb_set_back_clock_cycle(m_clock_cycle, clock_cycle_offset);
}



bool age::gb_clock::tick_speed_change_delay()
{
    if ((m_key1 & 1) == 0)
    {
        return false;
    }
    m_clock_cycle += is_double_speed() ? 0x10000 : 0x20000;
    return true;
}

void age::gb_clock::change_speed()
{
    AGE_ASSERT(m_key1 & 1);

    // toggle double speed
    m_key1 ^= 0x81;
    const bool double_speed = (m_key1 & 0x80) > 0;
    m_machine_cycle_clocks  = double_speed ? 2 : 4;

    AGE_GB_CLOG_CLOCK("double speed "
                      << (double_speed ? "activated" : "deactivated"))
}



age::uint8_t age::gb_clock::read_key1() const
{
    AGE_GB_CLOG_CLOCK("read key1 = " << AGE_LOG_HEX8(m_key1))
    return m_key1;
}

void age::gb_clock::write_key1(uint8_t value)
{
    m_key1 = (m_key1 & 0xFE) | (value & 0x01);
    AGE_GB_CLOG_CLOCK("write key1 = " << AGE_LOG_HEX8(m_key1))
}
