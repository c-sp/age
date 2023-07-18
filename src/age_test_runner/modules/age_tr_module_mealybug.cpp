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
    bool is_mealybug_mbc_test(const std::filesystem::path& rom_path)
    {
        return rom_path.filename().string() == "mbc3_rtc.gb";
    }

} // namespace



age::tr::age_tr_module age::tr::create_mealybug_module()
{
    return {
        'n',
        "mealybug",
        "run Mealybug Tearoom test roms",
        "mealybug-tearoom-tests",
        [](const std::filesystem::path& rom_path) {
            std::vector<age::tr::age_tr_test> tests;

            auto rom_contents = load_rom_file(rom_path);

            if (is_mealybug_mbc_test(rom_path))
            {
                tests.emplace_back(rom_path, rom_contents, gb_device_type::dmg);
                tests.emplace_back(rom_path, rom_contents, gb_device_type::cgb_abcd);
                return tests;
            }

            auto cgb_screenshot = find_screenshot(rom_path, "_cgb_c.png");
            auto dmg_screenshot = find_screenshot(rom_path, "_dmg_blob.png");

            if (dmg_screenshot.empty() && cgb_screenshot.empty())
            {
                return tests;
            }

            if (!cgb_screenshot.empty())
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    gb_device_type::cgb_abcd,
                    finished_after_ld_b_b(),
                    succeeded_with_screenshot(cgb_screenshot));
            }
            if (!dmg_screenshot.empty())
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    gb_device_type::dmg,
                    finished_after_ld_b_b(),
                    succeeded_with_screenshot(dmg_screenshot));
            }

            return tests;
        }};
}
