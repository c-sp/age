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

#include <optional>



namespace
{
    std::optional<age::gb_device_type> parse_cgb_type(char c)
    {
        switch (c)
        {
            case 'A':
            case 'B':
            case 'C':
            case 'D':
                return {age::gb_device_type::cgb_abcd};

            case 'E':
                return {age::gb_device_type::cgb_e};

            default:
                return {};
        }
    }

} // namespace



void age::tester::schedule_rom_age(const std::filesystem::path& rom_path,
                                   const schedule_test_t&       schedule)
{
    auto filename     = rom_path.filename().string();
    auto rom_contents = load_rom_file(rom_path);

    // no need to parse DMG device types
    if (filename.find("-dmg") != std::string::npos)
    {
        schedule(rom_contents, gb_device_type::dmg, gb_colors_hint::default_colors, run_common_test);
    }

    // parse CGB device types
    auto schedule_cgb_tests = [&](const std::string& prefix) {
        auto cgb_hint = filename.find(prefix);
        if (cgb_hint != std::string::npos)
        {
            cgb_hint += prefix.size();

            for (; cgb_hint < filename.size(); ++cgb_hint)
            {
                auto dev_type = parse_cgb_type(filename.at(cgb_hint));
                if (!dev_type.has_value())
                {
                    break;
                }
                schedule(rom_contents, dev_type.value(), gb_colors_hint::default_colors, run_common_test);
            }
        }
    };

    schedule_cgb_tests("-cgb");
    schedule_cgb_tests("-ncm");
}
