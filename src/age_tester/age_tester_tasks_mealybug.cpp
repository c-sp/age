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



namespace
{
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

    bool is_test_finished(const age::gb_emulator& emulator)
    {
        return emulator.get_test_info().m_ld_b_b;
    }

} // namespace



void age::tester::schedule_rom_mealybug(const std::filesystem::path& rom_path,
                                        const schedule_test_t&       schedule)
{
    auto cgb_screenshot = find_screenshot(rom_path, "_cgb_c.png");
    auto dmg_screenshot = find_screenshot(rom_path, "_dmg_blob.png");

    if (dmg_screenshot.empty() && cgb_screenshot.empty())
    {
        return;
    }

    auto rom_contents = load_rom_file(rom_path);

    if (!cgb_screenshot.empty())
    {
        schedule(rom_contents, gb_device_type::cgb_abcd, gb_colors_hint::cgb_acid2, new_screenshot_test(cgb_screenshot, is_test_finished));
    }
    if (!dmg_screenshot.empty())
    {
        schedule(rom_contents, gb_device_type::dmg, gb_colors_hint::dmg_greyscale, new_screenshot_test(dmg_screenshot, is_test_finished));
    }
}
