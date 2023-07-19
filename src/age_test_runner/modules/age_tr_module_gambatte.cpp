//
// © 2023 Christoph Sprenger <https://github.com/c-sp>
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

#include "age_tr_module.hpp"

#include <algorithm>
#include <cassert>
#include <optional>



namespace
{
    constexpr age::uint8_t _ = 0;
    constexpr age::uint8_t O = 1;

    // tile_data array contents copied from gambatte code
    // (gambatte/test/testrunner.cpp)
    constexpr age::uint8_array<16U * 8 * 8> tile_data
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



    //!
    //! gambatte tests run for 15 frames
    //! (see gambatte/test/testrunner.cpp)
    //!
    constexpr int test_frames = 15;

    //!
    //! The last 35112 samples generated by a gambatte "outaudio" test verified
    //! (see gambatte/test/testrunner.cpp).
    //! AGE generates 1 sample for every 2 clock cycles.
    //!
    constexpr int samples_per_frame = 35112;

    void run_gambatte_test(age::gb_emulator& emulator)
    {
        // tests are run frame by frame
        // (easier sample verification for "outaudio" tests)
        assert(emulator.get_pcm_sampling_rate() == emulator.get_cycles_per_second() / 2);
        for (int i = test_frames; i >= 0; --i)
        {
            emulator.emulate(2 * samples_per_frame);
        }
    }

    std::function<bool(const age::gb_emulator&)> succeed_with_hex_out(const age::uint8_vector& hex_out)
    {
        return [=](const age::gb_emulator& emulator) {
            const auto& screen       = emulator.get_screen_front_buffer();
            auto        screen_width = emulator.get_screen_width();

            // start with the first line
            // (the emulator screen buffer is filled upside down)
            int line_offset = 0;
            int tile_offset = 0;

            // the first pixel in the first line is always expected to be of the background color
            // (see tiles stored in tile_data)
            auto background = screen[line_offset];

            // examine each pixel
            for (int line = 0; line < 8; ++line)
            {
                int pixel_offset = 0;
                for (auto tile_index : hex_out)
                {
                    // Padding the expected result with zeroes to fail on e.g. result 0x1F when expecting 0x1
                    // does not work for all tests:
                    // e.g. gambatte/scx_during_m3/scx_m3_extend_1_dmg08_cgb04c_out3.gbc
                    // auto tile_index = (exp_index >= expected_result.size()) ? 0 : expected_result[exp_index];

                    ptrdiff_t tile_line_ofs = tile_index * 8 * 8 + tile_offset;
                    auto      tile          = std::span<const uint8_t, 8>{tile_data.begin() + tile_line_ofs, 8};

                    for (int tile_pixel = 0; tile_pixel < 8; ++tile_pixel, ++pixel_offset)
                    {
                        bool found_background  = screen[line_offset + pixel_offset] == background;
                        bool expect_background = tile[tile_pixel] == 0;

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

    std::function<bool(const age::gb_emulator&)> succeed_with_audio(bool audio)
    {
        return [=](const age::gb_emulator& emulator) {
            // evaluate test result by checking the first X samples of the last emulation iteration
            // (similar to gambatte/test/testrunner.cpp)
            const age::pcm_vector& audio_buffer = emulator.get_audio_buffer();
            assert(audio_buffer.size() >= samples_per_frame);

            const age::pcm_frame first_sample = emulator.get_audio_buffer()[0];

            auto count = std::count(begin(audio_buffer),
                                    begin(audio_buffer) + samples_per_frame,
                                    first_sample);

            return audio == (count != samples_per_frame);
        };
    }

} // namespace



age::tr::age_tr_module age::tr::create_gambatte_module()
{
    return {
        'g',
        "gambatte",
        "run Gambatte test roms",
        "gambatte",
        [](const std::filesystem::path& rom_path) {
            std::vector<age::tr::age_tr_test> tests;

            auto rom_contents = load_rom_file(rom_path);

            // check for expected characters on screen
            auto filename    = rom_path.filename().string();
            auto cgb_hex_out = parse_hex_out(filename, {"_dmg08_cgb04c_out", "_cgb04c_out"});
            if (!cgb_hex_out.empty())
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    age::gb_device_type::cgb_abcd,
                    run_gambatte_test,
                    succeed_with_hex_out(cgb_hex_out));
            }
            auto dmg_hex_out = parse_hex_out(filename, {"_dmg08_cgb04c_out", "_dmg08_out"});
            if (!dmg_hex_out.empty())
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    age::gb_device_type::dmg,
                    run_gambatte_test,
                    succeed_with_hex_out(dmg_hex_out));
            }

            // check for expected audio output
            auto outaudio_cgb = parse_boolean_out(filename, {"_dmg08_cgb04c_outaudio", "_cgb04c_outaudio"});
            if (outaudio_cgb.has_value())
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    age::gb_device_type::cgb_abcd,
                    run_gambatte_test,
                    succeed_with_audio(outaudio_cgb.value()));
            }
            auto outaudio_dmg = parse_boolean_out(filename, {"_dmg08_cgb04c_outaudio", "_dmg08_outaudio"});
            if (outaudio_dmg.has_value())
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    age::gb_device_type::dmg,
                    run_gambatte_test,
                    succeed_with_audio(outaudio_dmg.value()));
            }

            // check for screenshot file
            auto cgb_screenshot = find_screenshot(rom_path, "_cgb04c.png");
            if (!cgb_screenshot.empty())
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    age::gb_device_type::cgb_abcd,
                    gb_colors_hint::cgb_gambatte,
                    "",
                    run_gambatte_test,
                    succeeded_with_screenshot(cgb_screenshot));
            }
            auto dmg_screenshot = find_screenshot(rom_path, "_dmg08.png");
            if (!dmg_screenshot.empty())
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    age::gb_device_type::dmg,
                    run_gambatte_test,
                    succeeded_with_screenshot(dmg_screenshot));
            }

            return tests;
        }};
}
