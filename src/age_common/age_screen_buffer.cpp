//
// © 2017 Christoph Sprenger <https://github.com/c-sp>
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

#include <gfx/age_screen_buffer.hpp>

#include <cassert>



age::screen_buffer::screen_buffer(int16_t screen_width, int16_t screen_height)
    : m_screen_width(screen_width),
      m_screen_height(screen_height)
{
    assert(m_screen_width > 0);
    assert(m_screen_height > 0);

    auto buffer_size = static_cast<unsigned>(m_screen_width * m_screen_height);

    m_front_buffer = pixel_vector(buffer_size);
    m_back_buffer  = pixel_vector(buffer_size);
}


age::int16_t age::screen_buffer::get_screen_width() const
{
    return m_screen_width;
}

age::int16_t age::screen_buffer::get_screen_height() const
{
    return m_screen_height;
}

unsigned age::screen_buffer::get_current_frame_id() const
{
    return m_frame_id;
}

const age::pixel_vector& age::screen_buffer::get_front_buffer() const
{
    return m_front_buffer;
}


age::pixel_vector& age::screen_buffer::get_back_buffer()
{
    return m_back_buffer;
}

std::span<age::pixel> age::screen_buffer::get_back_buffer_line(int line)
{
    auto back_buffer_offset = line * m_screen_width;
    return {get_back_buffer().begin() + back_buffer_offset, static_cast<unsigned>(m_screen_width)};
}


void age::screen_buffer::switch_buffers()
{
    // https://stackoverflow.com/a/28130696
    using std::swap;
    swap(m_front_buffer, m_back_buffer);
    ++m_frame_id; // may wrap around but that's okay
}
