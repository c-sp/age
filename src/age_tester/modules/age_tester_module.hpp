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

#ifndef AGE_TESTER_MODULE_HPP
#define AGE_TESTER_MODULE_HPP

#include "age_tester_test.hpp"

#include <filesystem>
#include <string>
#include <vector>



namespace age::tester
{
    using create_tests_t = std::function<std::vector<age_tester_test>(const std::filesystem::path& rom_path)>;

    class age_tester_module
    {
    public:
        age_tester_module(char           arg_short_name,
                          std::string    arg_long_name,
                          std::string    arg_description,
                          std::string    test_suite_directory,
                          create_tests_t create_tests);

        [[nodiscard]] const std::string& arg_short_name() const;
        [[nodiscard]] const std::string& arg_long_name() const;
        [[nodiscard]] const std::string& arg_description() const;
        [[nodiscard]] const std::string& test_suite_directory() const;
        [[nodiscard]] bool               is_enabled() const;

        std::vector<age_tester_test> create_tests(const std::filesystem::path& rom_path) const;

        void enable_module(bool enabled);

        static bool is_module_enabled(const age_tester_module& module);

    private:
        std::string    m_arg_short_name;
        std::string    m_arg_long_name;
        std::string    m_arg_description;
        std::string    m_test_suite_directory;
        bool           m_is_enabled;
        create_tests_t m_create_tests;
    };



    std::string                        normalize_path_separator(const std::filesystem::path& path);
    std::shared_ptr<age::uint8_vector> load_rom_file(const std::filesystem::path& rom_path);

    age_tester_module create_same_suite_module();

} // namespace age::tester



#endif // AGE_TESTER_MODULE_HPP
