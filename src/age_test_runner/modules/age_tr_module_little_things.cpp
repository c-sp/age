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
    void add_firstwhite(const std::filesystem::path&       rom_path,
                        std::vector<age::tr::age_tr_test>& tests)
    {
        auto screenshot = rom_path.parent_path() / "firstwhite-dmg-cgb.png";
        if (std::filesystem::is_regular_file(screenshot))
        {
            auto rom_contents = age::tr::load_rom_file(rom_path);

            tests.emplace_back(
                rom_path,
                rom_contents,
                age::gb_device_type::cgb_abcd,
                age::tr::finished_after_milliseconds(500),
                age::tr::succeeded_with_screenshot(screenshot));

            tests.emplace_back(
                rom_path,
                rom_contents,
                age::gb_device_type::cgb_e,
                age::tr::finished_after_milliseconds(500),
                age::tr::succeeded_with_screenshot(screenshot));

            tests.emplace_back(
                rom_path,
                rom_contents,
                age::gb_device_type::dmg,
                age::tr::finished_after_milliseconds(500),
                age::tr::succeeded_with_screenshot(screenshot));
        }
    }



    void run_tellinglys(age::gb_emulator& emulator)
    {
        // wait for the main menu
        auto cycles_per_seconds = emulator.get_cycles_per_second();
        emulator.emulate(cycles_per_seconds);

        // emulate button presses
        int        clks_button_wait = cycles_per_seconds / 20;
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
        emulator.emulate(5 * cycles_per_seconds);
    }

    void add_tellinglys(const std::filesystem::path&       rom_path,
                        std::vector<age::tr::age_tr_test>& tests)
    {
        auto rom_contents = age::tr::load_rom_file(rom_path);

        auto cgb_screenshot = rom_path.parent_path() / "tellinglys-cgb.png";
        if (std::filesystem::is_regular_file(cgb_screenshot))
        {
            tests.emplace_back(
                rom_path,
                rom_contents,
                age::gb_device_type::cgb_abcd,
                run_tellinglys,
                age::tr::succeeded_with_screenshot(cgb_screenshot));

            tests.emplace_back(
                rom_path,
                rom_contents,
                age::gb_device_type::cgb_e,
                run_tellinglys,
                age::tr::succeeded_with_screenshot(cgb_screenshot));
        }

        auto dmg_screenshot = rom_path.parent_path() / "tellinglys-dmg.png";
        if (std::filesystem::is_regular_file(dmg_screenshot))
        {
            tests.emplace_back(
                rom_path,
                rom_contents,
                age::gb_device_type::dmg,
                run_tellinglys,
                age::tr::succeeded_with_screenshot(dmg_screenshot));
        }
    }

} // namespace



age::tr::age_tr_module age::tr::create_little_things_module()
{
    return {
        't',
        "little-things",
        "run little-things-gb test roms",
        "little-things-gb",
        [](const std::filesystem::path& rom_path) {
            std::vector<age::tr::age_tr_test> tests;

            auto filename = rom_path.filename().string();
            if (filename == "firstwhite.gb")
            {
                add_firstwhite(rom_path, tests);
            }
            else if (filename == "tellinglys.gb")
            {
                add_tellinglys(rom_path, tests);
            }

            return tests;
        }};
}
