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

#ifndef AGE_SCREEN_BUFFER_HPP
#define AGE_SCREEN_BUFFER_HPP

//!
//! \file
//!

#include <array>

#include <age_types.hpp>
#include <gfx/age_pixel.hpp>



namespace age {



class screen_buffer
{
public:

    screen_buffer(int16_t screen_width, int16_t screen_height);

    uint8_t get_front_buffer_index() const;
    int16_t get_screen_width() const;
    int16_t get_screen_height() const;

    const pixel_vector& get_front_buffer() const;
    const pixel_vector& get_back_buffer() const;

    pixel_vector& get_back_buffer();
    pixel* get_first_scanline_pixel(int16_t scanline);
    void switch_buffers();

private:

    const int16_t m_screen_width;
    const int16_t m_screen_height;

    std::array<pixel_vector, 2> m_buffers;
    uint8_t m_current_front_buffer = 0;
};



} // namespace age

#endif // AGE_SCREEN_BUFFER_HPP
