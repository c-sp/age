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

#ifndef AGE_GB_SERIAL_HPP
#define AGE_GB_SERIAL_HPP

//!
//! \file
//!

#include <age_non_copyable.hpp>
#include <age_types.hpp>

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
