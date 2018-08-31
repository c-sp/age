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

#include <age_types.hpp>

#include "age_gb_core.hpp"



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

public:

    uint8_t read_sb();
    uint8_t read_sc() const;

    void write_sb(uint8_t value);
    void write_sc(uint8_t value);

    void finish_transfer();
    void set_back_cycles(int offset);

    gb_serial(gb_core &core);

private:

    int transfer_init(uint8_t value);
    void transfer_update_sb();

    gb_core &m_core;
    gb_sio_state m_sio_state = gb_sio_state::no_transfer;
    int m_sio_cycles_per_bit = 0;
    int m_sio_last_receive_cycle = gb_no_cycle;

    uint8_t m_sb = 0;
    uint8_t m_sc = 0;
};

} // namespace age



#endif // AGE_GB_SERIAL_HPP
