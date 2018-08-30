//
// Copyright 2018 Christoph Sprenger
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

#include <age_non_copyable.hpp>
#include <age_types.hpp>
#include <emulator/age_gb_types.hpp>

#include "age_gb_bus.hpp"



namespace age
{



class gb_cpu : public non_copyable
{
public:

    void emulate_instruction();

    gb_cpu(gb_core &core, gb_bus &bus);

    gb_test_info get_test_info() const;

private:

    gb_core &m_core;
    gb_bus &m_bus;

    // The following values store the results of arithmetic/logical
    // Gameboy CPU operations.
    // Storing them as int16_t would be sufficient,
    // but we use int for better code readability
    // (no casts required).
    int m_zero_indicator = 0;
    int m_carry_indicator = 0;
    int m_hcs_flags = 0; //!< first operand and additional flags of the last instruction relevant for subtract- and half-carry-flag
    int m_hcs_operand = 0; //!< second operand of the last instruction relevant for subtract- and half-carry-flag

    uint16_t m_pc = 0;
    uint16_t m_sp = 0;

    uint8_t m_a = 0;
    uint8_t m_b = 0;
    uint8_t m_c = 0;
    uint8_t m_d = 0;
    uint8_t m_e = 0;
    uint8_t m_h = 0;
    uint8_t m_l = 0;

    uint8_t m_pc_increment = 1; //!< used for HALT emulation
    bool m_mooneye_debug_op = false; //!< used to indicate the finishing of a mooneye-gb test
};

} // namespace age



#endif // AGE_GB_CPU_HPP
