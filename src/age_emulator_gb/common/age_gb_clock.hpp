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
#include "age_gb_logger.hpp"



namespace age
{

    constexpr int gb_no_clock_cycle          = -1;
    constexpr int gb_clock_cycles_per_second = 4194304;

    void gb_set_back_clock_cycle(int& clock_cycle, int cycle_offset);



    struct gb_div_reset_details
    {
        //! the next counter increment aligned with the previous DIV
        //! (relative to the current clock cycle)
        int m_old_next_increment = 0;

        //! the next counter increment aligned with the current DIV reset
        //! (relative to the current clock cycle)
        int m_new_next_increment = 0;

        //! positive value:
        //! the next counter increment is delayed by the DIV reset
        //! (m_new_next_increment - m_old_next_increment).
        //!
        //! negative value:
        //! the DIV reset causes an immediate counter increment
        //! (-m_old_next_increment).
        int m_clk_adjust = 0;
    };



    class gb_clock
    {
        AGE_DISABLE_COPY(gb_clock);
        AGE_DISABLE_MOVE(gb_clock);

    public:
        explicit gb_clock(gb_logger& logger, const gb_device& device);
        ~gb_clock() = default;

        //! \brief Get the current 4Mhz cycle.
        //!
        //! This clock runs at 4Mhz regardless of the current
        //! Game Boy Color speed setting.
        [[nodiscard]] int get_clock_cycle() const
        {
            AGE_ASSERT(m_clock_cycle >= 0)
            return m_clock_cycle;
        }

        [[nodiscard]] int8_t get_machine_cycle_clocks() const
        {
            return m_machine_cycle_clocks;
        }

        [[nodiscard]] bool is_double_speed() const
        {
            return m_machine_cycle_clocks == 2;
        }

        void tick_machine_cycle();
        void tick_2_clock_cycles();
        void set_back_clock(int clock_cycle_offset);

        void tick_speed_change_delay();
        bool change_speed();

        [[nodiscard]] uint8_t read_key1() const;
        void                  write_key1(uint8_t value);

        [[nodiscard]] gb_div_reset_details get_div_reset_details(int lowest_counter_bit) const;
        [[nodiscard]] int                  get_div_offset() const;

        [[nodiscard]] uint8_t read_div() const;
        void                  write_div();

        // logging code is header-only to allow compile time optimization
        [[nodiscard]] gb_log_message_stream log(gb_log_category category) const
        {
            return log(category, m_clock_cycle);
        }
        [[nodiscard]] gb_log_message_stream log(gb_log_category category, int clock_cycle) const
        {
            int div_clock = clock_cycle + m_div_offset;
            return m_logger.log(category, clock_cycle, div_clock);
        }

    private:
        // logging code is header-only to allow compile time optimization
        [[nodiscard]] gb_log_message_stream log() const
        {
            return log(gb_log_category::lc_clock, m_clock_cycle);
        }

        gb_logger& m_logger;

        int     m_clock_cycle          = 0;
        int8_t  m_machine_cycle_clocks = 4;
        uint8_t m_key1                 = 0x7E;

        int m_old_div_offset = 0; //!< clock offset before the last reset
        int m_div_offset     = 0; //!< used to re-align DIV to clock on DIV-writes
    };

} // namespace age



#endif // AGE_GB_CLOCK_HPP
