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



age::gb_clock::gb_clock(gb_logger& logger, const gb_device& device)
    : m_logger(logger)
{
    if (device.cgb_mode())
    {
        // Gambatte tests:
        // div/start_inc_1_cgb_out1E
        // div/start_inc_2_cgb_out1F
        m_clock_cycle = 0x1F * 0x100;
        m_clock_cycle -= 96;
    }
    else if (device.cgb_in_dmg_mode())
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
    log() << "clock initialized to " << log_hex(m_clock_cycle);
    AGE_ASSERT((m_clock_cycle % 4) == 0);
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
    log() << "set back clock to " << (m_clock_cycle - clock_cycle_offset)
          << " (-" << clock_cycle_offset << ")";

    gb_set_back_clock_cycle(m_clock_cycle, clock_cycle_offset);
}



void age::gb_clock::tick_speed_change_delay()
{
    // same number of machine cycles, different number of T4 cycles
    int delay = is_double_speed() ? 0x10000 : 0x20000;
    log() << "applying speed change delay of " << log_hex(delay) << " clock cycles";
    m_clock_cycle += delay;
}

bool age::gb_clock::change_speed()
{
    if ((m_key1 & 1) == 0)
    {
        return false;
    }

    // toggle double speed
    m_key1 ^= 0x81;
    const bool double_speed = (m_key1 & 0x80) > 0;
    m_machine_cycle_clocks  = double_speed ? 2 : 4;

    log() << ((double_speed ? "double speed activated" : "single speed activated"));
    return true;
}



age::uint8_t age::gb_clock::read_key1() const
{
    log() << "read key1 == " << log_hex8(m_key1);
    return m_key1;
}

void age::gb_clock::write_key1(uint8_t value)
{
    m_key1 = (m_key1 & 0xFE) | (value & 0x01);
    log() << "write key1 = " << log_hex8(m_key1);
}



age::gb_div_reset_details age::gb_clock::get_div_reset_details(int lowest_counter_bit) const
{
    int lower_bits  = lowest_counter_bit - 1;
    int trigger_bit = lowest_counter_bit / 2;
    AGE_ASSERT((lowest_counter_bit & lower_bits) == 0); // only one bit set

    // calculate old and new (current) div-aligned clock
    int old_clock = m_clock_cycle + m_old_div_offset;
    int new_clock = m_clock_cycle + m_div_offset;

    // calculate the number of clock cycles until the next
    // counter increment for both clocks (old & new)
    gb_div_reset_details details;
    details.m_old_next_increment = lowest_counter_bit - (old_clock & lower_bits);
    details.m_new_next_increment = lowest_counter_bit - (new_clock & lower_bits);

    // if the bit triggering a counter increment is switched from
    // high to low by the div reset,
    // the counter is immediately incremented
    int old_trigger_bit = old_clock & trigger_bit;
    int new_trigger_bit = new_clock & trigger_bit;

    details.m_clk_adjust = (old_trigger_bit && !new_trigger_bit)
                               // trigger bit goes low
                               //      => immediate counter increment
                               //      => time to counter overflow decreased
                               ? -details.m_old_next_increment
                               // trigger bit not going low
                               //      => time to counter overflow increased
                               : details.m_new_next_increment - details.m_old_next_increment;
    return details;
}



int age::gb_clock::get_div_offset() const
{
    return m_div_offset;
}



age::uint8_t age::gb_clock::read_div() const
{
    int     shift       = is_double_speed() ? 7 : 8;
    int     result      = (m_clock_cycle + m_div_offset) >> shift;
    uint8_t result_byte = result & 0xFF;

    log() << "read DIV == " << log_hex8(result_byte)
          << "\n    * last increment on clock cycle " << ((result << shift) - m_div_offset)
          << "\n    * next increment on clock cycle " << (((result + 1) << shift) - m_div_offset);

    return result_byte;
}

void age::gb_clock::write_div()
{
    int div_counter    = m_clock_cycle & 0xFFFF;
    int new_div_offset = 0x10000 - div_counter;

    // log before adjusting m_div_offset
    log() << "DIV reset, changed DIV offset from " << log_hex16(m_div_offset)
          << " to " << log_hex16(new_div_offset);

    m_old_div_offset = m_div_offset;
    m_div_offset     = new_div_offset;
}
