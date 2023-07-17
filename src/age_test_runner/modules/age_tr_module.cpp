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

#include <cassert>
#include <fstream>
#include <iostream>
#include <utility>



age::tr::age_tr_module::age_tr_module(char           arg_short_name,
                                      std::string    arg_long_name,
                                      std::string    arg_description,
                                      std::string    test_suite_directory,
                                      create_tests_t create_tests)
    : m_arg_short_name(std::string("") + arg_short_name),
      m_arg_long_name(std::move(arg_long_name)),
      m_arg_description(std::move(arg_description)),
      m_test_suite_directory(std::move(test_suite_directory)),
      m_create_tests(std::move(create_tests))
{
    assert(m_arg_short_name.length() == 1);
    assert(m_arg_long_name.length() > 1);
    assert(!m_arg_description.empty());
}



const std::string& age::tr::age_tr_module::arg_short_name() const
{
    return m_arg_short_name;
}

const std::string& age::tr::age_tr_module::arg_long_name() const
{
    return m_arg_long_name;
}

const std::string& age::tr::age_tr_module::arg_description() const
{
    return m_arg_description;
}

const std::string& age::tr::age_tr_module::test_suite_directory() const
{
    return m_test_suite_directory;
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
    return module.is_enabled();
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
