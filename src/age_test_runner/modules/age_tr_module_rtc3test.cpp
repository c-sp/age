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



namespace
{
    std::function<void(age::gb_emulator&)> rtc3test_runner(const std::vector<int>& press_buttons,
                                                           int                     test_seconds)
    {
        return [=](age::gb_emulator& emulator) {
            auto cycles_per_second = emulator.get_cycles_per_second();

            // wait for the main menu
            emulator.emulate(cycles_per_second);

            // emulate button presses
            int clks_button_wait = cycles_per_second / 10;

            std::for_each(begin(press_buttons), end(press_buttons), [&](int button) {
                emulator.set_buttons_down(button);
                emulator.emulate(clks_button_wait);
                emulator.set_buttons_up(button);
                emulator.emulate(clks_button_wait);
            });

            // wait for test to finish
            for (int sec = 0; sec < test_seconds; ++sec)
            {
                emulator.emulate(cycles_per_second);
            }
        };
    }

    constexpr int basic_tests_seconds       = 13;
    constexpr int range_tests_seconds       = 8;
    constexpr int seb_second_writes_seconds = 26;

    constexpr const char* basic_tests_info       = "basic tests";
    constexpr const char* range_tests_info       = "range tests";
    constexpr const char* sub_second_writes_info = "sub second writes";

} // namespace



age::tr::age_tr_module age::tr::create_rtc3test_module()
{
    return {
        'r',
        "rtc3test",
        "run rtc3test test rom",
        "rtc3test",
        [](const std::filesystem::path& rom_path) {
            std::vector<age::tr::age_tr_test> tests;

            auto rom_contents = load_rom_file(rom_path);

            // dmg tests

            auto dmg_basic_png = find_screenshot(rom_path, "-basic-tests-dmg.png");
            if (!dmg_basic_png.empty())
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    gb_device_type::dmg,
                    basic_tests_info,
                    rtc3test_runner({gb_a}, basic_tests_seconds),
                    age::tr::succeeded_with_screenshot(dmg_basic_png));
            }

            auto dmg_range_png = find_screenshot(rom_path, "-range-tests-dmg.png");
            if (!dmg_range_png.empty())
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    gb_device_type::dmg,
                    range_tests_info,
                    rtc3test_runner({gb_down, gb_a}, range_tests_seconds),
                    age::tr::succeeded_with_screenshot(dmg_range_png));
            }

            auto dmg_subsec_png = find_screenshot(rom_path, "-sub-second-writes-dmg.png");
            if (!dmg_subsec_png.empty())
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    gb_device_type::dmg,
                    sub_second_writes_info,
                    rtc3test_runner({gb_down, gb_down, gb_a}, seb_second_writes_seconds),
                    age::tr::succeeded_with_screenshot(dmg_subsec_png));
            }

            // cgb tests

            auto cgb_basic_png = find_screenshot(rom_path, "-basic-tests-cgb.png");
            if (!cgb_basic_png.empty())
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    gb_device_type::cgb_abcd,
                    basic_tests_info,
                    rtc3test_runner({gb_a}, basic_tests_seconds),
                    age::tr::succeeded_with_screenshot(cgb_basic_png));

                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    gb_device_type::cgb_e,
                    basic_tests_info,
                    rtc3test_runner({gb_a}, basic_tests_seconds),
                    age::tr::succeeded_with_screenshot(cgb_basic_png));
            }

            auto cgb_range_png = find_screenshot(rom_path, "-range-tests-cgb.png");
            if (!cgb_range_png.empty())
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    gb_device_type::cgb_abcd,
                    range_tests_info,
                    rtc3test_runner({gb_down, gb_a}, range_tests_seconds),
                    age::tr::succeeded_with_screenshot(cgb_range_png));

                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    gb_device_type::cgb_e,
                    range_tests_info,
                    rtc3test_runner({gb_down, gb_a}, range_tests_seconds),
                    age::tr::succeeded_with_screenshot(cgb_range_png));
            }

            auto cgb_subsec_png = find_screenshot(rom_path, "-sub-second-writes-cgb.png");
            if (!cgb_subsec_png.empty())
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    gb_device_type::cgb_abcd,
                    sub_second_writes_info,
                    rtc3test_runner({gb_down, gb_down, gb_a}, seb_second_writes_seconds),
                    age::tr::succeeded_with_screenshot(cgb_subsec_png));

                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    gb_device_type::cgb_e,
                    sub_second_writes_info,
                    rtc3test_runner({gb_down, gb_down, gb_a}, seb_second_writes_seconds),
                    age::tr::succeeded_with_screenshot(cgb_subsec_png));
            }

            return tests;
        }};
}
