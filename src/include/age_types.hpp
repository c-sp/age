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

#ifndef AGE_TYPES_HPP
#define AGE_TYPES_HPP

//!
//! \file
//!

#include <array>
#include <cstdint> // std::uint8_t etc.
#include <limits> // std::numeric_limits
#include <string>
#include <type_traits> // std::underlying_type
#include <vector>



namespace age {

// typedefs

typedef std::uint8_t  uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;

typedef std::int8_t  int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;

//!
//! Defines an unsigned integer type that matches the architecture's default CPU register
//! size (32 bits or 64 bits) but is guaranteed to be at least 32 bits wide.
//!
typedef size_t uint;

template<uint _size>
using uint8_array = std::array<uint8, _size>;

typedef std::vector<uint8> uint8_vector;
typedef std::vector<std::string> string_vector;



// constants and constant expressions

const uint8_vector empty_uint8_vector = uint8_vector();

constexpr const char *project_name = "AGE";
constexpr const char *project_version = "1.0";

constexpr uint uint_max = std::numeric_limits<uint>::max();

//!
//! Convert the specified enum value to the corresponding value of the underlying type.
//!
template<typename E>
constexpr auto to_integral(E e) -> typename std::underlying_type<E>::type
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

} // namespace age



#endif // AGE_TYPES_HPP
