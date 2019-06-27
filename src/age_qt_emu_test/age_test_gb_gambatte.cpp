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

#include <algorithm> // std::count

#include <QChar>
#include <QDir>
#include <QFileInfo>
#include <QSharedPointer>

#include "age_test_gb.hpp"



namespace
{

//!
//! gambatte tests run for 15 frames
//! (see gambatte/test/testrunner.cpp)
//!
constexpr int gb_gambatte_test_frames = 15;

//!
//! important for gambatte "outaudio" tests:
//! these tests run until 15 * 35112 samples have been produced,
//! the last 35112 samples are then checked
//!
constexpr int gb_samples_per_frame = 35112;

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
            result.push_back(static_cast<age::uint8_t>(value));
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
    unsigned screen_width = static_cast<unsigned>(emulator.get_screen_width());

    // start with the first line
    // (the emulator screen buffer is filled upside down)
    unsigned line_offset = 0;
    unsigned tile_offset = 0;

    // the first pixel in the first line is always expected to be "white"
    // (see the tiles stored in tile_data)
    age::pixel white = screen[line_offset];

    // examine each pixel
    for (unsigned line = 0; line < 8; ++line)
    {
        unsigned pixel_offset = 0;
        for (age::uint8_t tile_index : expected_result)
        {
            const age::uint8_t *tile_ptr = &tile_data[tile_index * 8 * 8 + tile_offset];
            for (unsigned tile_pixel = 0; tile_pixel < 8; ++tile_pixel, ++pixel_offset)
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
        line_offset += screen_width;
        tile_offset += 8;
    }

    return true;
}



age::test_method gambatte_out_string_test(const age::uint8_vector &out_string, bool force_dmg)
{
    return [=](const age::uint8_vector &test_rom, const age::uint8_vector&)
    {
        // create emulator & run test
        age::gb_hardware hardware = force_dmg ? age::gb_hardware::dmg : age::gb_hardware::auto_detect;
        QSharedPointer<age::gb_emulator> emulator = QSharedPointer<age::gb_emulator>(new age::gb_emulator(test_rom, hardware));

        AGE_ASSERT(emulator->get_pcm_sampling_rate() == emulator->get_cycles_per_second() / 2);
        int test_cycles = gb_gambatte_test_frames * gb_samples_per_frame * 2;
        gb_emulate(*emulator, test_cycles);

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
            result.set(true);
        }
    }

    return result;
}



age::test_method gambatte_outaudio_test(bool expect_audio_output, bool force_dmg)
{
    return [=](const age::uint8_vector &test_rom, const age::uint8_vector&)
    {
        // create emulator & run test
        age::gb_hardware hardware = force_dmg ? age::gb_hardware::dmg : age::gb_hardware::auto_detect;
        QSharedPointer<age::gb_emulator> emulator = QSharedPointer<age::gb_emulator>(new age::gb_emulator(test_rom, hardware));

        // gambatte audio tests run for 15 * 35112 samples
        // (see gambatte/test/testrunner.cpp)
        AGE_ASSERT(emulator->get_pcm_sampling_rate() == emulator->get_cycles_per_second() / 2);

        for (int i = gb_gambatte_test_frames; i >= 0; --i)
        {
            gb_emulate(*emulator, gb_samples_per_frame * 2);
        }

        // evaluate test result by checking the first gb_samples_per_frame
        // pcm samples of the last test emulation iteration for equality
        // (similar to gambatte/test/testrunner.cpp)
        const age::pcm_vector &audio_buffer = emulator->get_audio_buffer();
        AGE_ASSERT(audio_buffer.size() >= gb_samples_per_frame);

        const age::pcm_sample first_sample = emulator->get_audio_buffer()[0];
        qint64 count = std::count(begin(audio_buffer),
                                  begin(audio_buffer) + gb_samples_per_frame,
                                  first_sample);

        // return an error message, if the test failed
        QString error;

        if (expect_audio_output && (count == gb_samples_per_frame))
        {
            error = "fail: expected audio output";
        }
        else if (!expect_audio_output && (count != gb_samples_per_frame))
        {
            error = QString("fail: found ")
                    .append(QString::number(count, 10))
                    .append(" non-silent audio samples");
        }

        return create_gb_test_result(*emulator, error);
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
        int millis = gb_gambatte_test_frames * 1000 / 59; // 59 fps
        return age::screenshot_test_png(for_dmg, false, millis);
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
