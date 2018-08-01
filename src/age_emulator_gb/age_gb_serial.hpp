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

class gb_serial : public non_copyable
{
public:

    uint8 read_sb() const;
    uint8 read_sc() const;

    void write_sb(uint8 value);
    void write_sc(uint8 value);

    void finish_transfer();

    gb_serial(gb_core &core);

private:

    uint8 m_sb = 0;
    uint8 m_sc = 0;
    gb_core &m_core;
};

} // namespace age



#endif // AGE_GB_SERIAL_HPP
