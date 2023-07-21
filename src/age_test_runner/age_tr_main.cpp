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

#include "age_tr_arguments.hpp"
#include "age_tr_run_tests.hpp"
#include "modules/age_tr_module.hpp"

#include <algorithm>
#include <iostream>
#include <thread>



namespace
{
    void check_run_all(std::vector<age::tr::age_tr_module>& modules)
    {
        // any module enabled -> no change
        auto any_enabled = std::find_if(begin(modules),
                                        end(modules),
                                        [](const auto& mod) {
                                            return mod.is_enabled();
                                        });
        if (any_enabled != end(modules))
        {
            return;
        }

        // no module enabled -> enable all modules
        for (auto& mod : modules)
        {
            mod.enable_module(true);
        }
    }

    void sort_and_print(std::vector<age::tr::test_result>& tests)
    {
        std::vector<std::string> test_names;

        std::transform(begin(tests),
                       end(tests),
                       std::back_inserter(test_names),
                       [](auto& res) {
                           return res.m_test_name;
                       });

        std::sort(begin(test_names), end(test_names));

        std::for_each(begin(test_names),
                      end(test_names),
                      [](auto& name) {
                          std::cout << "    " << name << std::endl;
                      });
    }

    std::string invoked_program(const std::vector<char*> args)
    {
        return args.empty() ? "" : std::filesystem::path(args[0]).filename().string();
    }

    void print_args_error(std::string                                invoked_program,
                          const age::tr::options&                    opts,
                          const std::vector<age::tr::age_tr_module>& modules)
    {
        std::for_each(begin(opts.m_unknown_options),
                      end(opts.m_unknown_options),
                      [&](auto& opt) {
                          std::cout << "unknown option: " << opt << std::endl;
                      });

        std::for_each(begin(opts.m_invalid_arg_options),
                      end(opts.m_invalid_arg_options),
                      [&](auto& opt) {
                          std::cout << "missing or invalid argument for option: " << opt << std::endl;
                      });

        std::cout << std::endl;
        age::tr::print_help(invoked_program, modules);
    }

    void print_test_categories(const std::vector<age::tr::age_tr_module>& modules)
    {
        std::vector<std::string> categories;
        for (const auto& mod : modules)
        {
            if (mod.is_enabled())
            {
                for (const auto& test_dir : mod.test_suite_directories())
                {
                    categories.push_back(test_dir);
                }
            }
        }
        std::sort(begin(categories), end(categories));

        std::cout << "test categories:";
        for (const auto& cat : categories)
        {
            std::cout << " " << cat;
        }
        std::cout << std::endl;
    }

} // namespace



int main(int argc, char** argv)
{
    std::vector<age::tr::age_tr_module> modules{
        age::tr::create_acid2_module(),
        age::tr::create_age_module(),
        age::tr::create_blargg_module(),
        age::tr::create_gambatte_module(),
        age::tr::create_little_things_module(),
        age::tr::create_mealybug_module(),
        age::tr::create_mooneye_module(),
        age::tr::create_mooneye_wilbertpol_module(),
        age::tr::create_rtc3test_module(),
        age::tr::create_same_suite_module()};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::vector<char*> args(argv, argv + argc);
    age::tr::options   opts = age::tr::parse_arguments(args, modules);

    check_run_all(modules);

    // just print the help text
    if (opts.m_help)
    {
        age::tr::print_help(invoked_program(args), modules);
        return 0;
    }

    // terminate with error on unknown/invalid argument
    if (!opts.m_unknown_options.empty() || !opts.m_invalid_arg_options.empty())
    {
        print_args_error(invoked_program(args), opts, modules);
        return 1;
    }

    // notify the user about what we're going to do
    print_test_categories(modules);
    std::cout << "looking for test roms in: " << opts.m_test_suite_path.string()
              << std::endl;

#ifndef AGE_COMPILE_LOGGER
    opts.m_log_categories = {};
#endif
    if (opts.m_log_categories.empty())
    {
        std::cout << "test rom logs disabled" << std::endl;
    }
    else
    {
        //! \todo print log categories
    }

    if (!opts.m_whitelist.empty())
    {
        std::cout << "whitelist: " << opts.m_whitelist << std::endl;
    }
    if (!opts.m_blacklist.empty())
    {
        std::cout << "blacklist: " << opts.m_blacklist << std::endl;
    }

    // check hardware concurrency
    unsigned threads = std::max(4U, std::thread::hardware_concurrency());
    std::cout << "using " << threads << " threads to run tests" << std::endl;

    // find test rom files & schedule test tasks
    auto results = age::tr::run_tests(opts, modules, threads);
    if (results.empty())
    {
        std::cout << "no tests found!" << std::endl;
        return 1;
    }

    std::vector<age::tr::test_result> passed_tests;
    std::copy_if(begin(results),
                 end(results),
                 std::back_inserter(passed_tests),
                 [](auto& res) {
                     return res.m_test_passed;
                 });

    std::cout << passed_tests.size() << " test(s) passed" << std::endl;
    if (opts.m_print_passed)
    {
        sort_and_print(passed_tests);
    }

    std::vector<age::tr::test_result> failed_tests;
    std::copy_if(begin(results),
                 end(results),
                 std::back_inserter(failed_tests),
                 [](auto& res) {
                     return !res.m_test_passed;
                 });

    std::cout << failed_tests.size() << " test(s) failed" << std::endl;
    if (opts.m_print_failed)
    {
        sort_and_print(failed_tests);
    }

    return failed_tests.empty() ? 0 : 1;
}
