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

#ifndef AGE_TR_ARGUMENTS_HPP
#define AGE_TR_ARGUMENTS_HPP

#include "modules/age_tr_module.hpp"

#include <emulator/age_gb_types.hpp>

#include <filesystem>
#include <string>
#include <vector>



namespace age::tr
{
    struct options
    {
        bool m_cgb_only = false;
        bool m_dmg_only = false;

        bool m_help         = false;
        bool m_write_logs   = false;
        bool m_print_passed = false;
        bool m_print_failed = false;

        //!
        //! path to the gameboy-test-roms test suite,
        //! see also: https://github.com/c-sp/gameboy-test-roms
        //!
        std::filesystem::path m_test_suite_path = {};

        gb_log_categories m_log_categories = {};
        std::string       m_whitelist      = {};
        std::string       m_blacklist      = {};

        std::vector<std::string> m_unknown_options     = {};
        std::vector<std::string> m_invalid_arg_options = {};
    };


    void print_help(const std::string&                invoked_program,
                    const std::vector<age_tr_module>& modules);

    options parse_arguments(const std::vector<char*>&   args,
                            std::vector<age_tr_module>& modules);

} // namespace age::tr



#endif // AGE_TR_ARGUMENTS_HPP
