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



age::tr::age_tr_module age::tr::create_age_module()
{
    return {
        'e',
        "age",
        "run age test roms",
        "age-test-roms",
        [](const std::filesystem::path& rom_path) {
            std::vector<age::tr::age_tr_test> tests;

            auto rom_contents = load_rom_file(rom_path);

            auto screenshots = find_screenshots(rom_path);
            if (screenshots.empty())
            {
                auto device_types = parse_device_types(rom_path.filename());

                tests.reserve(device_types.size());
                for (auto device_type : device_types)
                {
                    tests.emplace_back(rom_path, rom_contents, device_type);
                }
            }
            else
            {
                for (auto& screenshot : screenshots)
                {
                    auto device_types = parse_device_types(screenshot.filename());
                    for (auto device_type : device_types)
                    {
                        tests.emplace_back(rom_path,
                                           rom_contents,
                                           device_type,
                                           finished_after_ld_b_b(),
                                           succeeded_with_screenshot(screenshot));
                    }
                }
            }

            return tests;
        }};
}
