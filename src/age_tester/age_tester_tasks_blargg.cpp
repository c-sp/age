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
                                          const std::string&           prefix)
    {
        auto filename            = rom_path.filename();
        bool is_prefixed         = filename.string().substr(0, prefix.length()) == prefix;
        auto screenshot_filename = (is_prefixed ? filename : std::filesystem::path(prefix + filename.string())).replace_extension(".png");

        auto screenshot_path = rom_path.parent_path() / screenshot_filename;
        if (std::filesystem::is_regular_file(screenshot_path))
        {
            return screenshot_path;
        }
        return {};
    }



    int get_test_seconds(const std::string& screenshot_filename)
    {
        // see https://github.com/c-sp/gameboy-test-roms

        if (screenshot_filename == "cgb_cpu_instrs.png")
        {
            return 31;
        }
        if (screenshot_filename == "cgb_halt_bug.png")
        {
            return 2;
        }
        if (screenshot_filename == "cgb_interrupt_time.png")
        {
            return 2;
        }
        if (screenshot_filename == "cgb_instr_timing.png")
        {
            return 1;
        }
        if (screenshot_filename == "cgb_mem_timing.png")
        {
            return 4; // == max(3, 4)  =>  mem-timing and mem-timing-2
        }
        if (screenshot_filename == "cgb_sound.png")
        {
            return 37;
        }

        if (screenshot_filename == "dmg_cpu_instrs.png")
        {
            return 55;
        }
        if (screenshot_filename == "dmg_halt_bug.png")
        {
            return 2;
        }
        if (screenshot_filename == "dmg_instr_timing.png")
        {
            return 1;
        }
        if (screenshot_filename == "dmg_mem_timing.png")
        {
            return 4; // == max(3, 4)  =>  mem-timing and mem-timing-2
        }
        if (screenshot_filename == "dmg_sound.png")
        {
            return 36;
        }
        return 0;
    }

} // namespace



void age::tester::schedule_rom_blargg(const std::filesystem::path& rom_path,
                                      const schedule_test_t&       schedule)
{
    auto rom_contents = load_rom_file(rom_path);

    auto schedule_test = [&](gb_device_type device_type, gb_colors_hint colors_hint, const std::filesystem::path& screenshot_path) {
        age::int64_t seconds = get_test_seconds(screenshot_path.filename().string());

        if (seconds > 0)
        {
            schedule(rom_contents,
                     device_type,
                     colors_hint,
                     new_screenshot_test(screenshot_path,
                                         [=](const age::gb_emulator& emulator) {
                                             return emulator.get_emulated_cycles() >= seconds * emulator.get_cycles_per_second();
                                         }));
        }
    };

    auto cgb_screenshot = find_screenshot(rom_path, "cgb_");
    if (!cgb_screenshot.empty())
    {
        schedule_test(gb_device_type::cgb_abcd, gb_colors_hint::cgb_gambatte, cgb_screenshot);
        schedule_test(gb_device_type::cgb_e, gb_colors_hint::cgb_gambatte, cgb_screenshot);
    }

    auto dmg_screenshot = find_screenshot(rom_path, "dmg_");
    if (!dmg_screenshot.empty())
    {
        schedule_test(gb_device_type::dmg, gb_colors_hint::dmg_greyscale, dmg_screenshot);
    }
}
