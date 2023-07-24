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

#ifndef AGE_TR_MODULE_HPP
#define AGE_TR_MODULE_HPP

#include "../age_tr_cmd_option.hpp"
#include "age_tr_test.hpp"

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>



namespace age::tr
{
    using create_tests_t = std::function<std::vector<age_tr_test>(const std::filesystem::path& rom_path)>;

    class age_tr_module
    {
    public:
        age_tr_module(char           opt_short_name,
                      std::string    opt_long_name,
                      std::string    opt_description,
                      std::string    test_suite_directory,
                      create_tests_t create_tests);

        age_tr_module(char                     opt_short_name,
                      std::string              opt_long_name,
                      std::string              opt_description,
                      std::vector<std::string> test_suite_directories,
                      create_tests_t           create_tests);

        [[nodiscard]] const age_tr_cmd_option&        cmd_option() const;
        [[nodiscard]] const std::vector<std::string>& test_suite_directories() const;
        [[nodiscard]] std::vector<age_tr_test>        create_tests(const std::filesystem::path& rom_path) const;
        [[nodiscard]] bool                            is_enabled() const;

        void enable_module(bool enabled);

        static bool is_module_enabled(const age_tr_module& module);

    private:
        age_tr_cmd_option        m_cmd_option;
        std::vector<std::string> m_test_suite_directories;
        bool                     m_is_enabled = false;
        create_tests_t           m_create_tests;
    };

    age_tr_module create_acid2_module();
    age_tr_module create_age_module();
    age_tr_module create_blargg_module();
    age_tr_module create_gambatte_module();
    age_tr_module create_gbmicrotest_module();
    age_tr_module create_little_things_module();
    age_tr_module create_mealybug_module();
    age_tr_module create_mooneye_module();
    age_tr_module create_mooneye_wilbertpol_module();
    age_tr_module create_rtc3test_module();
    age_tr_module create_same_suite_module();



    std::string                        normalize_path_separator(const std::filesystem::path& path);
    std::shared_ptr<age::uint8_vector> load_rom_file(const std::filesystem::path& rom_path);

    std::vector<std::filesystem::path> find_screenshots(const std::filesystem::path& rom_path);
    std::filesystem::path              find_screenshot(const std::filesystem::path& rom_path,
                                                       const std::string&           screenshot_suffix);

    std::unordered_set<age::gb_device_type> parse_device_types(const std::string& filename);

} // namespace age::tr



#endif // AGE_TR_MODULE_HPP
