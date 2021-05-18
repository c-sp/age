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
    bool run_mooneye_test(age::gb_emulator& emulator)
    {
        // based on mooneye-gb/src/acceptance_tests/fixture.rs

        // run the test
        int cycles_per_step = emulator.get_cycles_per_second() >> 8;
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

} // namespace



void age::tester::schedule_rom_mooneye_gb(const std::filesystem::path& rom_path,
                                          const schedule_test_t&       schedule)
{
    auto filename     = rom_path.filename().string();
    bool explicit_cgb = (filename.find("-cgb") != std::string::npos) || (filename.find("-C") != std::string::npos);
    bool explicit_dmg = (filename.find("-dmg") != std::string::npos) || (filename.find("-G") != std::string::npos);

    auto rom_contents = load_rom_file(rom_path);
    if (explicit_cgb || !explicit_dmg)
    {
        schedule(rom_contents, gb_hardware::cgb, gb_colors_hint::default_colors, run_mooneye_test);
    }
    if (explicit_dmg || !explicit_cgb)
    {
        schedule(rom_contents, gb_hardware::dmg, gb_colors_hint::default_colors, run_mooneye_test);
    }
}
