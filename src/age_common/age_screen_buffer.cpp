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

#include <age_debug.hpp>

#include <gfx/age_screen_buffer.hpp>



age::screen_buffer::screen_buffer(uint screen_width, uint screen_height)
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



age::uint age::screen_buffer::get_front_buffer_index() const
{
    return m_current_front_buffer;
}

age::uint age::screen_buffer::get_screen_width() const
{
    return m_screen_width;
}

age::uint age::screen_buffer::get_screen_height() const
{
    return m_screen_height;
}

const age::pixel_vector& age::screen_buffer::get_front_buffer() const
{
    return m_buffers[m_current_front_buffer];
}

const age::pixel_vector& age::screen_buffer::get_back_buffer() const
{
    return m_buffers[1 - m_current_front_buffer];
}



age::pixel_vector& age::screen_buffer::get_back_buffer()
{
    return m_buffers[1 - m_current_front_buffer];
}

age::pixel* age::screen_buffer::get_first_scanline_pixel(uint scanline)
{
    AGE_ASSERT(scanline < m_screen_height);
    pixel *result = get_back_buffer().data() + scanline * m_screen_width;
    return result;
}

void age::screen_buffer::switch_buffers()
{
    m_current_front_buffer = 1 - m_current_front_buffer;
}
