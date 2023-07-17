//
// © 2022 Christoph Sprenger <https://github.com/c-sp>
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

#include "../age_emulator_gb/common/age_gb_clock.hpp"
#include "age_tr_tasks.hpp"



namespace
{
    age::tr::run_test_t new_rtc3test(const std::filesystem::path& screenshot_file,
                                         const std::vector<int>&      press_buttons,
                                         int                          test_seconds)
    {
        return age::tr::new_screenshot_test(screenshot_file, [=](age::gb_emulator& emulator) {
            // wait for the main menu
            emulator.emulate(age::gb_clock_cycles_per_second);

            // emulate button presses
            int clks_button_wait = age::gb_clock_cycles_per_second / 10;

            std::for_each(begin(press_buttons), end(press_buttons), [&](int button) {
                emulator.set_buttons_down(button);
                emulator.emulate(clks_button_wait);
                emulator.set_buttons_up(button);
                emulator.emulate(clks_button_wait);
            });

            // wait for test to finish
            for (int sec = 0; sec < test_seconds; ++sec)
            {
                emulator.emulate(age::gb_clock_cycles_per_second);
            }

            return true;
        });
    }



    std::filesystem::path find_screenshot(const std::filesystem::path& rom_path,
                                          const std::string&           screenshot_suffix)
    {
        auto base = rom_path;
        base.replace_extension(); // remove file extension

        auto screenshot = base;
        screenshot += screenshot_suffix;
        if (std::filesystem::is_regular_file(screenshot))
        {
            return screenshot;
        }

        return {};
    }



    constexpr int basic_tests_seconds       = 13;
    constexpr int range_tests_seconds       = 8;
    constexpr int seb_second_writes_seconds = 26;

    constexpr const char* basic_tests_info       = "basic tests";
    constexpr const char* range_tests_info       = "range tests";
    constexpr const char* sub_second_writes_info = "sub second writes";

} // namespace



void age::tr::schedule_rom_rtc3test(const std::filesystem::path& rom_path,
                                        const schedule_test_t&       schedule)
{
    auto rom_contents = load_rom_file(rom_path);

    // dmg tests

    auto dmg_basic_png = find_screenshot(rom_path, "-basic-tests-dmg.png");
    if (!dmg_basic_png.empty())
    {
        auto run_rtc3test = new_rtc3test(dmg_basic_png, {gb_a}, basic_tests_seconds);
        schedule({rom_contents, gb_device_type::dmg, run_rtc3test, basic_tests_info});
    }

    auto dmg_range_png = find_screenshot(rom_path, "-range-tests-dmg.png");
    if (!dmg_range_png.empty())
    {
        auto run_rtc3test = new_rtc3test(dmg_range_png, {gb_down, gb_a}, range_tests_seconds);
        schedule({rom_contents, gb_device_type::dmg, run_rtc3test, range_tests_info});
    }

    auto dmg_subsec_png = find_screenshot(rom_path, "-sub-second-writes-dmg.png");
    if (!dmg_subsec_png.empty())
    {
        auto run_rtc3test = new_rtc3test(dmg_subsec_png, {gb_down, gb_down, gb_a}, seb_second_writes_seconds);
        schedule({rom_contents, gb_device_type::dmg, run_rtc3test, sub_second_writes_info});
    }

    // cgb tests

    auto cgb_basic_png = find_screenshot(rom_path, "-basic-tests-cgb.png");
    if (!cgb_basic_png.empty())
    {
        auto run_rtc3test = new_rtc3test(cgb_basic_png, {gb_a}, basic_tests_seconds);
        schedule({rom_contents, gb_device_type::cgb_abcd, run_rtc3test, basic_tests_info});
        schedule({rom_contents, gb_device_type::cgb_e, run_rtc3test, basic_tests_info});
    }

    auto cgb_range_png = find_screenshot(rom_path, "-range-tests-cgb.png");
    if (!cgb_range_png.empty())
    {
        auto run_rtc3test = new_rtc3test(cgb_range_png, {gb_down, gb_a}, range_tests_seconds);
        schedule({rom_contents, gb_device_type::cgb_abcd, run_rtc3test, range_tests_info});
        schedule({rom_contents, gb_device_type::cgb_e, run_rtc3test, range_tests_info});
    }

    auto cgb_subsec_png = find_screenshot(rom_path, "-sub-second-writes-cgb.png");
    if (!cgb_subsec_png.empty())
    {
        auto run_rtc3test = new_rtc3test(cgb_subsec_png, {gb_down, gb_down, gb_a}, seb_second_writes_seconds);
        schedule({rom_contents, gb_device_type::cgb_abcd, run_rtc3test, sub_second_writes_info});
        schedule({rom_contents, gb_device_type::cgb_e, run_rtc3test, sub_second_writes_info});
    }
}
