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

#include <git_revision.hpp>

#include "age_tester_arguments.hpp"

#include <algorithm> // std::for_each
#include <iostream>  // std::cout

#include <getopt.h> // getopt_long



namespace
{
    constexpr char opt_acid2        = 'a';
    constexpr char opt_blargg       = 'b';
    constexpr char opt_cgb_only     = 'c';
    constexpr char opt_dmg_only     = 'd';
    constexpr char opt_age          = 'e';
    constexpr char opt_print_failed = 'f';
    constexpr char opt_gambatte     = 'g';
    constexpr char opt_help         = 'h';
    constexpr char opt_write_logs   = 'l';
    constexpr char opt_mooneye_gb   = 'm';
    constexpr char opt_mealybug     = 'n';
    constexpr char opt_print_passed = 'p';
    constexpr char opt_whitelist    = 'w';
    constexpr char opt_blacklist    = 'x';

    constexpr const char* optstring = ":abcdefghlmnpw:x:";

    constexpr const char* opt_long_acid2        = "acid2";
    constexpr const char* opt_long_blargg       = "blargg";
    constexpr const char* opt_long_cgb_only     = "cgb-only";
    constexpr const char* opt_long_dmg_only     = "dmg-only";
    constexpr const char* opt_long_age          = "age";
    constexpr const char* opt_long_print_failed = "print-failed";
    constexpr const char* opt_long_gambatte     = "gambatte";
    constexpr const char* opt_long_help         = "help";
    constexpr const char* opt_long_write_logs   = "write-logs";
    constexpr const char* opt_long_mooneye_gb   = "mooneye-gb";
    constexpr const char* opt_long_mealybug     = "mealybug";
    constexpr const char* opt_long_print_passed = "print-passed";
    constexpr const char* opt_long_whitelist    = "whitelist";
    constexpr const char* opt_long_blacklist    = "blacklist";

    constexpr const char* help_cmd_var = "%cmd%";
    constexpr const char* help_git_var = "%git%";
    constexpr const char* help_lines[] = {
        "Run (parts of) the gameboy-test-roms test suite with AGE.",
        "See also:",
        "  https://github.com/c-sp/gameboy-test-roms",
        "  https://github.com/c-sp/AGE",
        "",
#ifndef AGE_COMPILE_LOGGER
        "This test runner has been compiled without logging support!",
#endif
        "AGE git revision: %git%",
        "",
        "Usage:",
        "  %cmd% [options] [gameboy-test-roms path]",
        "",
        "  If no gameboy-test-roms path is specified,",
        "  the current working directory is used.",
        "",
        "Options:",
        "  -h, --help             print this text",
        "  -l, --write-logs       write test log files",
        "  -f, --print-failed     print failed tests",
        "  -p, --print-passed     print passed tests",
        "  -c, --cgb-only         run only Game Boy Color tests",
        "  -d, --dmg-only         run only Game Boy Classic tests",
        "  -w, --whitelist <...>  whitelist file/regex",
        "  -x, --blacklist <...>  blacklist file/regex",
        "",
        "Use the following category options to run a subset of the test suite.",
        "All tests will run when no category is specified.",
        "Multiple categories can be picked simultaneously.",
        "  -a, --acid2         run cgb-acid-2 and dmg-acid-2 test roms",
        "  -e, --age           run age test roms",
        "  -b, --blargg        run Blarggs test roms",
        "  -g, --gambatte      run Gambatte test roms",
        "  -m, --mooneye-gb    run Mooneye GB test roms",
        "  -n, --mealybug      run Mealybug Tearoom test roms",
    };
} // namespace



void age::tester::print_help(int argc, char** argv)
{
    std::string cmd_var(help_cmd_var);
    std::string git_var(help_git_var);

    std::for_each(std::begin(help_lines),
                  std::end(help_lines),
                  [&](auto line) {
                      std::string l(line);

                      // replace %cmd%
                      auto cmd_var_idx = l.find(cmd_var);
                      if (cmd_var_idx != std::string::npos)
                      {
                          auto cmd = argc >= 1 ? std::filesystem::path(argv[0]).filename().string() : "";
                          l.replace(cmd_var_idx, cmd_var.size(), cmd);
                      }

                      // replace %git%
                      auto git_var_idx = l.find(git_var);
                      if (git_var_idx != std::string::npos)
                      {
                          std::string git_rev{GIT_REV};
                          std::string git_tag{GIT_TAG};
                          std::string git_branch{GIT_BRANCH};


                          std::string branch = !git_tag.empty()      ? git_tag + ", "
                                               : !git_branch.empty() ? git_branch + ", "
                                                                     : "";

                          std::string git = git_rev.empty()
                                                ? "unknown"
                                                : git_rev + " (" + branch + GIT_DATE + ")";

                          l.replace(git_var_idx, git_var.size(), git);
                      }

                      std::cout << l << std::endl;
                  });
}



age::tester::options age::tester::parse_arguments(int argc, char** argv)
{
    // based on
    // https://en.wikipedia.org/wiki/Getopt#Using_GNU_extension_getopt_long

    struct option long_options[] = {
        {
            .name    = opt_long_acid2,
            .has_arg = no_argument,
            .flag    = nullptr,
            .val     = opt_acid2,
        },
        {
            .name    = opt_long_blargg,
            .has_arg = no_argument,
            .flag    = nullptr,
            .val     = opt_blargg,
        },
        {
            .name    = opt_long_cgb_only,
            .has_arg = no_argument,
            .flag    = nullptr,
            .val     = opt_cgb_only,
        },
        {
            .name    = opt_long_dmg_only,
            .has_arg = no_argument,
            .flag    = nullptr,
            .val     = opt_dmg_only,
        },
        {
            .name    = opt_long_age,
            .has_arg = no_argument,
            .flag    = nullptr,
            .val     = opt_age,
        },
        {
            .name    = opt_long_print_failed,
            .has_arg = no_argument,
            .flag    = nullptr,
            .val     = opt_print_failed,
        },
        {
            .name    = opt_long_gambatte,
            .has_arg = no_argument,
            .flag    = nullptr,
            .val     = opt_gambatte,
        },
        {
            .name    = opt_long_help,
            .has_arg = no_argument,
            .flag    = nullptr,
            .val     = opt_help,
        },
        {
            .name    = opt_long_write_logs,
            .has_arg = no_argument,
            .flag    = nullptr,
            .val     = opt_write_logs,
        },
        {
            .name    = opt_long_mooneye_gb,
            .has_arg = no_argument,
            .flag    = nullptr,
            .val     = opt_mooneye_gb,
        },
        {
            .name    = opt_long_mealybug,
            .has_arg = no_argument,
            .flag    = nullptr,
            .val     = opt_mealybug,
        },
        {
            .name    = opt_long_print_passed,
            .has_arg = no_argument,
            .flag    = nullptr,
            .val     = opt_print_passed,
        },
        {
            .name    = opt_long_whitelist,
            .has_arg = required_argument,
            .flag    = nullptr,
            .val     = opt_whitelist,
        },
        {
            .name    = opt_long_blacklist,
            .has_arg = required_argument,
            .flag    = nullptr,
            .val     = opt_blacklist,
        },
        {
            .name    = nullptr,
            .has_arg = 0,
            .flag    = nullptr,
            .val     = 0,
        },
    };

    age::tester::options options{};
    int                  c         = 0;
    int                  longindex = 0;

    // no getopt() error message on standard error
    opterr = 0;

    while ((c = getopt_long(argc, argv, optstring, long_options, &longindex)) != -1)
    {
        switch (c)
        {
            case opt_acid2:
                options.m_acid2 = true;
                break;

            case opt_blargg:
                options.m_blargg = true;
                break;

            case opt_cgb_only:
                options.m_cgb_only = true;
                break;

            case opt_dmg_only:
                options.m_dmg_only = true;
                break;

            case opt_age:
                options.m_age = true;
                break;

            case opt_print_failed:
                options.m_print_failed = true;
                break;

            case opt_gambatte:
                options.m_gambatte = true;
                break;

            case opt_help:
                options.m_help = true;
                break;

            case opt_write_logs:
                options.m_write_logs = true;
                break;

            case opt_mooneye_gb:
                options.m_mooneye_gb = true;
                break;

            case opt_mealybug:
                options.m_mealybug = true;
                break;

            case opt_print_passed:
                options.m_print_passed = true;
                break;

            case opt_whitelist:
                options.m_whitelist = std::string(optarg);
                break;

            case opt_blacklist:
                options.m_blacklist = std::string(optarg);
                break;

            case '?':
                // invalid long option: optopt == 0, use argv[optind] instead
                // see also: https://stackoverflow.com/a/53828745
                if (optopt == 0)
                {
                    options.m_unknown_options.emplace_back(argv[optind - 1]);
                }
                else
                {
                    options.m_unknown_options.emplace_back(std::string(1, static_cast<char>(optopt)));
                }
                break;

            case ':':
                options.m_invalid_arg_options.emplace_back(std::string(1, static_cast<char>(optopt)));
                break;

            default:
                break;
        }
    }

    options.m_test_suite_path = std::filesystem::current_path();
    if (optind < argc)
    {
        options.m_test_suite_path /= argv[optind];
    }

    //! \todo adjust & evaluate -l parameter
    options.m_log_categories = {
        gb_log_category::lc_clock,
        gb_log_category::lc_events,
        gb_log_category::lc_timer,
    };

    return options;
}
