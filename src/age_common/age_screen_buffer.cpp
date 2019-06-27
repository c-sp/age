//
// Copyright 2019 Christoph Sprenger
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



age::screen_buffer::screen_buffer(int16_t screen_width, int16_t screen_height)
    : m_screen_width(screen_width),
      m_screen_height(screen_height)
{
    AGE_ASSERT(m_screen_width > 0);
    AGE_ASSERT(m_screen_height > 0);

    // we rely on integral promotion to >=32 bits when multipling width & height
    auto buffer_size = static_cast<unsigned>(m_screen_width * m_screen_height);

    m_buffers[0] = pixel_vector(buffer_size);
    m_buffers[1] = pixel_vector(buffer_size);
}



age::uint8_t age::screen_buffer::get_front_buffer_index() const
{
    return m_current_front_buffer;
}

age::int16_t age::screen_buffer::get_screen_width() const
{
    return m_screen_width;
}

age::int16_t age::screen_buffer::get_screen_height() const
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

age::pixel* age::screen_buffer::get_first_scanline_pixel(int16_t scanline)
{
    AGE_ASSERT(scanline < m_screen_height);
    pixel *result = get_back_buffer().data() + scanline * m_screen_width;
    return result;
}

void age::screen_buffer::switch_buffers()
{
    m_current_front_buffer = 1 - m_current_front_buffer;
}
