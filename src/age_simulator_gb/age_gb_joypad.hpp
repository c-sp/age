//
// Copyright (c) 2010-2017 Christoph Sprenger
//
// This file is part of AGE ("Another Gameboy Emulator").
// <https://gitlab.com/csprenger/AGE>
//
// AGE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// AGE is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with AGE.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AGE_GB_JOYPAD_HPP
#define AGE_GB_JOYPAD_HPP

//!
//! \file
//!

#include "age_gb_core.hpp"



namespace age
{

class gb_joypad : public non_copyable
{
public:

    uint8 read_p1() const;
    void write_p1(uint8 byte);
    void set_buttons_down(uint buttons);
    void set_buttons_up(uint buttons);

    gb_joypad(gb_core &core);

private:

    gb_core &m_core;
    uint8 m_p1 = 0xCF;
    uint8 m_p14 = 0x0F;
    uint8 m_p15 = 0x0F;
};

} // namespace age



#endif // AGE_GB_JOYPAD_HPP
