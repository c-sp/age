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

#ifndef AGE_GB_CLOCK_HPP
#define AGE_GB_CLOCK_HPP

//!
//! \file
//!

#include <age_debug.hpp>
#include <age_types.hpp>

#include "age_gb_device.hpp"



namespace age
{

    constexpr int gb_no_clock_cycle          = -1;
    constexpr int gb_clock_cycles_per_second = 4194304;

    void gb_set_back_clock_cycle(int &clock_cycle, int cycle_offset);



    class gb_clock
    {
    public:
        explicit gb_clock(const gb_device& device);

        //! \brief Get the current 4Mhz cycle.
        //!
        //! This clock runs at 4Mhz regardless of the current
        //! Game Boy Color speed setting.
        [[nodiscard]] int    get_clock_cycle() const;
        [[nodiscard]] int8_t get_machine_cycle_clocks() const;
        [[nodiscard]] bool   is_double_speed() const;

        void tick_machine_cycle();
        void tick_2_clock_cycles();
        void set_back_clock(int clock_cycle_offset);

        bool                  trigger_speed_change();
        [[nodiscard]] uint8_t read_key1() const;
        void                  write_key1(uint8_t value);

    private:
        int     m_clock_cycle          = 0;
        int8_t  m_machine_cycle_clocks = 4;
        uint8_t m_key1                 = 0x7E;
    };

} // namespace age



#define AGE_GB_CLOG(log) AGE_LOG("clock " << m_clock.get_clock_cycle() << " - " << log) // NOLINT(bugprone-macro-parentheses)

#if 0
#define AGE_GB_CLOG_CLOCK(log) AGE_LOG("clock " << get_clock_cycle() << " - " << log)
#else
#define AGE_GB_CLOG_CLOCK(log)
#endif

#if 0
#define AGE_GB_CLOG_CPU(log) AGE_GB_CLOG(log)
#else
#define AGE_GB_CLOG_CPU(log)
#endif

#if 0
#define AGE_GB_CLOG_DIV(log) AGE_GB_CLOG(log)
#else
#define AGE_GB_CLOG_DIV(log)
#endif

#if 0
#define AGE_GB_CLOG_EVENTS(log) AGE_GB_CLOG(log)
#else
#define AGE_GB_CLOG_EVENTS(log)
#endif

#if 0
#define AGE_GB_CLOG_IRQS(log) AGE_GB_CLOG(log)
#else
#define AGE_GB_CLOG_IRQS(log)
#endif

#if 0
#define AGE_GB_CLOG_LCD_OAM(log) AGE_GB_CLOG(log)
#else
#define AGE_GB_CLOG_LCD_OAM(log)
#endif

#if 0
#define AGE_GB_CLOG_LCD_PORTS(log) AGE_GB_CLOG(log)
#else
#define AGE_GB_CLOG_LCD_PORTS(log)
#endif

// LY is read quite often by some test roms (e.g. checking for v-blank)
//  => create an extra logging category for it
#if 0
#define AGE_GB_CLOG_LCD_PORTS_LY(log) AGE_GB_CLOG(log)
#else
#define AGE_GB_CLOG_LCD_PORTS_LY(log)
#endif

#if 0
#define AGE_GB_CLOG_LCD_RENDER(log) AGE_GB_CLOG(log)
#else
#define AGE_GB_CLOG_LCD_RENDER(log)
#endif

#if 0
#define AGE_GB_CLOG_LCD_VRAM(log) AGE_GB_CLOG(log)
#else
#define AGE_GB_CLOG_LCD_VRAM(log)
#endif

#if 0
#define AGE_GB_CLOG_SERIAL(log) AGE_GB_CLOG(log)
#else
#define AGE_GB_CLOG_SERIAL(log)
#endif

#if 0
#define AGE_GB_CLOG_SOUND(log) AGE_GB_CLOG(log)
#else
#define AGE_GB_CLOG_SOUND(log)
#endif

#if 0
#define AGE_GB_CLOG_TIMER(log) AGE_GB_CLOG(log)
#else
#define AGE_GB_CLOG_TIMER(log)
#endif



#endif // AGE_GB_CLOCK_HPP
