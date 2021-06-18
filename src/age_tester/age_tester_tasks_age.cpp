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



void age::tester::schedule_rom_age(const std::filesystem::path& rom_path,
                                   const schedule_test_t&       schedule)
{
    auto filename     = rom_path.filename().string();
    bool explicit_cgb = filename.find("-cgb") != std::string::npos;
    bool explicit_dmg = filename.find("-dmg") != std::string::npos;

    auto rom_contents = load_rom_file(rom_path);
    if (explicit_cgb || !explicit_dmg)
    {
        schedule(rom_contents, gb_hardware::cgb, gb_colors_hint::default_colors, run_common_test);
    }
    if (explicit_dmg || !explicit_cgb)
    {
        schedule(rom_contents, gb_hardware::dmg, gb_colors_hint::default_colors, run_common_test);
    }
}
