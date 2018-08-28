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

#ifndef AGE_GB_JOYPAD_HPP
#define AGE_GB_JOYPAD_HPP

//!
//! \file
//!

#include <age_non_copyable.hpp>
#include <age_types.hpp>

#include "age_gb_core.hpp"



namespace age
{

class gb_joypad : public non_copyable
{
public:

    uint8 read_p1() const;
    void write_p1(uint8 byte);
    void set_buttons_down(int32_t buttons);
    void set_buttons_up(int32_t buttons);

    gb_joypad(gb_core &core);

private:

    gb_core &m_core;
    uint8 m_p1;
    uint8 m_p14 = 0x0F;
    uint8 m_p15 = 0x0F;
};

} // namespace age



#endif // AGE_GB_JOYPAD_HPP
