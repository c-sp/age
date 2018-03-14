
#include <age_debug.hpp>

#include "age_graphics.hpp"



age::video_buffer_handler::video_buffer_handler(uint screen_width, uint screen_height)
    : m_screen_width(screen_width),
      m_screen_height(screen_height)
{
    AGE_ASSERT(m_screen_width <= uint_max / m_screen_height);
    AGE_ASSERT(m_screen_height <= uint_max / m_screen_width);
    AGE_ASSERT(m_screen_width > 0);
    AGE_ASSERT(m_screen_height > 0);

    m_buffers[0] = pixel_vector(m_screen_width * m_screen_height);
    m_buffers[1] = pixel_vector(m_screen_width * m_screen_height);
}



age::uint age::video_buffer_handler::get_front_buffer_index() const
{
    return m_current_front_buffer;
}

age::uint age::video_buffer_handler::get_screen_width() const
{
    return m_screen_width;
}

age::uint age::video_buffer_handler::get_screen_height() const
{
    return m_screen_height;
}

const age::pixel_vector& age::video_buffer_handler::get_front_buffer() const
{
    return m_buffers[m_current_front_buffer];
}

const age::pixel_vector& age::video_buffer_handler::get_back_buffer() const
{
    return m_buffers[1 - m_current_front_buffer];
}



age::pixel_vector& age::video_buffer_handler::get_back_buffer()
{
    return m_buffers[1 - m_current_front_buffer];
}

age::pixel* age::video_buffer_handler::get_first_scanline_pixel(uint scanline)
{
    AGE_ASSERT(scanline < m_screen_height);
    // frames are currently stored upside-down
    pixel *result = get_back_buffer().data() + (m_screen_height - 1 - scanline) * m_screen_width;
    return result;
}

void age::video_buffer_handler::switch_buffers()
{
    m_current_front_buffer = 1 - m_current_front_buffer;
}
