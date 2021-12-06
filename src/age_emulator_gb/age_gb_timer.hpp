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

#ifndef AGE_GB_TIMER_HPP
#define AGE_GB_TIMER_HPP

//!
//! \file
//!

#include <age_types.hpp>

#include "common/age_gb_clock.hpp"
#include "common/age_gb_device.hpp"
#include "common/age_gb_events.hpp"
#include "common/age_gb_interrupts.hpp"



namespace age
{

    class gb_timer
    {
        AGE_DISABLE_COPY(gb_timer);
        AGE_DISABLE_MOVE(gb_timer);

    public:
        gb_timer(const gb_device&      device,
                 const gb_clock&       clock,
                 gb_interrupt_trigger& interrupts,
                 gb_events&            events);

        ~gb_timer() = default;

        uint8_t               read_tima();
        [[nodiscard]] uint8_t read_tma() const;
        [[nodiscard]] uint8_t read_tac() const;

        void write_tima(uint8_t value);
        void write_tma(uint8_t value);
        void write_tac(uint8_t value);

        void trigger_interrupt();
        void update_state();
        void set_back_clock(int clock_cycle_offset);

        void after_speed_change();
        void after_div_reset(bool during_stop);

    private:
        // logging code is header-only to allow for compile time optimization
        [[nodiscard]] gb_log_message_stream log() const
        {
            return m_clock.log(gb_log_category::lc_timer);
        }

        [[nodiscard]] uint8_t get_clock_shift() const;

        bool update_timer_state();
        void start_timer();
        void stop_timer();
        void set_clk_timer_zero(int new_clk_timer_zero);

        const gb_device&      m_device;
        const gb_clock&       m_clock;
        gb_interrupt_trigger& m_interrupts;
        gb_events&            m_events;

        int     m_clk_timer_zero    = gb_no_clock_cycle;
        int     m_clk_last_overflow = gb_no_clock_cycle;
        uint8_t m_clock_shift       = 0;

        uint8_t m_tima = 0;
        uint8_t m_tma  = 0;
        uint8_t m_tac  = 0xF8;
    };

} // namespace age



#endif // AGE_GB_TIMER_HPP
