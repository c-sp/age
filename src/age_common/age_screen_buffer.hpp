
#ifndef AGE_SCREEN_BUFFER_HPP
#define AGE_SCREEN_BUFFER_HPP

//!
//! \file
//!

#include <array>

#include <age_pixel.hpp>
#include <age_types.hpp>



namespace age {



class screen_buffer
{
public:

    screen_buffer(uint screen_width, uint screen_height);

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

#endif // AGE_SCREEN_BUFFER_HPP
