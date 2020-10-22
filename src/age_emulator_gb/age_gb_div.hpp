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

#ifndef AGE_GB_DIV_HPP
#define AGE_GB_DIV_HPP

//!
//! \file
//!

#include <age_types.hpp>

#include "common/age_gb_clock.hpp"



namespace age
{

class gb_div
{
    AGE_DISABLE_COPY(gb_div);

public:

    gb_div(const gb_clock &clock);

    int get_div_offset() const;

    uint8_t read_div() const;
    int write_div();

private:

    const gb_clock &m_clock;
    int m_div_offset = 0;
};

} // namespace age



#endif // AGE_GB_DIV_HPP
