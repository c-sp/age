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

#ifndef AGE_PIXEL_HPP
#define AGE_PIXEL_HPP

//!
//! \file
//!

#include <cstring> // memcpy
#include <vector>

#include <age_debug.hpp>
#include <age_types.hpp>



namespace age
{

    struct pixel
    {
        pixel()
            : pixel(0, 0, 0)
        {}

        explicit pixel(unsigned rgb)
            : pixel(static_cast<uint8_t>(rgb >> 16),
                    static_cast<uint8_t>(rgb >> 8),
                    static_cast<uint8_t>(rgb))
        {}

        pixel(int r, int g, int b)
            : pixel(r, g, b, 0xFF)
        {}

        pixel(int r, int g, int b, int a)
            : m_r(static_cast<uint8_t>(r)),
              m_g(static_cast<uint8_t>(g)),
              m_b(static_cast<uint8_t>(b)),
              m_a(static_cast<uint8_t>(a))
        {
            AGE_ASSERT(r >= 0)
            AGE_ASSERT(g >= 0)
            AGE_ASSERT(b >= 0)
            AGE_ASSERT(a >= 0)

            AGE_ASSERT(r <= 255)
            AGE_ASSERT(g <= 255)
            AGE_ASSERT(b <= 255)
            AGE_ASSERT(a <= 255)
        }


        [[nodiscard]] uint32_t get_32bits() const
        {
            uint32_t color = 0;
            memcpy(&color, &m_r, sizeof(uint32_t));
            return color;
        }

        void set_32bits(uint32_t color)
        {
            memcpy(&m_r, &color, sizeof(pixel));
        }


        bool operator==(const pixel& other) const
        {
            return this->get_32bits() == other.get_32bits();
        }

        bool operator!=(const pixel& other) const
        {
            return !(*this == other);
        }

        uint8_t m_r;
        uint8_t m_g;
        uint8_t m_b;
        uint8_t m_a;
    };

    static_assert(sizeof(pixel) == 4, "expected pixel size of 4 bytes (RGBA)");

    using pixel_vector = std::vector<pixel>;

} // namespace age



#endif // AGE_PIXEL_HPP
