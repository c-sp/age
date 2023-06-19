//
// Copyright 2022 Christoph Sprenger
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

#include "age_gb_memory.hpp"

#include <cassert>

namespace
{
    constexpr int idx_rtc_seconds = 0;
    constexpr int idx_rtc_minutes = 1;
    constexpr int idx_rtc_hours   = 2;
    constexpr int idx_rtc_days    = 3;
    constexpr int idx_rtc_control = 4;

    constexpr int rtc_reg_seconds = 8;
    constexpr int rtc_reg_minutes = 9;
    constexpr int rtc_reg_hours   = 10;
    constexpr int rtc_reg_days    = 11;
    constexpr int rtc_reg_control = 12;

    constexpr uint8_t rtc_day_bit9   = 0x01;
    constexpr uint8_t rtc_stop_timer = 0x40;
    constexpr uint8_t rtc_day_carry  = 0x80;

    constexpr age::uint8_array<5> rtc_reg_bits{0x3F, 0x3F, 0x1F, 0xFF, 0xC1};
    constexpr std::array<int, 5>  rtc_reg_limits{60, 60, 24, 256, 0};

    constexpr bool is_valid_rtc_reg(uint8_t rtc_reg)
    {
        return (rtc_reg >= rtc_reg_seconds) && (rtc_reg <= rtc_reg_control);
    }

    constexpr const char* get_rtc_reg_name(uint8_t rtc_reg)
    {
        switch (rtc_reg)
        {
            case rtc_reg_seconds: return "0x8 (seconds)";
            case rtc_reg_minutes: return "0x9 (minutes)";
            case rtc_reg_hours: return "0xA (hours)";
            case rtc_reg_days: return "0xB (days)";
            case rtc_reg_control: return "0xC (control)";
            default: return "(unknown)";
        }
    }

    void log_rtc_values(age::gb_log_message_stream& msg,
                        const age::uint8_array<5>&  rtc_reg)
    {
        msg << "control: " << age::log_hex8(rtc_reg[idx_rtc_control])
            << ", days: " << age::log_hex8(rtc_reg[idx_rtc_days])
            << ", hours: " << age::log_hex8(rtc_reg[idx_rtc_hours])
            << ", minutes: " << age::log_hex8(rtc_reg[idx_rtc_minutes])
            << ", seconds: " << age::log_hex8(rtc_reg[idx_rtc_seconds]);
    }

    template<int rtc_reg>
    int update_rtc_reg(age::uint8_array<5>& regs, int add)
    {
        int     reg_idx = rtc_reg - rtc_reg_seconds;
        int     factor  = rtc_reg_limits[reg_idx];
        uint8_t bits    = rtc_reg_bits[reg_idx];

        int sum = regs[reg_idx] + add;

        // Special handling for out-of-bounds values:
        // a carry to the next higher register happens only when hitting
        // the exact required value, e.g. 60 seconds, not when seconds >= 60.
        // If seconds have been set to anything >= 60, the value wraps
        // around to zero before any carry.
        if (regs[reg_idx] >= rtc_reg_limits[reg_idx])
        {
            // no wraparound to zero yet
            if (sum <= bits)
            {
                regs[reg_idx] = sum;
                return 0;
            }
            // simulate wraparound to zero and continue as usual
            sum -= bits + 1;
        }

        assert(sum >= 0);
        int carry     = sum / factor;
        regs[reg_idx] = sum % factor;

        return carry;
    }

} // namespace



void age::gb_memory::mbc3rtc_write(gb_memory& memory, uint16_t address, uint8_t value)
{
    auto& mbc_data = memory.get_mbc_data<gb_mbc3rtc_data>();

    assert(address < 0x8000);
    switch (address & 0x6000)
    {
        case 0x0000:
        case 0x2000:
            mbc3_write(memory, address, value);
            break;

        case 0x4000:
            mbc_data.m_mapped_rtc_reg = value;
            if (is_valid_rtc_reg(mbc_data.m_mapped_rtc_reg))
            {
                memory.log_mbc() << "mapping RTC register "
                                 << get_rtc_reg_name(mbc_data.m_mapped_rtc_reg)
                                 << " to cartridge ram area";
            }
            else
            {
                mbc3_write(memory, address, value);
            }
            break;

        case 0x6000:
            // latch real time clock
            if ((mbc_data.m_last_latch_write == 0) && (value == 1))
            {
                mbc3rtc_update(memory);
                mbc_data.m_rtc_reg_latched = mbc_data.m_rtc_reg;

                auto msg = memory.log_mbc();
                msg << "latching RTC values, ";
                log_rtc_values(msg, mbc_data.m_rtc_reg_latched);
            }
            mbc_data.m_last_latch_write = value;
            break;
    }
}

void age::gb_memory::mbc3rtc_cart_ram_write(gb_memory& memory, uint16_t address, uint8_t value)
{
    auto& mbc_data = memory.get_mbc_data<gb_mbc3rtc_data>();

    auto rtc_reg = mbc_data.m_mapped_rtc_reg;
    if (is_valid_rtc_reg(rtc_reg))
    {
        mbc3rtc_update(memory);

        // reduce to writable bits
        value &= rtc_reg_bits[rtc_reg - rtc_reg_seconds];

        mbc_data.m_rtc_reg[rtc_reg - rtc_reg_seconds] = value;
        memory.log_mbc() << "writing RTC register "
                         << get_rtc_reg_name(rtc_reg) << " = " << log_hex8(value);

        if (rtc_reg == rtc_reg_seconds)
        {
            mbc_data.m_clks_sec_remainder = 0;
        }
    }
    else
    {
        cart_ram_write(memory, address, value);
    }
}

age::uint8_t age::gb_memory::mbc3rtc_cart_ram_read(gb_memory& memory, uint16_t address)
{
    auto& mbc_data = memory.get_mbc_data<gb_mbc3rtc_data>();

    if (is_valid_rtc_reg(mbc_data.m_mapped_rtc_reg))
    {
        auto result = mbc_data.m_rtc_reg_latched[mbc_data.m_mapped_rtc_reg - 8];

        memory.log_mbc() << "reading latched RTC register "
                         << get_rtc_reg_name(mbc_data.m_mapped_rtc_reg)
                         << " == " << log_hex8(result);

        return result;
    }
    return cart_ram_read(memory, address);
}



void age::gb_memory::mbc3rtc_update(gb_memory& memory)
{
    auto& mbc_data = memory.get_mbc_data<gb_mbc3rtc_data>();

    // RTC stopped
    if (mbc_data.m_rtc_reg[idx_rtc_control] & rtc_stop_timer)
    {
        memory.log_mbc() << "no RTC update: RTC switched off";
        mbc_data.m_clks_last_update = memory.m_clock.get_clock_cycle();
        return;
    }

    // calculate number of seconds elapsed since last full second
    int clk_current             = memory.m_clock.get_clock_cycle();
    int clks_diff               = clk_current - mbc_data.m_clks_last_update + mbc_data.m_clks_sec_remainder;
    mbc_data.m_clks_last_update = clk_current;

    auto msg = memory.log_mbc();
    msg << "updating RTC"
        << "\n    * update cycles: " << clks_diff;

    // no RTC tick?
    if (clks_diff < gb_clock_cycles_per_second)
    {
        mbc_data.m_clks_sec_remainder = clks_diff;
        msg << ", no RTC update required"
            << "\n    * remaining sub-second cycles: " << mbc_data.m_clks_sec_remainder;
        return;
    }

    // update RTC registers
    int seconds                   = clks_diff / gb_clock_cycles_per_second;
    mbc_data.m_clks_sec_remainder = clks_diff % gb_clock_cycles_per_second;
    msg << ", " << seconds << " completed second(s)";

    int minutes = update_rtc_reg<rtc_reg_seconds>(mbc_data.m_rtc_reg, seconds);
    int hours   = update_rtc_reg<rtc_reg_minutes>(mbc_data.m_rtc_reg, minutes);
    int days    = update_rtc_reg<rtc_reg_hours>(mbc_data.m_rtc_reg, hours);

    days += ((mbc_data.m_rtc_reg[idx_rtc_control] & 1) ? 256 : 0);
    int days_high_bits = update_rtc_reg<rtc_reg_days>(mbc_data.m_rtc_reg, days);

    uint8_t days_msb   = (days_high_bits & 1) ? rtc_day_bit9 : 0;
    uint8_t days_carry = (mbc_data.m_rtc_reg[idx_rtc_control] & rtc_day_carry)
                         | ((days_high_bits >= 2) ? rtc_day_carry : 0);

    mbc_data.m_rtc_reg[idx_rtc_control] = days_msb | days_carry;

    msg << "\n    * ";
    log_rtc_values(msg, mbc_data.m_rtc_reg);
    msg << "\n    * remaining sub-second cycles: " << mbc_data.m_clks_sec_remainder;
}
