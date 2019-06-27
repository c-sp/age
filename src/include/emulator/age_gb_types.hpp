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

#ifndef AGE_GB_TYPES_HPP
#define AGE_GB_TYPES_HPP

//!
//! \file
//!

#include <age_types.hpp>



namespace age
{

//!
//! \brief Configures the hardware (Gameboy or Gameboy Color) to emulate.
//!
//! Setting specific hardware is usually only required for running tests.
//! The end user should probably let the emulator auto detect the hardware
//! required by the rom to run.
//!
enum class gb_hardware
{
    auto_detect,
    cgb,
    dmg
};



//!
//! \brief The operating mode defined by configured hardware and rom capabilities.
//!
enum class gb_mode
{
    dmg,
    dmg_on_cgb,
    cgb
};



//!
//! \brief Struct containing parts of the Gameboy CPU state.
//!
//! This struct was implemented for evaluating test rom results.
//!
struct gb_test_info
{
    gb_mode m_mode;
    bool m_mooneye_debug_op;

    uint8_t m_a = 0;
    uint8_t m_b = 0;
    uint8_t m_c = 0;
    uint8_t m_d = 0;
    uint8_t m_e = 0;
    uint8_t m_h = 0;
    uint8_t m_l = 0;
};

} // namespace age



#endif // AGE_GB_TYPES_HPP
