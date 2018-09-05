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
#include <limits>
#include <type_traits> // std::underlying_type
#include <vector>



#define AGE_DISABLE_COPY(Class) \
    private: \
    Class(const Class&) = delete; \
    Class& operator=(const Class&) = delete



namespace age
{

// C++ arithmetic operators do not accept types smaller than int
// (see https://en.cppreference.com/w/cpp/language/implicit_conversion).
// AGE relies on int being 32 bits or more.

static_assert(sizeof(int) >= 4, "AGE requires int being at least 32 bits wide");
static_assert(sizeof(unsigned) >= 4, "AGE requires unsigned int being at least 32 bits wide");
static_assert(sizeof(std::size_t) >= sizeof(int), "AGE requires std::size_t to be at least as wide as int");



// typedefs
// (define the STL integer types as part of the age namespace for less verbose code)

typedef std::uint8_t  uint8_t;
typedef std::uint16_t uint16_t;
typedef std::uint32_t uint32_t;
typedef std::uint64_t uint64_t;

typedef std::int8_t  int8_t;
typedef std::int16_t int16_t;
typedef std::int32_t int32_t;
typedef std::int64_t int64_t;

typedef std::size_t size_t;

template<size_t _size> using uint8_array = std::array<uint8_t, _size>;

typedef std::vector<uint8_t> uint8_vector;



// constant expressions

constexpr int int_max = std::numeric_limits<int>::max();
constexpr int16_t int16_t_max = std::numeric_limits<int16_t>::max();
constexpr int32_t int32_t_max = std::numeric_limits<int32_t>::max();

constexpr const char *project_name = "AGE";

//!
//! Convert the specified enum value to the associated value of the underlying type.
//!
template<typename E>
constexpr auto to_integral(E e) -> typename std::underlying_type<E>::type
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

} // namespace age



#endif // AGE_TYPES_HPP
