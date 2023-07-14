//
// © 2018 Christoph Sprenger <https://github.com/c-sp>
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

#ifndef AGE_UTILITIES_HPP
#define AGE_UTILITIES_HPP

//!
//! \file
//!

#include <age_types.hpp>



namespace age
{

    uint32_t crc32(const uint8_vector& data);
    uint32_t crc32(uint8_vector::const_iterator begin, uint8_vector::const_iterator end);

} // namespace age



#endif // AGE_UTILITIES_HPP
