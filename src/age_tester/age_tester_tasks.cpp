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

#include <gfx/age_png.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>



bool age::tester::run_common_test(age::gb_emulator& emulator)
{
    // used for Mooneye-GB, SameSuite and AGE test roms
    // (originally based on mooneye-gb/src/acceptance_tests/fixture.rs)

    // run the test
    int cycles_per_step = emulator.get_cycles_per_second() / 256;
    int max_cycles      = emulator.get_cycles_per_second() * 120;

    for (int cycles = 0; cycles < max_cycles; cycles += cycles_per_step)
    {
        emulator.emulate(cycles_per_step);
        // the test is finished when LD B, B has been executed
        if (emulator.get_test_info().m_ld_b_b)
        {
            break;
        }
    }

    // evaluate the test result
    // (test passed => fibonacci sequence in cpu regs)
    age::gb_test_info info = emulator.get_test_info();
    return (3 == info.m_b) && (5 == info.m_c) && (8 == info.m_d) && (13 == info.m_e) && (21 == info.m_h) && (34 == info.m_l);
}



bool age::tester::has_executed_ld_b_b(const age::gb_emulator& emulator)
{
    return emulator.get_test_info().m_ld_b_b;
}

age::tester::run_test_t age::tester::run_until(const std::function<bool(const age::gb_emulator&)>& test_finished)
{
    return [=](age::gb_emulator& emulator) {
        int step_cycles = emulator.get_cycles_per_second() / 256;
        while (!test_finished(emulator))
        {
            emulator.emulate(step_cycles);
        }
        return true;
    };
}



std::string age::tester::normalize_path_separator(const std::string& path)
{
    if constexpr (std::filesystem::path::preferred_separator == '/')
    {
        return path;
    }
    std::string result = path;
    std::replace(begin(result), end(result), static_cast<char>(std::filesystem::path::preferred_separator), '/');
    return result;
}



std::shared_ptr<age::uint8_vector> age::tester::load_rom_file(const std::filesystem::path& rom_path)
{
    std::ifstream rom_file(rom_path, std::ios::in | std::ios::binary);
    return std::make_shared<age::uint8_vector>((std::istreambuf_iterator<char>(rom_file)), std::istreambuf_iterator<char>());
}



age::tester::run_test_t age::tester::new_screenshot_test(const std::filesystem::path& screenshot_png_path,
                                                         const run_test_t&            run_test)
{
    return [=](age::gb_emulator& emulator) {
        run_test(emulator);

        // load png
        auto screenshot = read_png_file(screenshot_png_path,
                                        emulator.get_screen_width(),
                                        emulator.get_screen_height());
        if (screenshot.empty())
        {
            std::cout << "could not load screenshot: " << screenshot_png_path.string() << std::endl;
            return false;
        }

        // compare screen to screenshot
        auto screen = emulator.get_screen_front_buffer();
        if (screenshot.size() != screen.size())
        {
            std::cout << "screenshot size mismatch (expected "
                      << screen.size()
                      << " pixel, got "
                      << screenshot.size()
                      << "pixel): " << screenshot_png_path.string() << std::endl;
            return false;
        }

        bool screenshot_matches = screen == screenshot;
        if (!screenshot_matches)
        {
            auto png_path = screenshot_png_path;
            png_path.replace_extension();
            png_path += "_actual.png";

            write_png_file(emulator.get_screen_front_buffer(),
                           emulator.get_screen_width(),
                           emulator.get_screen_height(),
                           png_path.string());
        }

        return screenshot_matches;
    };
}
