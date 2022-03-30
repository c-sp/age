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

#include "age_tester_tasks.hpp"



namespace
{
    std::filesystem::path find_screenshot(const std::filesystem::path& rom_path)
    {
        auto screenshot_path = rom_path.parent_path() / "firstwhite-dmg-cgb.png";
        if (std::filesystem::is_regular_file(screenshot_path))
        {
            return screenshot_path;
        }
        return {};
    }

} // namespace



void age::tester::schedule_rom_firstwhite(const std::filesystem::path& rom_path,
                                          const schedule_test_t&       schedule)
{
    auto screenshot = find_screenshot(rom_path);
    if (!screenshot.empty())
    {
        auto rom_contents = load_rom_file(rom_path);

        auto schedule_test = [&, screenshot](gb_device_type device_type, gb_colors_hint colors_hint) {
            schedule({rom_contents,
                      device_type,
                      colors_hint,
                      new_screenshot_test(screenshot,
                                          run_until([=](const age::gb_emulator& emulator) {
                                              return emulator.get_emulated_cycles() >= emulator.get_cycles_per_second() / 2;
                                          }))});
        };

        schedule_test(gb_device_type::dmg, gb_colors_hint::dmg_greyscale);
        schedule_test(gb_device_type::cgb_abcd, gb_colors_hint::cgb_acid2);
        schedule_test(gb_device_type::cgb_e, gb_colors_hint::cgb_acid2);
    }
}
