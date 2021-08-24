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

#include <algorithm>
#include <optional>



namespace
{
    constexpr age::uint8_t _ = 0;
    constexpr age::uint8_t O = 1;

    // tile_data array contents copied from gambatte code
    // (gambatte/test/testrunner.cpp)
    constexpr age::uint8_array<16 * 8 * 8> tile_data
        = {{_, _, _, _, _, _, _, _,
            _, O, O, O, O, O, O, O,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, O, O, O, O, O, O, O,

            _, _, _, _, _, _, _, _,
            _, _, _, _, O, _, _, _,
            _, _, _, _, O, _, _, _,
            _, _, _, _, O, _, _, _,
            _, _, _, _, O, _, _, _,
            _, _, _, _, O, _, _, _,
            _, _, _, _, O, _, _, _,
            _, _, _, _, O, _, _, _,

            _, _, _, _, _, _, _, _,
            _, O, O, O, O, O, O, O,
            _, _, _, _, _, _, _, O,
            _, _, _, _, _, _, _, O,
            _, O, O, O, O, O, O, O,
            _, O, _, _, _, _, _, _,
            _, O, _, _, _, _, _, _,
            _, O, O, O, O, O, O, O,

            _, _, _, _, _, _, _, _,
            _, O, O, O, O, O, O, O,
            _, _, _, _, _, _, _, O,
            _, _, _, _, _, _, _, O,
            _, _, O, O, O, O, O, O,
            _, _, _, _, _, _, _, O,
            _, _, _, _, _, _, _, O,
            _, O, O, O, O, O, O, O,

            _, _, _, _, _, _, _, _,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, O, O, O, O, O, O, O,
            _, _, _, _, _, _, _, O,
            _, _, _, _, _, _, _, O,
            _, _, _, _, _, _, _, O,

            _, _, _, _, _, _, _, _,
            _, O, O, O, O, O, O, O,
            _, O, _, _, _, _, _, _,
            _, O, _, _, _, _, _, _,
            _, O, O, O, O, O, O, _,
            _, _, _, _, _, _, _, O,
            _, _, _, _, _, _, _, O,
            _, O, O, O, O, O, O, _,

            _, _, _, _, _, _, _, _,
            _, O, O, O, O, O, O, O,
            _, O, _, _, _, _, _, _,
            _, O, _, _, _, _, _, _,
            _, O, O, O, O, O, O, O,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, O, O, O, O, O, O, O,

            _, _, _, _, _, _, _, _,
            _, O, O, O, O, O, O, O,
            _, _, _, _, _, _, _, O,
            _, _, _, _, _, _, O, _,
            _, _, _, _, _, O, _, _,
            _, _, _, _, O, _, _, _,
            _, _, _, O, _, _, _, _,
            _, _, _, O, _, _, _, _,

            _, _, _, _, _, _, _, _,
            _, _, O, O, O, O, O, _,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, _, O, O, O, O, O, _,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, _, O, O, O, O, O, _,

            _, _, _, _, _, _, _, _,
            _, O, O, O, O, O, O, O,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, O, O, O, O, O, O, O,
            _, _, _, _, _, _, _, O,
            _, _, _, _, _, _, _, O,
            _, O, O, O, O, O, O, O,

            _, _, _, _, _, _, _, _,
            _, _, _, _, O, _, _, _,
            _, _, O, _, _, _, O, _,
            _, O, _, _, _, _, _, O,
            _, O, O, O, O, O, O, O,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,

            _, _, _, _, _, _, _, _,
            _, O, O, O, O, O, O, _,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, O, O, O, O, O, O, _,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, O, O, O, O, O, O, _,

            _, _, _, _, _, _, _, _,
            _, _, O, O, O, O, O, _,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, _,
            _, O, _, _, _, _, _, _,
            _, O, _, _, _, _, _, _,
            _, O, _, _, _, _, _, O,
            _, _, O, O, O, O, O, _,

            _, _, _, _, _, _, _, _,
            _, O, O, O, O, O, O, _,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, O, _, _, _, _, _, O,
            _, O, O, O, O, O, O, _,

            _, _, _, _, _, _, _, _,
            _, O, O, O, O, O, O, O,
            _, O, _, _, _, _, _, _,
            _, O, _, _, _, _, _, _,
            _, O, O, O, O, O, O, O,
            _, O, _, _, _, _, _, _,
            _, O, _, _, _, _, _, _,
            _, O, O, O, O, O, O, O,

            _, _, _, _, _, _, _, _,
            _, O, O, O, O, O, O, O,
            _, O, _, _, _, _, _, _,
            _, O, _, _, _, _, _, _,
            _, O, O, O, O, O, O, O,
            _, O, _, _, _, _, _, _,
            _, O, _, _, _, _, _, _,
            _, O, _, _, _, _, _, _}};

    //!
    //! gambatte tests run for 15 frames
    //! (see gambatte/test/testrunner.cpp)
    //!
    constexpr int test_frames = 15;

    //!
    //! gambatte "outaudio" tests run for 15 frames,
    //! the last 35112 samples are then checked
    //! (see gambatte/test/testrunner.cpp)
    //!
    constexpr int samples_per_frame = 35112;

    bool is_test_finished(const age::gb_emulator& emulator)
    {
        return emulator.get_emulated_cycles() >= test_frames * samples_per_frame * 2;
    }

    void run_test(age::gb_emulator& emulator)
    {
        AGE_ASSERT(emulator.get_pcm_sampling_rate() == emulator.get_cycles_per_second() / 2)
        for (int i = test_frames; i >= 0; --i)
        {
            emulator.emulate(samples_per_frame * 2);
        }
    }



    age::uint8_t to_hex(char c)
    {
        return ((c >= 'a') && (c <= 'f'))   ? (c - 'a' + 10)
               : ((c >= 'A') && (c <= 'F')) ? (c - 'A' + 10)
               : ((c >= '0') && (c <= '9')) ? (c - '0')
                                            : 0xFF;
    }

    age::uint8_vector parse_hex_out(const std::string&              str,
                                    const std::vector<std::string>& prefixes)
    {
        for (const auto& prefix : prefixes)
        {
            auto pos = str.find(prefix);
            // prefix not found => continue with next prefix
            if ((pos == std::string::npos) || (pos + prefix.length() >= str.length()))
            {
                continue;
            }
            // prefix found => parse value
            age::uint8_vector result;

            for (auto& c : str.substr(pos + prefix.length()))
            {
                // end-of-value reached
                if ((c == '.') || (c == '_'))
                {
                    break;
                }
                // convert hex character
                age::uint8_t byte = to_hex(c);
                // invalid character found?
                if (byte >= 0x10)
                {
                    return {};
                }
                // store valid byte
                result.emplace_back(byte);
            }

            // return parsed value
            return result;
        }

        // found nothing
        return {};
    }

    age::tester::run_test_t new_hex_out_test(const age::uint8_vector& expected_result)
    {
        return [=](age::gb_emulator& emulator) {
            run_test(emulator);

            const auto& screen       = emulator.get_screen_front_buffer();
            auto        screen_width = emulator.get_screen_width();

            // start with the first line
            // (the emulator screen buffer is filled upside down)
            int line_offset = 0;
            int tile_offset = 0;

            // the first pixel in the first line is always expected to be of the background color
            // (see tiles stored in tile_data)
            age::pixel background = screen[line_offset];

            // examine each pixel
            for (int line = 0; line < 8; ++line)
            {
                int pixel_offset = 0;
                for (size_t exp_index = 0; exp_index < 20; ++exp_index)
                {
                    // pad the expected result with zeroes to fail on e.g. result 0x1F when expecting 0x1
                    auto tile_index = (exp_index >= expected_result.size()) ? 0 : expected_result[exp_index];

                    const age::uint8_t* tile_ptr = &tile_data[tile_index * 8 * 8 + tile_offset];
                    for (int tile_pixel = 0; tile_pixel < 8; ++tile_pixel, ++pixel_offset)
                    {
                        bool found_background  = screen[line_offset + pixel_offset] == background;
                        bool expect_background = tile_ptr[tile_pixel] == 0;

                        if (found_background != expect_background)
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
        };
    }



    std::optional<bool> parse_boolean_out(const std::string&              str,
                                          const std::vector<std::string>& prefixes)
    {
        auto hex_out = parse_hex_out(str, prefixes);
        if (hex_out.size() != 1)
        {
            return {};
        }
        switch (hex_out[0])
        {
            case 0:
                return {false};
            case 1:
                return {true};
            default:
                return {};
        }
    }

    age::tester::run_test_t new_audio_test(bool expect_audio_out)
    {
        return [=](age::gb_emulator& emulator) {
            run_test(emulator);

            // evaluate test result by checking the first X samples of the last emulation iteration
            // (similar to gambatte/test/testrunner.cpp)
            const age::pcm_vector& audio_buffer = emulator.get_audio_buffer();
            AGE_ASSERT(audio_buffer.size() >= samples_per_frame)

            const age::pcm_sample first_sample = emulator.get_audio_buffer()[0];

            auto count = std::count(begin(audio_buffer),
                                    begin(audio_buffer) + samples_per_frame,
                                    first_sample);

            return expect_audio_out == (count != samples_per_frame);
        };
    }



    std::filesystem::path find_screenshot(const std::filesystem::path& rom_path,
                                          const std::string&           suffix)
    {
        auto base = rom_path;
        base.replace_extension(); // remove file extension

        auto screenshot = base;
        screenshot += suffix;
        if (std::filesystem::is_regular_file(screenshot))
        {
            return screenshot;
        }

        return {};
    }

} // namespace



void age::tester::schedule_rom_gambatte(const std::filesystem::path& rom_path,
                                        const schedule_test_t&       schedule)
{
    auto rom_contents = load_rom_file(rom_path);
    auto filename     = rom_path.filename().string();

    // check for expected audio output
    auto outaudio_cgb = parse_boolean_out(filename, {"_dmg08_cgb04c_outaudio", "_cgb04c_outaudio"});
    if (outaudio_cgb.has_value())
    {
        schedule(rom_contents, gb_hardware::cgb, gb_colors_hint::cgb_gambatte, new_audio_test(outaudio_cgb.value()));
    }
    auto outaudio_dmg = parse_boolean_out(filename, {"_dmg08_cgb04c_outaudio", "_dmg08_outaudio"});
    if (outaudio_dmg.has_value())
    {
        schedule(rom_contents, gb_hardware::dmg, gb_colors_hint::dmg_greyscale, new_audio_test(outaudio_dmg.value()));
    }

    // check for expected characters on screen
    auto out_cgb = parse_hex_out(filename, {"_dmg08_cgb04c_out", "_cgb04c_out"});
    if (!out_cgb.empty())
    {
        schedule(rom_contents, gb_hardware::cgb, gb_colors_hint::cgb_gambatte, new_hex_out_test(out_cgb));
    }
    auto out_dmg = parse_hex_out(filename, {"_dmg08_cgb04c_out", "_dmg08_out"});
    if (!out_dmg.empty())
    {
        schedule(rom_contents, gb_hardware::dmg, gb_colors_hint::dmg_greyscale, new_hex_out_test(out_dmg));
    }

    // check for screenshot file
    auto screen_cgb = find_screenshot(rom_path, "_cgb04c.png");
    if (!screen_cgb.empty())
    {
        schedule(rom_contents, gb_hardware::cgb, gb_colors_hint::cgb_gambatte, new_screenshot_test(screen_cgb, is_test_finished));
    }
    auto screen_dmg = find_screenshot(rom_path, "_dmg08.png");
    if (!screen_dmg.empty())
    {
        schedule(rom_contents, gb_hardware::dmg, gb_colors_hint::dmg_greyscale, new_screenshot_test(screen_dmg, is_test_finished));
    }
}
