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

#include "age_tr_tasks.hpp"



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



void age::tr::schedule_rom_same_suite(const std::filesystem::path& rom_path,
                                          const schedule_test_t&       schedule)
{
    auto normalized_rom_path = age::tr::normalize_path_separator(rom_path.string());
    auto rom_contents        = load_rom_file(rom_path);

    schedule({rom_contents, gb_device_type::cgb_e, run_common_test});
    if (allow_dmg_test(normalized_rom_path))
    {
        schedule({rom_contents, gb_device_type::dmg, run_common_test});
    }
}
