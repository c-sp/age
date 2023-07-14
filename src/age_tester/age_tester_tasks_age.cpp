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

    std::optional<age::gb_device_type> parse_dmg_type(char c)
    {
        if (c == 'C')
        {
            return {age::gb_device_type::dmg};
        }
        return {};
    }

    void for_each_device_type(const std::filesystem::path&                                   file_path,
                              const std::string&                                             type_prefix,
                              const std::function<std::optional<age::gb_device_type>(char)>& parse_type,
                              const std::function<void(age::gb_device_type)>&                callback)
    {
        auto filename  = file_path.filename().string();
        auto type_hint = filename.find(type_prefix);
        if (type_hint != std::string::npos)
        {
            type_hint += type_prefix.size();

            for (; type_hint < filename.size(); ++type_hint)
            {
                auto dev_type = parse_type(filename.at(type_hint));
                if (!dev_type.has_value())
                {
                    break;
                }
                callback(dev_type.value());
            }
        }
    }



    constexpr const char* prefix_dmg = "-dmg";
    constexpr const char* prefix_cgb = "-cgb";
    constexpr const char* prefix_ncm = "-ncm";

    std::vector<std::string> find_screenshots(const std::filesystem::path& rom_path)
    {
        std::vector<std::string> screenshots;

        auto rom_path_no_ext = std::filesystem::path{rom_path}.replace_extension().string(); // discard file extension
        auto path_dmg        = rom_path_no_ext + prefix_dmg;
        auto path_cgb        = rom_path_no_ext + prefix_cgb;
        auto path_ncm        = rom_path_no_ext + prefix_ncm;

        for (const auto& entry : std::filesystem::directory_iterator{rom_path.parent_path()})
        {
            if (!entry.is_regular_file() || (entry.path().extension().string() != ".png"))
            {
                continue;
            }

            auto path = entry.path().string();
            if ((path.find(path_dmg) == 0) || (path.find(path_cgb) == 0) || (path.find(path_ncm) == 0))
            {
                screenshots.emplace_back(path);
            }
        }

        return screenshots;
    }

} // namespace



void age::tester::schedule_rom_age(const std::filesystem::path& rom_path,
                                   const schedule_test_t&       schedule)
{
    auto filename     = rom_path.filename().string();
    auto rom_contents = load_rom_file(rom_path);

    // schedule "regular" tests

    for_each_device_type(rom_path, prefix_dmg, parse_dmg_type, [&](gb_device_type type) {
        schedule({rom_contents, type, run_common_test});
    });
    for_each_device_type(rom_path, prefix_cgb, parse_cgb_type, [&](gb_device_type type) {
        schedule({rom_contents, type, run_common_test});
    });
    for_each_device_type(rom_path, prefix_ncm, parse_cgb_type, [&](gb_device_type type) {
        schedule({rom_contents, type, run_common_test});
    });

    // schedule screenshot tests

    auto screenshots = find_screenshots(rom_path);
    std::for_each(begin(screenshots), end(screenshots), [&](const auto& screenshot_path) {
        for_each_device_type(screenshot_path, prefix_dmg, parse_dmg_type, [&](gb_device_type type) {
            schedule({rom_contents,
                      type,
                      new_screenshot_test(screenshot_path, run_until(has_executed_ld_b_b))});
        });

        for_each_device_type(screenshot_path, prefix_cgb, parse_cgb_type, [&](gb_device_type type) {
            schedule({rom_contents,
                      type,
                      new_screenshot_test(screenshot_path, run_until(has_executed_ld_b_b))});
        });

        for_each_device_type(screenshot_path, prefix_ncm, parse_cgb_type, [&](gb_device_type type) {
            schedule({rom_contents,
                      type,
                      new_screenshot_test(screenshot_path, run_until(has_executed_ld_b_b))});
        });
    });
}
