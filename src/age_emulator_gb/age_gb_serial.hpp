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

#ifndef AGE_GB_SERIAL_HPP
#define AGE_GB_SERIAL_HPP

//!
//! \file
//!

#include "age_gb_core.hpp"



namespace age
{

enum class gb_serial_state
{
    idle,
    waiting_for_external,
    during_transfer_external,
    during_transfer_internal
};



class gb_serial : public non_copyable
{
public:

    uint8 read_sb() const;
    uint8 read_sc() const;

    void write_sb(uint8 value);
    void write_sc(uint8 value);

    void emulate(uint cycles_elapsed);
    void switch_double_speed_mode();

    gb_serial(gb_core &core);

private:

    void finish_transfer();

    gb_serial_state m_state = gb_serial_state::idle;
    uint m_cycles_left = 0;
    uint8 m_sb = 0;
    uint8 m_sc = 0;
    gb_core &m_core;
    const bool m_is_cgb;
    uint8_vector m_send_buffer;
    uint8_vector m_receive_buffer;
};

} // namespace age



#endif // AGE_GB_SERIAL_HPP
