//
// Copyright (c) 2010-2018 Christoph Sprenger
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

#include <memory> // std::shared_ptr

#include <QChar>
#include <QDir>
#include <QFileInfo>

#include "age_test_gb.hpp"



namespace age {

//!
//! gambatte tests run for 15 frames
//! (see gambatte/test/testrunner.cpp)
//!
constexpr uint gb_gambatte_test_frames = 15;
constexpr uint gb_frames_per_second = 59;

uint gb_cycles_per_frame(const gb_emulator &emulator)
{
    return emulator.get_cycles_per_second() / gb_frames_per_second;
}

uint test_cycles(const gb_emulator &emulator)
{
    uint cycles_per_frame = gb_cycles_per_frame(emulator);
    return cycles_per_frame * gb_gambatte_test_frames;
}

}





//---------------------------------------------------------
//
//   gambatte "out string" tests
//
//---------------------------------------------------------

// the following tile_data array contents were copied from gambatte code
// (gambatte/test/testrunner.cpp)
#define _ 0
#define O 1
constexpr age::uint8_array<16 * 8 * 8> tile_data =
{{
     _,_,_,_,_,_,_,_,
     _,O,O,O,O,O,O,O,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,O,O,O,O,O,O,O,

     _,_,_,_,_,_,_,_,
     _,_,_,_,O,_,_,_,
     _,_,_,_,O,_,_,_,
     _,_,_,_,O,_,_,_,
     _,_,_,_,O,_,_,_,
     _,_,_,_,O,_,_,_,
     _,_,_,_,O,_,_,_,
     _,_,_,_,O,_,_,_,

     _,_,_,_,_,_,_,_,
     _,O,O,O,O,O,O,O,
     _,_,_,_,_,_,_,O,
     _,_,_,_,_,_,_,O,
     _,O,O,O,O,O,O,O,
     _,O,_,_,_,_,_,_,
     _,O,_,_,_,_,_,_,
     _,O,O,O,O,O,O,O,

     _,_,_,_,_,_,_,_,
     _,O,O,O,O,O,O,O,
     _,_,_,_,_,_,_,O,
     _,_,_,_,_,_,_,O,
     _,_,O,O,O,O,O,O,
     _,_,_,_,_,_,_,O,
     _,_,_,_,_,_,_,O,
     _,O,O,O,O,O,O,O,

     _,_,_,_,_,_,_,_,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,O,O,O,O,O,O,O,
     _,_,_,_,_,_,_,O,
     _,_,_,_,_,_,_,O,
     _,_,_,_,_,_,_,O,

     _,_,_,_,_,_,_,_,
     _,O,O,O,O,O,O,O,
     _,O,_,_,_,_,_,_,
     _,O,_,_,_,_,_,_,
     _,O,O,O,O,O,O,_,
     _,_,_,_,_,_,_,O,
     _,_,_,_,_,_,_,O,
     _,O,O,O,O,O,O,_,

     _,_,_,_,_,_,_,_,
     _,O,O,O,O,O,O,O,
     _,O,_,_,_,_,_,_,
     _,O,_,_,_,_,_,_,
     _,O,O,O,O,O,O,O,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,O,O,O,O,O,O,O,

     _,_,_,_,_,_,_,_,
     _,O,O,O,O,O,O,O,
     _,_,_,_,_,_,_,O,
     _,_,_,_,_,_,O,_,
     _,_,_,_,_,O,_,_,
     _,_,_,_,O,_,_,_,
     _,_,_,O,_,_,_,_,
     _,_,_,O,_,_,_,_,

     _,_,_,_,_,_,_,_,
     _,_,O,O,O,O,O,_,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,_,O,O,O,O,O,_,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,_,O,O,O,O,O,_,

     _,_,_,_,_,_,_,_,
     _,O,O,O,O,O,O,O,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,O,O,O,O,O,O,O,
     _,_,_,_,_,_,_,O,
     _,_,_,_,_,_,_,O,
     _,O,O,O,O,O,O,O,

     _,_,_,_,_,_,_,_,
     _,_,_,_,O,_,_,_,
     _,_,O,_,_,_,O,_,
     _,O,_,_,_,_,_,O,
     _,O,O,O,O,O,O,O,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,

     _,_,_,_,_,_,_,_,
     _,O,O,O,O,O,O,_,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,O,O,O,O,O,O,_,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,O,O,O,O,O,O,_,

     _,_,_,_,_,_,_,_,
     _,_,O,O,O,O,O,_,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,_,
     _,O,_,_,_,_,_,_,
     _,O,_,_,_,_,_,_,
     _,O,_,_,_,_,_,O,
     _,_,O,O,O,O,O,_,

     _,_,_,_,_,_,_,_,
     _,O,O,O,O,O,O,_,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,O,_,_,_,_,_,O,
     _,O,O,O,O,O,O,_,

     _,_,_,_,_,_,_,_,
     _,O,O,O,O,O,O,O,
     _,O,_,_,_,_,_,_,
     _,O,_,_,_,_,_,_,
     _,O,O,O,O,O,O,O,
     _,O,_,_,_,_,_,_,
     _,O,_,_,_,_,_,_,
     _,O,O,O,O,O,O,O,

     _,_,_,_,_,_,_,_,
     _,O,O,O,O,O,O,O,
     _,O,_,_,_,_,_,_,
     _,O,_,_,_,_,_,_,
     _,O,O,O,O,O,O,O,
     _,O,_,_,_,_,_,_,
     _,O,_,_,_,_,_,_,
     _,O,_,_,_,_,_,_
 }};
#undef O
#undef _



age::uint8_vector parse_out_string(const QString &string, const QString &prefix)
{
    age::uint8_vector result;

    int index = string.indexOf(prefix, 0, Qt::CaseInsensitive);
    if (index >= 0)
    {
        // start with the first character after the prefix
        index += prefix.length();
        for (; index < string.length(); ++index)
        {
            // try to convert a character to integer
            bool converted = false;
            int value = string.mid(index, 1).toInt(&converted, 16);

            // break the loop, the conversion failed
            if (!converted)
            {
                // clear the result, if we found an invalid character
                QChar c = string.at(index);
                if ((c != '_') && (c != '.'))
                {
                    result.clear();
                }
                break;
            }

            // store the converted value
            AGE_ASSERT((value >= 0) && (value < 0x10));
            result.push_back(static_cast<age::uint8>(value));
        }
    }

    // we cannot handle more than 20 characters (one gameboy scanline)
    if (result.size() > 20)
    {
        result.clear();
    }
    return result;
}



bool evaluate_out_string_result(const age::gb_emulator &emulator, const age::uint8_vector &expected_result)
{
    const age::pixel_vector &screen = emulator.get_screen_front_buffer();

    // start with the first line
    // (the emulator screen buffer is filled upside down)
    age::uint line_offset = 0;
    age::uint tile_offset = 0;

    // the first pixel in the first line is always expected to be "white"
    // (see the tiles stored in tile_data)
    age::pixel white = screen[line_offset];

    // examine each pixel
    for (age::uint line = 0; line < 8; ++line)
    {
        age::uint pixel_offset = 0;
        for (age::uint8 tile_index : expected_result)
        {
            const age::uint8 *tile_ptr = &tile_data[tile_index * 8 * 8 + tile_offset];
            for (age::uint tile_pixel = 0; tile_pixel < 8; ++tile_pixel, ++pixel_offset)
            {
                bool found_white = screen[line_offset + pixel_offset] == white;
                bool expect_white = tile_ptr[tile_pixel] == 0;

                if (found_white != expect_white)
                {
                    return false;
                }
            }
        }

        // next line
        line_offset += emulator.get_screen_width();
        tile_offset += 8;
    }

    return true;
}



age::test_method gambatte_out_string_test(const age::uint8_vector &out_string, bool force_dmg)
{
    return [=](const age::uint8_vector &test_rom, const age::uint8_vector&) {

        // create emulator & run test
        std::shared_ptr<age::gb_emulator> emulator = std::make_shared<age::gb_emulator>(test_rom, force_dmg);
        emulator->emulate(test_cycles(*emulator));

        // evaluate test result
        bool pass = evaluate_out_string_result(*emulator, out_string);

        // return an error message, if the test failed
        return create_gb_test_result(*emulator, pass ? "" : "failed");
    };
}





//---------------------------------------------------------
//
//   gambatte audio tests
//
//---------------------------------------------------------

age::optional<bool> parse_outaudio_flag(const QString &string, const QString &prefix)
{
    age::optional<bool> result;

    int index = string.indexOf(prefix, 0, Qt::CaseInsensitive);
    if (index >= 0)
    {
        // check the first character after the prefix
        index += prefix.length();

        // 0 -> no audio output expected
        if (string.at(index) == '0')
        {
            result.set(false);
        }

        // 1 -> audio output expected
        else if (string.at(index) == '1')
        {
            result.set(false);
        }
    }

    return result;
}



age::test_method gambatte_outaudio_test(bool expect_audio_output, bool force_dmg)
{
    return [=](const age::uint8_vector &test_rom, const age::uint8_vector&) {

        // create emulator & run test
        std::shared_ptr<age::gb_emulator> emulator = std::make_shared<age::gb_emulator>(test_rom, force_dmg);

        // gambatte tests run for 15 frames
        // (see gambatte/test/testrunner.cpp)
        uint cycles_per_frame = gb_cycles_per_frame(*emulator);

        emulator->emulate(test_cycles(*emulator));

        // evaluate test result by checking the first cycles_per_frame
        // pcm samples for equality
        // (similar to gambatte/test/testrunner.cpp)
        bool all_equal = true;
        const age::pcm_sample first_sample = emulator->get_audio_buffer()[0];

        for (age::uint i = 1; i < cycles_per_frame; ++i)
        {
            if (emulator->get_audio_buffer()[i] != first_sample)
            {
                all_equal = false;
                break;
            }
        }

        // return an error message, if the test failed
        bool pass = all_equal == expect_audio_output;
        return create_gb_test_result(*emulator, pass ? "" : "failed");
    };
}





//---------------------------------------------------------
//
//   gambatte screenshot tests
//
//---------------------------------------------------------

QString find_screenshot_file(const QString &test_file, const QString &suffix)
{
    QFileInfo test_file_info(test_file);
    QDir test_file_dir = test_file_info.absoluteDir();

    QString screenshot_file_name = test_file_info.completeBaseName() + suffix;
    bool screenshot_exists = test_file_dir.exists(screenshot_file_name);

    return screenshot_exists ? test_file_dir.absoluteFilePath(screenshot_file_name) : "";
}





//---------------------------------------------------------
//
//   gambatte test method creation
//
//---------------------------------------------------------

age::test_method gambatte_test(const QString &test_file_name, QString &result_file_name, bool for_dmg)
{
    // determine the type of the test by looking for different types of results
    // (based on gambatte/test/testrunner.cpp)

    // check if the test result is a string on the gameboy screen
    age::optional<bool> outaudio_flag = parse_outaudio_flag(test_file_name, for_dmg ? "dmg08_outaudio" : "cgb04c_outaudio");
    if (!outaudio_flag.is_set())
    {
        outaudio_flag = parse_outaudio_flag(test_file_name, "dmg08_cgb04c_outaudio");
    }
    if (outaudio_flag.is_set())
    {
        return gambatte_outaudio_test(outaudio_flag.get(true), for_dmg);
    }

    // check if the test result is a string on the gameboy screen
    age::uint8_vector out_string = parse_out_string(test_file_name, for_dmg ? "dmg08_out" : "cgb04c_out");
    if (out_string.empty())
    {
        out_string = parse_out_string(test_file_name, "dmg08_cgb04c_out");
    }
    if (!out_string.empty())
    {
        return gambatte_out_string_test(out_string, for_dmg);
    }

    // check if the test result is a screenshot
    result_file_name = find_screenshot_file(test_file_name, for_dmg ? "_dmg08.png" : "_cgb04c.png");
    if (result_file_name.isEmpty())
    {
        result_file_name = find_screenshot_file(test_file_name, "_dmg08_cgb04c.png");
    }
    if (!result_file_name.isEmpty())
    {
        return age::screenshot_test_png(for_dmg, false, age::gb_gambatte_test_frames * 1000 / age::gb_frames_per_second);
    }

    // return an empty method as we don't know the gambatte test type
    return age::test_method{};
}



age::test_method age::gambatte_dmg_test(const QString &test_file_name, QString &result_file_name)
{
    return gambatte_test(test_file_name, result_file_name, true);
}

age::test_method age::gambatte_cgb_test(const QString &test_file_name, QString &result_file_name)
{
    return gambatte_test(test_file_name, result_file_name, false);
}
