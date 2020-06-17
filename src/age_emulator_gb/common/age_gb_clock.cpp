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



age::gb_clock::gb_clock(const gb_device &device)
{
    //
    // verified by gambatte tests
    //
    // adjust the initial clock cycle
    // (due to the Gameboy playing the nintendo intro we're not starting with
    // cycle 0 at PC 0x0100)
    //
    //      div/start_inc_1_cgb_out1E
    //      div/start_inc_2_cgb_out1F
    //      div/start_inc_1_dmg08_outAB
    //      div/start_inc_2_dmg08_outAC
    //
    //      tima/tc00_start_1_outF0
    //      tima/tc00_start_2_outF1
    //
    if (device.is_cgb())
    {
        m_clock_cycle = 0x1F * 0x100;
        m_clock_cycle -= 96;
    }
    else
    {
        m_clock_cycle = 0xAC * 0x100;
        m_clock_cycle -= 52;
    }
}



int age::gb_clock::get_clock_cycle() const
{
    AGE_ASSERT(m_clock_cycle >= 0);
    return m_clock_cycle;
}

age::int8_t age::gb_clock::get_machine_cycle_clocks() const
{
    AGE_ASSERT((m_machine_cycle_clocks == 2) || (m_machine_cycle_clocks == 4));
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

void age::gb_clock::set_double_speed(bool is_double_speed)
{
    m_machine_cycle_clocks = is_double_speed ? 2 : 4;
}
