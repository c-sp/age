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
    std::filesystem::path find_screenshot(const std::filesystem::path &dir,
                                          const std::string &filename)
    {
        auto screenshot_path = dir / filename;
        if (is_regular_file(screenshot_path))
        {
            return screenshot_path;
        }
        return {};
    }

    bool is_test_finished(const age::gb_emulator &emulator)
    {
        return emulator.get_test_info().m_ld_b_b;
    }

} // namespace



void age::tester::schedule_rom_acid2_cgb(const std::filesystem::path &rom_path,
                                         const schedule_test_t &schedule)
{
    auto filename = rom_path.filename().string();
    if (filename != "cgb-acid2.gbc")
    {
        return;
    }
    auto rom_contents = load_rom_file(rom_path);

    auto cgb_screenshot = find_screenshot(rom_path.parent_path(), "cgb-acid2.png");
    if (!cgb_screenshot.empty())
    {
        schedule(rom_contents, gb_hardware::cgb, gb_colors_hint::cgb_acid2, new_screenshot_test(cgb_screenshot, is_test_finished));
    }
}



void age::tester::schedule_rom_acid2_dmg(const std::filesystem::path &rom_path,
                                         const schedule_test_t &schedule)
{
    auto filename = rom_path.filename().string();
    if (filename != "dmg-acid2.gb")
    {
        return;
    }
    auto rom_contents = load_rom_file(rom_path);

    auto cgb_screenshot = find_screenshot(rom_path.parent_path(), "dmg-acid2-cgb.png");
    if (!cgb_screenshot.empty())
    {
        schedule(rom_contents, gb_hardware::cgb, gb_colors_hint::default_colors, new_screenshot_test(cgb_screenshot, is_test_finished));
    }

    auto dmg_screenshot = find_screenshot(rom_path.parent_path(), "dmg-acid2-dmg.png");
    if (!dmg_screenshot.empty())
    {
        schedule(rom_contents, gb_hardware::dmg, gb_colors_hint::dmg_greyscale, new_screenshot_test(dmg_screenshot, is_test_finished));
    }
}
