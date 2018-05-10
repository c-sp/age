//
// Copyright (c) 2010-2018 Christoph Sprenger
//
// This file is part of AGE ("Another Gameboy Emulator").
// <https://gitlab.com/csprenger/AGE>
//
// AGE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// AGE is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with AGE.  If not, see <http://www.gnu.org/licenses/>.
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
