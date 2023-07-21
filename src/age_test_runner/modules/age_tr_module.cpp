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

#include <fstream>
#include <iostream>
#include <optional>
#include <utility>



namespace
{
    std::unordered_set<age::gb_device_type> parse_device_groups(const std::string& filename, size_t idx)
    {
        std::unordered_set<age::gb_device_type> result;

        for (; idx < filename.size(); ++idx)
        {
            auto c = filename.at(idx);
            if (c == 'G')
            {
                result.insert(age::gb_device_type::dmg);
            }
            else if (c == 'C')
            {
                result.insert(age::gb_device_type::cgb_abcd);
                result.insert(age::gb_device_type::cgb_e);
            }
            else if (c != 'A' && c != 'S')
            {
                break;
            }
        }
        return result;
    }

    void parse_dmg_types(const std::string& filename, size_t idx, std::unordered_set<age::gb_device_type>& device_types)
    {
        size_t i = idx;
        for (; i < filename.size(); ++i)
        {
            auto c = filename.at(i);
            if (c == 'A' || c == 'B' || c == 'C')
            {
                device_types.insert(age::gb_device_type::dmg);
            }
            else if (c != '0')
            {
                break;
            }
        }

        // if this test is not constrained to any DMG type,
        // run it for all DMG types
        // (we don't distinguish between DMG types at the moment)
        if (i == idx)
        {
            device_types.insert(age::gb_device_type::dmg);
        }
    }

    void parse_cgb_types(const std::string& filename, size_t idx, std::unordered_set<age::gb_device_type>& device_types)
    {
        size_t i = idx;
        for (; i < filename.size(); ++i)
        {
            auto c = filename.at(i);
            if (c == 'A' || c == 'B' || c == 'C' || c == 'D')
            {
                device_types.insert(age::gb_device_type::cgb_abcd);
            }
            else if (c == 'E')
            {
                device_types.insert(age::gb_device_type::cgb_e);
            }
            else if (c != '0')
            {
                break;
            }
        }

        // if this test is not constrained to any CGB type,
        // run it for all CGB types
        if (i == idx)
        {
            device_types.insert(age::gb_device_type::cgb_abcd);
            device_types.insert(age::gb_device_type::cgb_e);
        }
    }

} // namespace



age::tr::age_tr_module::age_tr_module(char           arg_short_name,
                                      std::string    arg_long_name,
                                      std::string    arg_description,
                                      std::string    test_suite_directory,
                                      create_tests_t create_tests)
    : age_tr_module(arg_short_name,
                    std::move(arg_long_name),
                    std::move(arg_description),
                    std::vector{std::move(test_suite_directory)},
                    std::move(create_tests))
{
}

age::tr::age_tr_module::age_tr_module(char                     arg_short_name,
                                      std::string              arg_long_name,
                                      std::string              arg_description,
                                      std::vector<std::string> test_suite_directories,
                                      age::tr::create_tests_t  create_tests)
    : m_cmd_option(arg_short_name, std::move(arg_long_name), std::move(arg_description)),
      m_test_suite_directories(std::move(test_suite_directories)),
      m_create_tests(std::move(create_tests))
{
}


const age::tr::age_tr_cmd_option& age::tr::age_tr_module::cmd_option() const
{
    return m_cmd_option;
}

const std::vector<std::string>& age::tr::age_tr_module::test_suite_directories() const
{
    return m_test_suite_directories;
}

bool age::tr::age_tr_module::is_enabled() const
{
    return m_is_enabled;
}

std::vector<age::tr::age_tr_test> age::tr::age_tr_module::create_tests(const std::filesystem::path& rom_path) const
{
    return m_create_tests(rom_path);
}

void age::tr::age_tr_module::enable_module(bool enabled)
{
    m_is_enabled = enabled;
}

bool age::tr::age_tr_module::is_module_enabled(const age::tr::age_tr_module& module)
{
    return module.m_is_enabled;
}



std::string age::tr::normalize_path_separator(const std::filesystem::path& path)
{
    if constexpr (std::filesystem::path::preferred_separator == '/')
    {
        return path;
    }
    std::string result = path;
    std::replace(begin(result), end(result), static_cast<char>(std::filesystem::path::preferred_separator), '/');
    return result;
}

std::shared_ptr<age::uint8_vector> age::tr::load_rom_file(const std::filesystem::path& rom_path)
{
    std::ifstream rom_file(rom_path, std::ios::in | std::ios::binary);
    return std::make_shared<uint8_vector>(
        std::istreambuf_iterator<char>(rom_file),
        std::istreambuf_iterator<char>());
}



std::vector<std::filesystem::path> age::tr::find_screenshots(const std::filesystem::path& rom_path)
{
    // mooneye test suite examples:
    //  - sprite_priority.gb
    //  - sprite_priority-cgb.png
    //  - sprite_priority-dmg.png
    //
    // age-test-roms examples:
    //  - m3-bg-scx-ds.gb
    //  - m3-bg-scx-nocgb.gb
    //  - m3-bg-scx.gb
    //  - m3-bg-scx-cgbBCE.png
    //  - m3-bg-scx-dmgC.png
    //  - m3-bg-scx-ds-cgbBCE.png
    //  - m3-bg-scx-nocgb-ncmBCE.png
    //
    std::vector<std::filesystem::path> screenshots;

    auto rom_path_no_ext = std::filesystem::path{rom_path}.replace_extension().string(); // discard file extension

    for (const auto& entry : std::filesystem::directory_iterator{rom_path.parent_path()})
    {
        if (!entry.is_regular_file() || (entry.path().extension().string() != ".png"))
        {
            continue;
        }

        auto path     = entry.path();
        auto path_str = path.string();
        if (path_str.ends_with("_actual.png"))
        {
            continue;
        }

        if (path_str.starts_with(rom_path_no_ext + "-dmg")
            || path_str.starts_with(rom_path_no_ext + "-cgb")
            || path_str.starts_with(rom_path_no_ext + "-ncm"))
        {
            screenshots.emplace_back(path);
        }
    }

    return screenshots;
}

std::filesystem::path age::tr::find_screenshot(const std::filesystem::path& rom_path,
                                               const std::string&           screenshot_suffix)
{
    auto base = rom_path;
    base.replace_extension(); // remove file extension

    auto screenshot = base;
    screenshot += screenshot_suffix;
    if (std::filesystem::is_regular_file(screenshot))
    {
        return screenshot;
    }

    return {};
}



std::unordered_set<age::gb_device_type> age::tr::parse_device_types(const std::string& filename)
{
    std::unordered_set<age::gb_device_type> result;

    // test runs on specific CGB types
    std::string cgb_prefix("-cgb");
    auto        cgb_hint = filename.find(cgb_prefix);
    if (cgb_hint != std::string::npos)
    {
        parse_cgb_types(filename, cgb_hint + cgb_prefix.size(), result);
    }
    std::string ncm_prefix("-ncm");
    auto        ncm_hint = filename.find(ncm_prefix);
    if (ncm_hint != std::string::npos)
    {
        parse_cgb_types(filename, ncm_hint + ncm_prefix.size(), result);
    }

    // test runs on specific DMG types
    std::string dmg_prefix("-dmg");
    auto        dmg_hint = filename.find(dmg_prefix);
    if (dmg_hint != std::string::npos)
    {
        parse_dmg_types(filename, dmg_hint + dmg_prefix.size(), result);
    }

    // test runs on all CGB types
    if (filename.find("-C") != std::string::npos)
    {
        result.insert(age::gb_device_type::cgb_abcd);
        result.insert(age::gb_device_type::cgb_e);
    }

    // test runs on all DMG types
    // (we don't distinguish between DMG types at the moment)
    if (filename.find("-G") != std::string::npos)
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
