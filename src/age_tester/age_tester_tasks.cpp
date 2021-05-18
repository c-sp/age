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

#include "age_tester_tasks.hpp"

#include <csetjmp>
#include <fstream>
#include <iostream>
#include <png.h>



namespace
{
    //!
    //! Read any color_type into 8bit depth, RGBA format.
    //! See http://www.libpng.org/pub/png/libpng-manual.txt
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
    //! Read a png image file.
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
        // check png signature
        std::uint8_t sig[8];
        fread(sig, 1, 8, fp);
        if (!png_check_sig(sig, 8))
        {
            return {}; // bad png signature
        }

        // allocate pixel buffer before any png data structures
        // to prevent memory leaks as we have to manually free png data structures
        age::pixel_vector pixels(screen_width * screen_height, age::pixel());

        png_bytep row_pointers[screen_height];
        for (int i = 0; i < screen_height; i++)
        {
            row_pointers[i] = reinterpret_cast<unsigned char*>(&pixels[i * screen_width]);
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



    age::pixel_vector load_png(const std::filesystem::path& screenshot_png_path,
                               const age::gb_emulator&      emulator)
    {
        FILE*             file   = fopen(screenshot_png_path.string().c_str(), "rb");
        age::pixel_vector result = read_png_file(file, emulator.get_screen_width(), emulator.get_screen_height());
        fclose(file);
        return result;
    }

} // namespace



std::shared_ptr<age::uint8_vector> age::tester::load_rom_file(const std::filesystem::path& rom_path)
{
    std::ifstream rom_file(rom_path, std::ios::in | std::ios::binary);
    return std::make_shared<age::uint8_vector>((std::istreambuf_iterator<char>(rom_file)), std::istreambuf_iterator<char>());
}



age::tester::run_test_t age::tester::new_screenshot_test(const std::filesystem::path& screenshot_png_path,
                                                         const test_finished_t&       test_finished)
{
    return [=](age::gb_emulator& emulator) {
        int cycles_per_step = emulator.get_cycles_per_second() >> 8;

        while (!test_finished(emulator))
        {
            emulator.emulate(cycles_per_step);
        }

        // load png
        auto png_data = load_png(screenshot_png_path, emulator);

        // compare screen to screenshot
        auto screenshot = png_data.data();
        auto screen     = emulator.get_screen_front_buffer().data();

        for (int i = 0, max = emulator.get_screen_width() * emulator.get_screen_height(); i < max; ++i)
        {
            if (*screen != *screenshot)
            {
                // int x = i % emulator.get_screen_width();
                // int y = emulator.get_screen_height() - 1 - (i / emulator.get_screen_width());
                // AGE_LOG(screenshot_png_path << ": screen and screenshot differ at position ("
                //         << x << ',' << y << ')'
                //         << ": expected 0x" << std::hex << screenshot->m_color << ", found 0x" << screen->m_color);
                return false;
            }
            ++screen;
            ++screenshot;
        }

        return true;
    };
}
