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



#define AGE_DISABLE_COPY(Class)   \
private:                          \
    Class(const Class&) = delete; \
    Class& operator=(const Class&) = delete



// Use this to mark unused (but required) parameters,
// e.g. for callbacks.
//
// See also:
// https://stackoverflow.com/questions/1486904/how-do-i-best-silence-a-warning-about-unused-variables#comment51105057_1486931
#define AGE_UNUSED(arg) ((void) &(arg))



namespace age
{

    // C++ arithmetic operators do not accept types smaller than int
    // (see https://en.cppreference.com/w/cpp/language/implicit_conversion).
    // AGE relies on int being 32 bits or more.

    static_assert(sizeof(int) >= 4, "AGE requires int being at least 32 bits wide");
    static_assert(sizeof(unsigned) == sizeof(int), "AGE requires unsigned int being exactly as wide as int");
    static_assert(sizeof(std::size_t) >= sizeof(int), "AGE requires std::size_t to be at least as wide as int");



    // typedefs
    // (define the STL integer types as part of the age namespace for less verbose code)

    using uint8_t  = std::uint8_t;
    using uint16_t = std::uint16_t;
    using uint32_t = std::uint32_t;
    using uint64_t = std::uint64_t;

    using int8_t  = std::int8_t;
    using int16_t = std::int16_t;
    using int32_t = std::int32_t;
    using int64_t = std::int64_t;

    using size_t = std::size_t;

    template<size_t Size>
    using uint8_array = std::array<uint8_t, Size>;

    using uint8_vector = std::vector<uint8_t>;



    // constant expressions

    constexpr int     int_max     = std::numeric_limits<int>::max();
    constexpr int16_t int16_t_max = std::numeric_limits<int16_t>::max();
    constexpr int16_t int16_t_min = std::numeric_limits<int16_t>::min();
    constexpr int32_t int32_t_max = std::numeric_limits<int32_t>::max();

    //!
    //! Convert the specified enum value to the associated value of the underlying type.
    //! See also: https://stackoverflow.com/a/14589519
    //!
    //! Can be replaced by std::to_underlying with C++23.
    //!
    template<typename Enum>
    constexpr auto to_underlying(Enum e) -> typename std::underlying_type<Enum>::type
    {
        return static_cast<typename std::underlying_type<Enum>::type>(e);
    }

} // namespace age



#endif // AGE_TYPES_HPP
