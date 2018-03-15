//
// Copyright (c) 2010-2017 Christoph Sprenger
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
    // frames are currently stored upside-down
    pixel *result = get_back_buffer().data() + (m_screen_height - 1 - scanline) * m_screen_width;
    return result;
}

void age::screen_buffer::switch_buffers()
{
    m_current_front_buffer = 1 - m_current_front_buffer;
}
