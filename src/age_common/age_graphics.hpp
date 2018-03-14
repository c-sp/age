
#ifndef AGE_GRAPHICS_HPP
#define AGE_GRAPHICS_HPP

//!
//! \file
//!

#include <array>
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

typedef std::vector<pixel> pixel_vector;



class video_buffer_handler
{
public:

    video_buffer_handler(uint screen_width, uint screen_height);

    uint get_front_buffer_index() const;
    uint get_screen_width() const;
    uint get_screen_height() const;

    const pixel_vector& get_front_buffer() const;
    const pixel_vector& get_back_buffer() const;

    pixel_vector& get_back_buffer();
    pixel* get_first_scanline_pixel(uint scanline);
    void switch_buffers();

private:

    const uint m_screen_width;
    const uint m_screen_height;

    std::array<pixel_vector, 2> m_buffers;
    uint m_current_front_buffer = 0;
};



} // namespace age



#endif // AGE_GRAPHICS_HPP
