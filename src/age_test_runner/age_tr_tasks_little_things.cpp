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
    age::tr::run_test_t new_tellinglys_test(const std::filesystem::path& screenshot_file)
    {
        return age::tr::new_screenshot_test(screenshot_file, [=](age::gb_emulator& emulator) {
            // wait for the main menu
            emulator.emulate(age::gb_clock_cycles_per_second);

            // emulate button presses
            int        clks_button_wait = age::gb_clock_cycles_per_second / 20;
            std::array press_buttons{age::gb_start, age::gb_select,
                                     age::gb_a, age::gb_b,
                                     age::gb_up, age::gb_down,
                                     age::gb_left, age::gb_right};

            std::for_each(begin(press_buttons), end(press_buttons), [&](int button) {
                emulator.set_buttons_down(button);
                emulator.emulate(clks_button_wait);
                emulator.set_buttons_up(button);
                emulator.emulate(clks_button_wait);
            });

            // wait for test to finish
            emulator.emulate(5 * age::gb_clock_cycles_per_second);

            return true;
        });
    }

} // namespace



void age::tr::schedule_rom_little_things(const std::filesystem::path& rom_path,
                                         const schedule_test_t&       schedule)
{
    if (rom_path.filename().string() == "firstwhite.gb")
    {
        auto screenshot_path = rom_path.parent_path() / "firstwhite-dmg-cgb.png";
        if (std::filesystem::is_regular_file(screenshot_path))
        {
            auto rom_contents = load_rom_file(rom_path);

            auto schedule_test = [&, screenshot_path](gb_device_type device_type, gb_colors_hint colors_hint) {
                schedule({rom_contents,
                          device_type,
                          colors_hint,
                          new_screenshot_test(screenshot_path,
                                              run_until([=](const gb_emulator& emulator) {
                                                  return emulator.get_emulated_cycles() >= emulator.get_cycles_per_second() / 2;
                                              }))});
            };

            schedule_test(gb_device_type::dmg, gb_colors_hint::dmg_greyscale);
            schedule_test(gb_device_type::cgb_abcd, gb_colors_hint::cgb_acid2);
            schedule_test(gb_device_type::cgb_e, gb_colors_hint::cgb_acid2);
        }
    }

    if (rom_path.filename().string() == "tellinglys.gb")
    {
        auto rom_contents = load_rom_file(rom_path);

        auto screenshot_path_cgb = rom_path.parent_path() / "tellinglys-cgb.png";
        if (std::filesystem::is_regular_file(screenshot_path_cgb))
        {
            auto run_test = new_tellinglys_test(screenshot_path_cgb);
            schedule({rom_contents, gb_device_type::cgb_abcd, run_test});
            schedule({rom_contents, gb_device_type::cgb_e, run_test});
        }

        auto screenshot_path_dmg = rom_path.parent_path() / "tellinglys-dmg.png";
        if (std::filesystem::is_regular_file(screenshot_path_dmg))
        {
            auto run_test = new_tellinglys_test(screenshot_path_dmg);
            schedule({rom_contents, gb_device_type::dmg, run_test});
        }
    }
}
