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

#include "age_tr_run_tests.hpp"
#include "age_tr_thread_pool.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <queue>
#include <ranges>
#include <regex>



namespace
{
    std::vector<std::string> read_list_file(const std::filesystem::path& file_path)
    {
        std::vector<std::string> result;

        std::ifstream ifs(file_path);
        std::string   line;
        while (std::getline(ifs, line))
        {
            // strip comment
            line = line.substr(0, line.find('#'));

            // trim left & right
            auto is_space = [](auto c) {
                return !std::isspace(c);
            };

            line.erase(begin(line), std::find_if(begin(line), end(line), is_space));
            line.erase(std::find_if(rbegin(line), rend(line), is_space).base(), end(line));

            // discard empty lines
            if (!line.empty())
            {
                result.emplace_back(line);
            }
        }

        return result;
    }



    using path_matcher = std::function<bool(const std::string&)>;

    bool match_everything([[maybe_unused]] const std::string& value)
    {
        return true;
    }

    bool match_nothing([[maybe_unused]] const std::string& value)
    {
        return false;
    }

    path_matcher new_regex_matcher(const std::string& regex_string)
    {
        std::regex regex(regex_string, std::regex_constants::ECMAScript);

        return [=](const std::string& absolute_path) {
            return std::regex_search(absolute_path, regex);
        };
    }

    path_matcher new_list_matcher(const std::filesystem::path& file)
    {
        auto regex_strings = read_list_file(file);

        std::vector<path_matcher> matcher_list;
        std::transform(begin(regex_strings), end(regex_strings), std::back_inserter(matcher_list), [](const std::string& s) {
            return [=](const std::string& absolute_path) {
                return absolute_path.find(s) != std::string::npos;
            };
        });

        return [=](const std::string& absolute_path) {
            return std::any_of(begin(matcher_list), end(matcher_list), [&](auto& m) {
                return m(absolute_path);
            });
        };
    }

    path_matcher new_matcher(const std::string& file_or_regex)
    {
        auto possible_file = std::filesystem::current_path() / file_or_regex;
        return std::filesystem::is_regular_file(possible_file)
                   ? new_list_matcher(possible_file)
                   : new_regex_matcher(file_or_regex);
    }



    void traverse_directory(const std::filesystem::path&                             path,
                            const std::function<void(const std::filesystem::path&)>& file_callback)
    {
        std::queue<std::filesystem::path> directories;
        if (std::filesystem::is_directory(path))
        {
            directories.push(path);
        }

        while (!directories.empty())
        {
            auto dir = directories.front();
            directories.pop();

            for (const auto& entry : std::filesystem::directory_iterator(dir))
            {
                if (entry.is_directory())
                {
                    directories.push(entry.path());
                }
                else if (entry.is_regular_file())
                {
                    file_callback(entry.path());
                }
            }
        }
    }

    void find_roms(const std::filesystem::path&                             path,
                   const path_matcher&                                      use_file,
                   const std::function<void(const std::filesystem::path&)>& file_callback)
    {
        const std::array<std::string, 3> rom_file_extensions{".gb", ".gbc", ".cgb"};
        const auto*                      ext_end = end(rom_file_extensions);

        traverse_directory(path, [&](const std::filesystem::path& file_path) {
            auto file_extension = file_path.extension().string();

            if (std::find(begin(rom_file_extensions), ext_end, file_extension) == ext_end)
            {
                return;
            }
            std::string path = age::tr::normalize_path_separator(file_path.string());
            if (use_file(path))
            {
                file_callback(file_path);
            }
        });
    }

} // namespace



std::vector<age::tr::test_result> age::tr::run_tests(const options&                    opts,
                                                     const std::vector<age_tr_module>& modules,
                                                     unsigned                          threads)
{
    auto whitelist = opts.m_whitelist.length() ? new_matcher(opts.m_whitelist) : match_everything;
    auto blacklist = opts.m_blacklist.length() ? new_matcher(opts.m_blacklist) : match_nothing;

    auto matcher = [&](const std::string& path) {
        return whitelist(path) && !blacklist(path);
    };

    blocking_vector<test_result> results;
    int                          rom_count = 0;
    {
        auto        last_update = std::chrono::system_clock::now();
        thread_pool pool(threads, [&](size_t task_queue_size, size_t tasks_running, size_t tasks_finished) {
            auto                          now   = std::chrono::system_clock::now();
            std::chrono::duration<double> diff  = now - last_update;
            auto                          total = task_queue_size + tasks_running + tasks_finished;
            if ((diff.count() > 0.3) || (total == tasks_finished))
            {
                last_update = now;
                std::cout << "\rfinished " << tasks_finished << " of " << total << " test(s)" << std::flush;
                if (total == tasks_finished)
                {
                    std::cout << std::endl;
                }
            }
        });

        for (const auto& mod : modules | std::views::filter(age_tr_module::is_module_enabled))
        {
            for (const auto& directory : mod.test_suite_directories())
            {
                find_roms(opts.m_test_suite_path / directory,
                          matcher,
                          [&](const std::filesystem::path& rom_path) {
                              ++rom_count;
                              std::vector<age_tr_test> tests = mod.create_tests(rom_path);

                              // we don't allow auto_detect tests, the device type
                              // must have been explicitly specified
                              erase_if(tests, [](const age_tr_test& test) {
                                  return test.device_type() == gb_device_type::auto_detect;
                              });

                              // no tests to run -> mark this as failed test
                              if (tests.empty())
                              {
                                  results.push({rom_path.string() + " (no test scheduled)", false});
                                  return;
                              }

                              // silently ignore DMG/GBC tests, if requested
                              erase_if(tests, [&](const age_tr_test& test) {
                                  switch (test.device_type())
                                  {
                                      case gb_device_type::auto_detect:
                                          break;

                                      case gb_device_type::dmg:
                                          return opts.m_cgb_only;

                                      case gb_device_type::cgb_abcd:
                                      case gb_device_type::cgb_e:
                                          return opts.m_dmg_only;
                                  }
                                  return false;
                              });

                              // schedule tests for this rom file
                              for (auto& test : tests)
                              {
                                  pool.queue_task([test, &opts, &results]() mutable {
                                      auto begin = std::chrono::high_resolution_clock::now();

                                      test.init_test(opts.m_log_categories);
                                      test.run_test();
                                      auto passed = test.test_succeeded();

                                      auto end = std::chrono::high_resolution_clock::now();
                                      results.push({test.test_name(), passed});
                                  });
                              }
                          });
            }
        }

        // by letting the thread pool go out of scope we wait for it to finish
    }
    // wait for "finished X of Y tests" logs to finish before logging this
    std::cout << "tests were created from " << rom_count << " rom(s)" << std::endl;

    return results.copy();
}
