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

#include "age_tester_tasks.hpp"



namespace
{
    std::filesystem::path find_screenshot(const std::filesystem::path& rom_path,
                                          const std::string&           suffix)
    {
        auto rom_path_filename = std::filesystem::path{rom_path}.replace_extension().string();

        for (const auto& entry : std::filesystem::directory_iterator{rom_path.parent_path()})
        {
            auto path = entry.path().string();

            if (!entry.is_regular_file()
                || (entry.path().extension().string() != ".png")
                || (path.find(rom_path_filename) != 0)
                || (path.find("_actual.png") != std::string::npos))
            {
                continue;
            }

            if (path.find(suffix, rom_path_filename.size()) != std::string::npos)
            {
                return path;
            }
        }
        return {};
    }



    int get_test_seconds(const std::string& screenshot_filename, age::gb_device_type device_type)
    {
        // see https://github.com/c-sp/gameboy-test-roms/blob/master/src/howto/blargg.md

        if (screenshot_filename == "cgb_sound-cgb.png")
        {
            return 37;
        }
        if (screenshot_filename == "cpu_instrs-dmg-cgb.png")
        {
            return device_type == age::gb_device_type::dmg ? 55 : 31;
        }
        if (screenshot_filename == "dmg_sound-dmg.png")
        {
            return 36;
        }
        if (screenshot_filename == "halt_bug-dmg-cgb.png")
        {
            return 2;
        }
        if (screenshot_filename == "instr_timing-dmg-cgb.png")
        {
            return 1;
        }
        if (screenshot_filename == "interrupt_time-cgb.png")
        {
            return 2;
        }
        if (screenshot_filename == "interrupt_time-dmg.png")
        {
            return 2;
        }
        if (screenshot_filename == "mem_timing-dmg-cgb.png")
        {
            return 4; // == max(3, 4)  =>  mem-timing and mem-timing-2
        }
        return 0;
    }

} // namespace



void age::tester::schedule_rom_blargg(const std::filesystem::path& rom_path,
                                      const schedule_test_t&       schedule)
{
    auto rom_contents = load_rom_file(rom_path);

    auto schedule_test = [&](gb_device_type device_type, const std::filesystem::path& screenshot_path) {
        const int64_t seconds = get_test_seconds(screenshot_path.filename().string(), device_type);

        if (seconds > 0)
        {
            schedule({rom_contents,
                      device_type,
                      new_screenshot_test(screenshot_path,
                                          run_until([=](const age::gb_emulator& emulator) {
                                              return emulator.get_emulated_cycles() >= seconds * emulator.get_cycles_per_second();
                                          }))});
        }
    };

    auto cgb_screenshot = find_screenshot(rom_path, "-cgb");
    if (!cgb_screenshot.empty())
    {
        schedule_test(gb_device_type::cgb_abcd, cgb_screenshot);
        schedule_test(gb_device_type::cgb_e, cgb_screenshot);
    }

    auto dmg_screenshot = find_screenshot(rom_path, "-dmg");
    if (!dmg_screenshot.empty())
    {
        schedule_test(gb_device_type::dmg, dmg_screenshot);
    }
}
