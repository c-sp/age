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
