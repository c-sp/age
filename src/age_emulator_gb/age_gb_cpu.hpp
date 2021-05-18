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

#ifndef AGE_GB_CPU_HPP
#define AGE_GB_CPU_HPP

//!
//! \file
//!

#include <age_types.hpp>
#include <emulator/age_gb_types.hpp>

#include "age_gb_bus.hpp"
#include "age_gb_timer.hpp"
#include "common/age_gb_clock.hpp"
#include "common/age_gb_device.hpp"
#include "common/age_gb_interrupts.hpp"



namespace age
{

    constexpr int8_t gb_cpu_state_ei     = 0x01;
    constexpr int8_t gb_cpu_state_frozen = 0x02;



    class gb_cpu
    {
        AGE_DISABLE_COPY(gb_cpu);

    public:
        gb_cpu(const gb_device&         device,
               gb_clock&                clock,
               gb_interrupt_dispatcher& interrupts,
               gb_bus&                  bus);

        [[nodiscard]] gb_test_info get_test_info() const;
        void                       emulate();

    private:
        void dispatch_interrupt();
        void handle_state();

        void    set_flags(int from_value);
        void    tick_push_byte(int byte);
        uint8_t tick_read_byte(int address);
        void    execute_prefetched();

        const gb_device&         m_device;
        gb_clock&                m_clock;
        gb_interrupt_dispatcher& m_interrupts;
        gb_bus&                  m_bus;

        // The following values store the results of arithmetic/logical
        // Gameboy CPU operations.
        // Storing them as int16_t would be sufficient,
        // but we use int for better code readability (no casts required).
        int m_zero_indicator  = 0;
        int m_carry_indicator = 0;
        int m_hcs_flags       = 0; //!< first operand and additional flags of the last instruction relevant for subtract- and half-carry-flag
        int m_hcs_operand     = 0; //!< second operand of the last instruction relevant for subtract- and half-carry-flag

        uint16_t m_pc = 0;
        uint16_t m_sp = 0;

        uint8_t m_a = 0;
        uint8_t m_b = 0;
        uint8_t m_c = 0;
        uint8_t m_d = 0;
        uint8_t m_e = 0;
        uint8_t m_h = 0;
        uint8_t m_l = 0;

        uint8_t m_prefetched_opcode = 0;
        int8_t  m_cpu_state         = 0;

        bool m_ld_b_b = false; //!< used to indicate a finished test rom
    };

} // namespace age



#endif // AGE_GB_CPU_HPP
