//
// Copyright 2022 Christoph Sprenger
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

#ifndef AGE_PNG_HPP
#define AGE_PNG_HPP

//!
//! \file
//!

// the following code requires libpng
#ifdef LIBPNG_FOUND

#include <gfx/age_pixel.hpp>

#include <cstdio>
#include <string>



namespace age
{
    //!
    //! Read a png image file as 32 bit RGBA.
    //! Based on
    //! <ul>
    //!     <li> http://www.libpng.org/pub/png/libpng-manual.txt
    //!     <li> https://gist.github.com/niw/5963798
    //! </ul>
    //!
    pixel_vector read_png_file(const std::string& file_path,
                               int                image_width,
                               int                image_height);

    //!
    //! Write a png image file as 32 bit RGBA.
    //! Based on https://gist.github.com/niw/5963798
    //!
    void write_png_file(const pixel_vector& data,
                        int                 image_width,
                        int                 image_height,
                        const std::string&  file_path);

} // namespace age



#endif // LIBPNG_FOUND

#endif // AGE_PNG_HPP
