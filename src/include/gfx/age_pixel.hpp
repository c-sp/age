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

    pixel(int32_t r, int32_t g, int32_t b)
    {
        AGE_ASSERT(r >= 0);
        AGE_ASSERT(g >= 0);
        AGE_ASSERT(b >= 0);
        AGE_ASSERT(r <= 255);
        AGE_ASSERT(g <= 255);
        AGE_ASSERT(b <= 255);

        m_rgba[0] = r & 0xFF;
        m_rgba[1] = g & 0xFF;
        m_rgba[2] = b & 0xFF;
        m_rgba[3] = 0xFF;
    }

    bool operator==(const pixel &other) const
    {
        return m_color == other.m_color;
    }

    bool operator!=(const pixel &other) const
    {
        return m_color != other.m_color;
    }

    // We use byte-level access to be independent of byte ordering
    // and 32 bit integer access to speed up operations like comparison.
    union
    {
        uint8_t m_rgba[4];
        uint32_t m_color;
    };
};

static_assert(sizeof(pixel) == 4, "expected pixel size of 4 bytes (RGBA)");

typedef std::vector<pixel> pixel_vector;



} // namespace age

#endif // AGE_PIXEL_HPP
