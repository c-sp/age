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

#include "age_gb_div.hpp"



age::gb_div::gb_div(const gb_clock &clock)
    : m_clock(clock)
{
}



int age::gb_div::get_div_offset() const
{
    return m_div_offset;
}



age::uint8_t age::gb_div::read_div() const
{
    //! \todo examine DIV behavior during speed change
    int clk = m_clock.get_clock_cycle();
    int shift = m_clock.is_double_speed() ? 7 : 8;
    uint8_t result = ((clk + m_div_offset) >> shift) & 0xFF;

    AGE_GB_CLOG_DIV("read DIV " AGE_LOG_HEX8(result));
    return result;
}

int age::gb_div::write_div()
{
    int old_offset = m_div_offset;

    int div_internal = m_clock.get_clock_cycle() & 0xFFFF;
    m_div_offset = 0x10000 - div_internal;
    AGE_GB_CLOG_DIV("reset DIV, changing offset from " << AGE_LOG_HEX16(old_offset)
                    << " to " AGE_LOG_HEX16(m_div_offset));

    return old_offset;
}
