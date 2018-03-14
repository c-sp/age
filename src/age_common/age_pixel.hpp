
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
        : pixel(0xFF000000 + (r << 16) + (g << 8) + b)
    {
        AGE_ASSERT(r <= 255);
        AGE_ASSERT(g <= 255);
        AGE_ASSERT(b <= 255);
    }

    pixel(uint32 x8r8g8b8)
        : m_x8r8g8b8(x8r8g8b8)
    {}

    bool operator==(const pixel &other) const
    {
        return m_x8r8g8b8 == other.m_x8r8g8b8;
    }

    bool operator!=(const pixel &other) const
    {
        return m_x8r8g8b8 != other.m_x8r8g8b8;
    }

    uint32 m_x8r8g8b8;
};

constexpr uint sizeof_pixel = sizeof(pixel);

typedef std::vector<pixel> pixel_vector;



} // namespace age

#endif // AGE_PIXEL_HPP
