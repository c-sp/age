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

#ifndef AGE_PIXEL_HPP
#define AGE_PIXEL_HPP

//!
//! \file
//!

#include <vector>

#include <age_debug.hpp>
#include <age_types.hpp>



namespace age {



struct pixel
{
    pixel()
        : pixel(0, 0, 0)
    {}

    pixel(uint r, uint g, uint b)
        : pixel(0xFF000000 + (b << 16) + (g << 8) + r)
    {
        AGE_ASSERT(r <= 255);
        AGE_ASSERT(g <= 255);
        AGE_ASSERT(b <= 255);
    }

    pixel(uint32 x8r8g8b8)
        : m_a8b8g8r8(x8r8g8b8)
    {}

    bool operator==(const pixel &other) const
    {
        return m_a8b8g8r8 == other.m_a8b8g8r8;
    }

    bool operator!=(const pixel &other) const
    {
        return m_a8b8g8r8 != other.m_a8b8g8r8;
    }

    uint32 m_a8b8g8r8;
};

constexpr uint sizeof_pixel = sizeof(pixel);

typedef std::vector<pixel> pixel_vector;



} // namespace age

#endif // AGE_PIXEL_HPP
