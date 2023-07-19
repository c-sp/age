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
    std::function<bool(const age::gb_emulator&)> finished_after_invalid_opcode()
    {
        return [](const age::gb_emulator& emulator) {
            int max_cycles = emulator.get_cycles_per_second() * 120;
            return emulator.get_test_info().m_invalid_opcode == 0xED
                   || emulator.get_emulated_cycles() >= max_cycles;
        };
    }
} // namespace



age::tr::age_tr_module age::tr::create_mooneye_wilbertpol_module()
{
    return {
        'o',
        "mooneye-wilbertpol",
        "run Mooneye Test Suite adjusted by wilbertpol",
        "mooneye-test-suite-wilbertpol",
        [](const std::filesystem::path& rom_path) {
            auto rom_contents = load_rom_file(rom_path);
            auto device_types = parse_device_types(rom_path.filename());

            std::vector<age::tr::age_tr_test> tests;
            tests.reserve(device_types.size());
            for (auto device_type : device_types)
            {
                tests.emplace_back(rom_path,
                                   rom_contents,
                                   device_type,
                                   finished_after_invalid_opcode(),
                                   succeeded_with_fibonacci_regs());
            }

            return tests;
        }};
}
