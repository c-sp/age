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
    constexpr std::array dmg_tests{
        "/apu/div_write_trigger.gb",
        "/apu/div_write_trigger_10.gb"};

    bool allow_dmg_test(std::string normalized_rom_path)
    {
        return std::any_of(begin(dmg_tests),
                           end(dmg_tests),
                           [&](const auto* allowed) {
                               return normalized_rom_path.find(allowed) != std::string::npos;
                           });
    }

} // namespace



age::tr::age_tr_module age::tr::create_same_suite_module()
{
    return {
        's',
        "same-suite",
        "run SameSuite test roms",
        "same-suite",
        [](const std::filesystem::path& rom_path) {
            auto normalized_rom_path = normalize_path_separator(rom_path);
            auto rom_contents        = load_rom_file(rom_path);

            std::vector<age::tr::age_tr_test> tests;
            tests.emplace_back(
                rom_path,
                rom_contents,
                gb_device_type::cgb_e,
                finished_after_ld_b_b(),
                succeeded_with_fibonacci_regs());

            if (allow_dmg_test(normalized_rom_path))
            {
                tests.emplace_back(
                    rom_path,
                    rom_contents,
                    gb_device_type::dmg,
                    finished_after_ld_b_b(),
                    succeeded_with_fibonacci_regs());
            }

            return tests;
        }};
}
