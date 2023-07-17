//
// © 2022 Christoph Sprenger <https://github.com/c-sp>
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

#include <optional>
#include <unordered_set>



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

    std::unordered_set<age::gb_device_type> parse_cgb_types(const std::string& filename, size_t idx)
    {
        std::unordered_set<age::gb_device_type> result;

        for (; idx < filename.size(); ++idx)
        {
            auto dev_type = parse_cgb_type(filename.at(idx));
            if (!dev_type.has_value())
            {
                break;
            }
            result.insert(dev_type.value());
        }

        // if this test is not constrained to any CGB type,
        // run it for all CGB types
        if (result.empty())
        {
            result = {
                age::gb_device_type::cgb_abcd,
                age::gb_device_type::cgb_e,
            };
        }
        return result;
    }



    std::unordered_set<age::gb_device_type> parse_device_types(const std::string& filename)
    {
        std::unordered_set<age::gb_device_type> result;

        // test runs on all CGB types
        if (filename.find("-C") != std::string::npos)
        {
            result.insert(age::gb_device_type::cgb_abcd);
            result.insert(age::gb_device_type::cgb_e);
        }

        // test runs on specific CGB types
        std::string cgb_prefix("-cgb");
        auto        cgb_hint = filename.find(cgb_prefix);
        if (cgb_hint != std::string::npos)
        {
            auto cgb_types = parse_cgb_types(filename, cgb_hint + cgb_prefix.size());
            result.insert(begin(cgb_types), end(cgb_types));
        }

        // test runs on all DMG types
        // (we don't distinguish DMG types at the moment)
        if ((filename.find("-G") != std::string::npos) || (filename.find("-dmg") != std::string::npos))
        {
            result.insert(age::gb_device_type::dmg);
        }

        // if this test is not constrained to any device type,
        // run it for all device types
        if (result.empty())
        {
            result = {
                age::gb_device_type::dmg,
                age::gb_device_type::cgb_abcd,
                age::gb_device_type::cgb_e,
            };
        }
        return result;
    }



    bool run_test(age::gb_emulator& emulator)
    {
        // run the test
        int cycles_per_step = emulator.get_cycles_per_second() / 256;
        int max_cycles      = emulator.get_cycles_per_second() * 120;

        for (int cycles = 0; cycles < max_cycles; cycles += cycles_per_step)
        {
            emulator.emulate(cycles_per_step);
            // the test is finished when LD B, B has been executed
            if (emulator.get_test_info().m_invalid_opcode == 0xED)
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



void age::tr::schedule_rom_mooneye_wilbertpol(const std::filesystem::path& rom_path,
                                              const schedule_test_t&       schedule)
{
    auto filename     = rom_path.filename().string();
    auto rom_contents = load_rom_file(rom_path);

    auto device_types = parse_device_types(filename);
    std::for_each(begin(device_types),
                  end(device_types),
                  [&](const auto& dev_type) {
                      schedule({rom_contents, dev_type, run_test});
                  });
}
