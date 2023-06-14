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

// the following code requires libpng
#ifdef LIBPNG_FOUND

#include <age_types.hpp>
#include <gfx/age_png.hpp>

#include <png.h>

#include <csetjmp>



namespace
{
    class auto_closing_file
    {
        AGE_DISABLE_COPY(auto_closing_file);
        AGE_DISABLE_MOVE(auto_closing_file);

        FILE* m_file_ptr = nullptr;

    public:
        auto_closing_file(const std::string& file_path, const char* mode)
            : m_file_ptr(fopen(file_path.c_str(), mode))
        {
        }

        ~auto_closing_file()
        {
            if (m_file_ptr != nullptr)
            {
                auto state = fclose(m_file_ptr);
                if (state != 0) {
                    //! \todo log something
                }
                m_file_ptr = nullptr;
            }
        }

        FILE* file_ptr()
        {
            return m_file_ptr;
        }
    };



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

} // namespace



age::pixel_vector age::read_png_file(const std::string& file_path,
                                     int                image_width,
                                     int                image_height)
{
    if ((image_width < 1) || (image_height <= 1))
    {
        return {};
    }

    // open file for reading
    auto_closing_file file(file_path, "rb");
    auto*             file_ptr = file.file_ptr();
    if (!file_ptr)
    {
        return {};
    }

    // check png signature
    age::uint8_array<8> sig;
    auto                bytes_read = fread(sig.data(), 1, 8, file_ptr);
    if (bytes_read < 8)
    {
        return {}; // error or file too short
    }
    if (!png_check_sig(sig.data(), 8))
    {
        return {}; // bad png signature
    }

    // allocate pixel buffer before any png data structures
    // to prevent memory leaks as we have to manually free png data structures
    size_t            screen_width_u = image_width;
    age::pixel_vector pixels(screen_width_u * image_height, age::pixel());

    png_bytep row_pointers[image_height];
    for (int i = 0; i < image_height; i++)
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

    png_init_io(png_ptr, file_ptr);
    png_set_sig_bytes(png_ptr, 8); // we already read the signature
    png_read_info(png_ptr, info_ptr);

    // check image dimensions
    auto width  = png_get_image_width(png_ptr, info_ptr);
    auto height = png_get_image_height(png_ptr, info_ptr);

    if ((width != image_width) || (height != image_height))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return {};
    }

    // read image data
    configure_rgba(png_ptr, info_ptr);
    png_read_image(png_ptr, &row_pointers[0]);

    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    return pixels;
}



void age::write_png_file(const age::pixel_vector& data,
                         int                      image_width,
                         int                      image_height,
                         const std::string&       file_path)
{
    auto_closing_file file(file_path, "wb");
    auto*             file_ptr = file.file_ptr();
    if (!file_ptr)
    {
        return;
    }

    // We cannot be sure that png_write_image(png_structrp, png_bytepp) does not
    // modify image data, as the second parameter is not declared const.
    // We thus use an image data copy.
    age::pixel_vector data_copy(data);
    size_t            screen_width_u = image_width;
    png_bytep         row_pointers[image_height];
    for (int i = 0; i < image_height; i++)
    {
        row_pointers[i] = reinterpret_cast<unsigned char*>(&data_copy[i * screen_width_u]);
    }

    // create png data structures
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
    {
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_write_struct(&png_ptr, nullptr);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr))) // NOLINT(cert-err52-cpp)
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return;
    }

    png_init_io(png_ptr, file_ptr);

    // Output is 8bit depth, RGBA format.
    png_set_IHDR(
        png_ptr,
        info_ptr,
        image_width,
        image_height,
        8,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);

    png_write_image(png_ptr, &row_pointers[0]);
    png_write_end(png_ptr, nullptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);
}



#endif // LIBPNG_FOUND
