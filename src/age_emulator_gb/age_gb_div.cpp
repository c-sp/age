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

#include "age_gb_div.hpp"



age::gb_div::gb_div(const gb_clock& clock)
    : m_clock(clock)
{
}



age::gb_div_reset_details age::gb_div::calculate_reset_details(int lowest_counter_bit) const
{
    int lower_bits  = lowest_counter_bit - 1;
    int trigger_bit = lowest_counter_bit / 2;
    AGE_ASSERT((lowest_counter_bit & lower_bits) == 0); // only one bit set

    // calculate old and new (current) div-aligned clock
    int current_clk = m_clock.get_clock_cycle();
    int old_clock   = current_clk + m_old_div_offset;
    int new_clock   = current_clk + m_div_offset;

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



int age::gb_div::get_div_offset() const
{
    return m_div_offset;
}



age::uint8_t age::gb_div::read_div() const
{
    //! \todo examine DIV behavior during speed change
    int clk    = m_clock.get_clock_cycle();
    int shift  = m_clock.is_double_speed() ? 7 : 8;
    int result = (clk + m_div_offset) >> shift;

    AGE_GB_CLOG_DIV("read DIV " << AGE_LOG_HEX8(result & 0xFF))
    AGE_GB_CLOG_DIV("    * last increment on lock cycle "
                    << ((result << shift) - m_div_offset))
    AGE_GB_CLOG_DIV("    * next increment on lock cycle "
                    << (((result + 1) << shift) - m_div_offset))

    return result & 0xFF;
}

void age::gb_div::write_div()
{
    int div_internal = m_clock.get_clock_cycle() & 0xFFFF;
    m_old_div_offset = m_div_offset;
    m_div_offset     = 0x10000 - div_internal;
    AGE_GB_CLOG_DIV("reset DIV, changing offset from " << AGE_LOG_HEX16(m_old_div_offset)
                                                       << " to " AGE_LOG_HEX16(m_div_offset))
}
