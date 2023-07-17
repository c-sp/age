//
// © 2021 Christoph Sprenger <https://github.com/c-sp>
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

#include "age_tr_tasks.hpp"



namespace
{
    std::filesystem::path find_screenshot(const std::filesystem::path& dir,
                                          const std::string&           filename)
    {
        auto screenshot_path = dir / filename;
        if (std::filesystem::is_regular_file(screenshot_path))
        {
            return screenshot_path;
        }
        return {};
    }

} // namespace



void age::tr::schedule_rom_acid2_cgb(const std::filesystem::path& rom_path,
                                         const schedule_test_t&       schedule)
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
        schedule({rom_contents,
                  gb_device_type::cgb_abcd,
                  new_screenshot_test(cgb_screenshot, run_until(has_executed_ld_b_b))});

        schedule({rom_contents,
                  gb_device_type::cgb_e,
                  new_screenshot_test(cgb_screenshot, run_until(has_executed_ld_b_b))});
    }
}



void age::tr::schedule_rom_acid2_dmg(const std::filesystem::path& rom_path,
                                         const schedule_test_t&       schedule)
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
        schedule({rom_contents,
                  gb_device_type::cgb_abcd,
                  new_screenshot_test(cgb_screenshot, run_until(has_executed_ld_b_b))});

        schedule({rom_contents,
                  gb_device_type::cgb_e,
                  new_screenshot_test(cgb_screenshot, run_until(has_executed_ld_b_b))});
    }

    auto dmg_screenshot = find_screenshot(rom_path.parent_path(), "dmg-acid2-dmg.png");
    if (!dmg_screenshot.empty())
    {
        schedule({rom_contents,
                  gb_device_type::dmg,
                  new_screenshot_test(dmg_screenshot, run_until(has_executed_ld_b_b))});
    }
}
