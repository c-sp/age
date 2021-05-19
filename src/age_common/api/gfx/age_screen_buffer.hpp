//
// Copyright 2020 Christoph Sprenger
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



namespace age
{

    class screen_buffer
    {
    public:
        screen_buffer(int16_t screen_width, int16_t screen_height);

        [[nodiscard]] int16_t  get_screen_width() const;
        [[nodiscard]] int16_t  get_screen_height() const;
        [[nodiscard]] unsigned get_current_frame_id() const;

        [[nodiscard]] const pixel_vector& get_front_buffer() const;

        pixel_vector& get_back_buffer();
        void          switch_buffers();

    private:
        const int16_t m_screen_width;
        const int16_t m_screen_height;

        pixel_vector m_front_buffer;
        pixel_vector m_back_buffer;
        unsigned     m_frame_id = 0; // unsigned for well defined wrap around behaviour
    };

} // namespace age



#endif // AGE_SCREEN_BUFFER_HPP
