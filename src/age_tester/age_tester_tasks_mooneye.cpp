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



    std::unordered_set<age::gb_device_type> parse_decive_types(const std::string& filename)
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

} // namespace



void age::tester::schedule_rom_mooneye(const std::filesystem::path& rom_path,
                                       const schedule_test_t&       schedule)
{
    auto filename     = rom_path.filename().string();
    auto rom_contents = load_rom_file(rom_path);

    auto device_types = parse_decive_types(filename);
    std::for_each(begin(device_types),
                  end(device_types),
                  [&](const auto& dev_type) {
                      schedule({rom_contents, dev_type, run_common_test});
                  });
}
