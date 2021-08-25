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

#include <cstdio>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>



namespace
{
    age::pixel_vector load_png(const std::filesystem::path& screenshot_png_path,
                               const age::gb_emulator&      emulator)
    {
        FILE*             file   = fopen(screenshot_png_path.string().c_str(), "rb");
        age::pixel_vector result = age::read_png_file(file, emulator.get_screen_width(), emulator.get_screen_height());
        fclose(file);
        return result;
    }

} // namespace



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
                                                         const test_finished_t&       test_finished)
{
    return [=](age::gb_emulator& emulator) {
        int cycles_per_step = emulator.get_cycles_per_second() >> 8;

        while (!test_finished(emulator))
        {
            emulator.emulate(cycles_per_step);
        }

        // load png
        auto png_data = load_png(screenshot_png_path, emulator);

        // compare screen to screenshot
        const auto* screen     = emulator.get_screen_front_buffer().data();
        const auto* screenshot = png_data.data();
        if (!screenshot)
        {
            std::cout << "could not load screenshot: " << screenshot_png_path.string() << std::endl;
            return false;
        }

        for (int i = 0, max = emulator.get_screen_width() * emulator.get_screen_height(); i < max; ++i)
        {
            if (*screen != *screenshot)
            {
                // int x = i % emulator.get_screen_width();
                // int y = emulator.get_screen_height() - 1 - (i / emulator.get_screen_width());
                // AGE_LOG(screenshot_png_path << ": screen and screenshot differ at position ("
                //         << x << ',' << y << ')'
                //         << ": expected 0x" << std::hex << screenshot->m_color << ", found 0x" << screen->m_color);
                return false;
            }
            ++screen;
            ++screenshot;
        }

        return true;
    };
}
