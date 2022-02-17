//
// Copyright 2021 Christoph Sprenger
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

#include <age_types.hpp>
#include <gfx/age_pixel.hpp>

#include <csetjmp>
#include <cstdio>
#include <png.h>



namespace age
{
    //!
    //! Configure libpng data structures for reading any
    //! color_type as 32 bit RGBA
    //! (see http://www.libpng.org/pub/png/libpng-manual.txt).
    //!
    void configure_rgba(png_structp png_ptr, png_infop info_ptr)
    {
        auto color_type = png_get_color_type(png_ptr, info_ptr);
        auto bit_depth  = png_get_bit_depth(png_ptr, info_ptr);

        if (bit_depth == 16)
        {
            png_set_strip_16(png_ptr);
        }

        if (color_type == PNG_COLOR_TYPE_PALETTE)
        {
            png_set_palette_to_rgb(png_ptr);
        }

        // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        {
            png_set_expand_gray_1_2_4_to_8(png_ptr);
        }

        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        {
            png_set_tRNS_to_alpha(png_ptr);
        }

        // These color_type don't have an alpha channel then fill it with 0xff.
        if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
        {
            png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
        }

        if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        {
            png_set_gray_to_rgb(png_ptr);
        }

        png_read_update_info(png_ptr, info_ptr);
    }



    //!
    //! Read a png image file as 32 bit RGBA.
    //! Based on
    //! <ul>
    //!     <li> http://www.libpng.org/pub/png/libpng-manual.txt
    //!     <li> https://gist.github.com/niw/5963798
    //! </ul>
    //!
    age::pixel_vector read_png_file(FILE* fp,
                                    int   screen_width,
                                    int   screen_height)
    {
        if ((screen_width < 1) || (screen_height <= 1))
        {
            return {};
        }

        // check png signature
        uint8_array<8> sig;
        fread(sig.data(), 1, 8, fp);
        if (!png_check_sig(sig.data(), 8))
        {
            return {}; // bad png signature
        }

        // allocate pixel buffer before any png data structures
        // to prevent memory leaks as we have to manually free png data structures
        size_t screen_width_u = screen_width;
        age::pixel_vector pixels(screen_width_u * screen_height, age::pixel());

        png_bytep row_pointers[screen_height];
        for (int i = 0; i < screen_height; i++)
        {
            row_pointers[i] = reinterpret_cast<unsigned char*>(&pixels[i * screen_width_u]);
        }

        // create png data structures
        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png_ptr)
        {
            return {}; // out of memory
        }
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
        {
            png_destroy_read_struct(&png_ptr, nullptr, nullptr);
            return {}; // out of memory
        }

        if (setjmp(png_jmpbuf(png_ptr))) // NOLINT(cert-err52-cpp)
        {
            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
            return {};
        }

        png_init_io(png_ptr, fp);
        png_set_sig_bytes(png_ptr, 8); // we already read the signature
        png_read_info(png_ptr, info_ptr);

        // check image dimensions
        auto width  = png_get_image_width(png_ptr, info_ptr);
        auto height = png_get_image_height(png_ptr, info_ptr);

        if ((width != screen_width) || (height != screen_height))
        {
            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
            return {};
        }

        // read image data
        configure_rgba(png_ptr, info_ptr);
        png_read_image(png_ptr, row_pointers);

        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return pixels;
    }

} // namespace age



#endif // LIBPNG_FOUND

#endif // AGE_PNG_HPP
