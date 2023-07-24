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

#include <iostream>



namespace
{
    bool test_succeeded(const age::gb_emulator& emulator)
    {
        auto test_info = emulator.get_test_info();

        if (test_info.m_ff82 == 0x01)
        {
            return true;
        }
        if (test_info.m_ff82 == 0xFF)
        {
            return false;
        }

        std::cout << "unexpected GBMicrotest result: "
                  << static_cast<int>(test_info.m_ff82)
                  << std::endl;
        return false;
    }

} // namespace



age::tr::age_tr_module age::tr::create_gbmicrotest_module()
{
    return {
        'i',
        "gbmicrotest",
        "run GBMicrotest test roms",
        "gbmicrotest",
        [](const std::filesystem::path& rom_path) {
            auto normalized_rom_path = normalize_path_separator(rom_path);
            auto rom_contents        = load_rom_file(rom_path);

            int millis = rom_path.filename().string() == "is_if_set_during_ime0.gb"
                             ? 380
                             : 1000 / 59 * 2; // about 2 frames

            std::vector<age::tr::age_tr_test> tests;
            tests.emplace_back(rom_path,
                               rom_contents,
                               gb_device_type::dmg,
                               finished_after_milliseconds(millis),
                               test_succeeded);

            return tests;
        }};
}
