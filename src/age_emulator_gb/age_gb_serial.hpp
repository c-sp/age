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

#ifndef AGE_GB_SERIAL_HPP
#define AGE_GB_SERIAL_HPP

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

    enum class gb_sio_state
    {
        no_transfer,
        transfer_external_clock,
        transfer_internal_clock
    };



    class gb_serial
    {
        AGE_DISABLE_COPY(gb_serial);
        AGE_DISABLE_MOVE(gb_serial);

    public:
        gb_serial(const gb_device&      device,
                  const gb_clock&       clock,
                  gb_interrupt_trigger& interrupts,
                  gb_events&            events);
        ~gb_serial() = default;

        uint8_t               read_sb();
        [[nodiscard]] uint8_t read_sc() const;

        void write_sb(uint8_t value);
        void write_sc(uint8_t value);

        void update_state();
        void after_div_reset();
        void set_back_clock(int clock_cycle_offset);

    private:
        // logging code is header-only to allow compile time optimization
        [[nodiscard]] gb_log_message_stream log() const
        {
            return m_clock.log(gb_log_category::lc_serial);
        }

        void start_transfer(uint8_t value_sc);
        void stop_transfer(gb_sio_state new_state);

        const gb_device&      m_device;
        const gb_clock&       m_clock;
        gb_interrupt_trigger& m_interrupts;
        gb_events&            m_events;

        gb_sio_state m_sio_state       = gb_sio_state::no_transfer;
        int          m_sio_clk_started = gb_no_clock_cycle;
        int          m_sio_clock_shift = 0;
        uint8_t      m_sio_initial_sb  = 0;

        uint8_t m_sb = 0;
        uint8_t m_sc = 0;
    };

} // namespace age



#endif // AGE_GB_SERIAL_HPP
