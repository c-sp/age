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
    void add_cgb_acid2(const std::filesystem::path&       rom_path,
                       std::vector<age::tr::age_tr_test>& tests)
    {
        auto rom_contents = age::tr::load_rom_file(rom_path);

        auto cgb_screenshot = age::tr::find_screenshot(rom_path, ".png");
        if (!cgb_screenshot.empty())
        {
            tests.emplace_back(
                rom_path,
                rom_contents,
                age::gb_device_type::cgb_abcd,
                age::tr::finished_after_ld_b_b(),
                age::tr::succeeded_with_screenshot(cgb_screenshot));

            tests.emplace_back(
                rom_path,
                rom_contents,
                age::gb_device_type::cgb_e,
                age::tr::finished_after_ld_b_b(),
                age::tr::succeeded_with_screenshot(cgb_screenshot));
        }
    }

    void add_dmg_acid2(const std::filesystem::path&       rom_path,
                       std::vector<age::tr::age_tr_test>& tests)
    {
        auto rom_contents = age::tr::load_rom_file(rom_path);

        auto cgb_screenshot = age::tr::find_screenshot(rom_path, "-cgb.png");
        if (!cgb_screenshot.empty())
        {
            tests.emplace_back(
                rom_path,
                rom_contents,
                age::gb_device_type::cgb_abcd,
                age::tr::finished_after_ld_b_b(),
                age::tr::succeeded_with_screenshot(cgb_screenshot));

            tests.emplace_back(
                rom_path,
                rom_contents,
                age::gb_device_type::cgb_e,
                age::tr::finished_after_ld_b_b(),
                age::tr::succeeded_with_screenshot(cgb_screenshot));
        }

        auto dmg_screenshot = age::tr::find_screenshot(rom_path, "-dmg.png");
        if (!dmg_screenshot.empty())
        {
            tests.emplace_back(
                rom_path,
                rom_contents,
                age::gb_device_type::dmg,
                age::tr::finished_after_ld_b_b(),
                age::tr::succeeded_with_screenshot(dmg_screenshot));
        }
    }

} // namespace



age::tr::age_tr_module age::tr::create_acid2_module()
{
    return {
        'a',
        "acid2",
        "run cgb-acid-2 and dmg-acid-2 test roms",
        std::vector<std::string>{"cgb-acid2", "dmg-acid2"},
        [](const std::filesystem::path& rom_path) {
            std::vector<age::tr::age_tr_test> tests;

            auto filename = rom_path.filename().string();
            if (filename == "cgb-acid2.gbc")
            {
                add_cgb_acid2(rom_path, tests);
            }
            else if (filename == "dmg-acid2.gb")
            {
                add_dmg_acid2(rom_path, tests);
            }

            return tests;
        }};
}
