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
    int test_duration_ms(const std::string& screenshot_filename, age::gb_device_type device_type)
    {
        // see https://github.com/c-sp/gameboy-test-roms/blob/master/src/howto/blargg.md

        if (screenshot_filename == "cgb_sound-cgb.png")
        {
            return 37000;
        }
        if (screenshot_filename == "cpu_instrs-dmg-cgb.png")
        {
            return device_type == age::gb_device_type::dmg ? 55000 : 31000;
        }
        if (screenshot_filename == "dmg_sound-dmg.png")
        {
            return 36000;
        }
        if (screenshot_filename == "halt_bug-dmg-cgb.png")
        {
            return 2000;
        }
        if (screenshot_filename == "instr_timing-dmg-cgb.png")
        {
            return 1000;
        }
        if (screenshot_filename == "interrupt_time-cgb.png")
        {
            return 2000;
        }
        if (screenshot_filename == "interrupt_time-dmg.png")
        {
            return 2000;
        }
        if (screenshot_filename == "mem_timing-dmg-cgb.png")
        {
            return 4000; // == max(3000, 4000)  =>  mem-timing and mem-timing-2
        }
        return 0;
    }

} // namespace



age::tr::age_tr_module age::tr::create_blargg_module()
{
    return {
        'b',
        "blargg",
        "run Blarggs test roms",
        "blargg",
        [](const std::filesystem::path& rom_path) {
            std::vector<age::tr::age_tr_test> tests;

            auto rom_contents = load_rom_file(rom_path);

            auto dmg_cgb_screenshot = find_screenshot(rom_path, "-dmg-cgb.png");

            auto cgb_screenshot = dmg_cgb_screenshot.empty() ? find_screenshot(rom_path, "-cgb.png") : dmg_cgb_screenshot;
            if (!cgb_screenshot.empty())
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    gb_device_type::cgb_abcd,
                    finished_after_milliseconds(test_duration_ms(cgb_screenshot.filename(), gb_device_type::cgb_abcd)),
                    succeeded_with_screenshot(cgb_screenshot));

                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    gb_device_type::cgb_e,
                    finished_after_milliseconds(test_duration_ms(cgb_screenshot.filename(), gb_device_type::cgb_e)),
                    succeeded_with_screenshot(cgb_screenshot));
            }

            auto dmg_screenshot = dmg_cgb_screenshot.empty() ? find_screenshot(rom_path, "-dmg.png") : dmg_cgb_screenshot;
            if (!dmg_screenshot.empty())
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    gb_device_type::dmg,
                    finished_after_milliseconds(test_duration_ms(dmg_screenshot.filename(), gb_device_type::dmg)),
                    succeeded_with_screenshot(dmg_screenshot));
            }

            return tests;
        }};
}
