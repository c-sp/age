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

#include <git_revision.hpp>

#include "age_tr_arguments.hpp"
#include "age_tr_cmd_option.hpp"

#include <algorithm> // std::for_each
#include <iostream>  // std::cout

#include <getopt.h>  // getopt_long



namespace
{
    constexpr char opt_cgb_only     = 'c';
    constexpr char opt_dmg_only     = 'd';
    constexpr char opt_print_failed = 'f';
    constexpr char opt_help         = 'h';
    constexpr char opt_write_logs   = 'l';
    constexpr char opt_print_passed = 'p';
    constexpr char opt_whitelist    = 'w';
    constexpr char opt_blacklist    = 'x';

    std::vector<age::tr::age_tr_cmd_option> cmd_options()
    {
        return {
            {opt_help, "help", "print this text"},
            {opt_print_failed, "print-failed", "print failed tests"},
            {opt_print_passed, "print-passed", "print passed tests"},
            {opt_cgb_only, "cgb-only", "run only Game Boy Color (CGB) tests"},
            {opt_dmg_only, "dmg-only", "run only Game Boy Classic (DMG) tests"},
            {opt_whitelist, "whitelist", true, "whitelist file/regex"},
            {opt_blacklist, "blacklist", true, "blacklist file/regex"},
            {opt_write_logs,
             "write-logs",
#ifdef AGE_COMPILE_LOGGER
             "write test log files"
#else
             "write test log files (not supported by this test runner!)"
#endif
            },
        };
    }

    void print_options(const std::vector<age::tr::age_tr_cmd_option>& opts)
    {
        std::vector<age::tr::age_tr_cmd_option> sorted_opts = opts;
        std::sort(sorted_opts.begin(), sorted_opts.end());

        std::size_t col2_chars = 0;
        for (const auto& opt : sorted_opts)
        {
            std::size_t chars = opt.opt_long_name().length()
                                + (opt.argument_required() ? 6 : 0);
            if (chars > col2_chars)
            {
                col2_chars = chars;
            }
        }

        for (const auto& opt : sorted_opts)
        {
            auto col2 = opt.opt_long_name()
                        + (opt.argument_required() ? " <...>" : "");
            col2.resize(col2_chars, ' ');

            std::cout << "  -" << opt.opt_short_name()
                      << ", --" << col2
                      << "  " << opt.opt_description()
                      << std::endl;
        }
    }

} // namespace



void age::tr::print_help(const std::string&                invoked_program,
                         const std::vector<age_tr_module>& modules)
{
    std::cout << "Run (parts of) the gameboy-test-roms test suite with AGE." << std::endl;
    std::cout << "See also:" << std::endl;
    std::cout << "  https://github.com/c-sp/gameboy-test-roms" << std::endl;
    std::cout << "  https://github.com/c-sp/AGE" << std::endl;
    std::cout << std::endl;

#ifndef AGE_COMPILE_LOGGER
    std::cout << "This test runner has been compiled without logging!" << std::endl;
#endif
    std::string git_rev{GIT_REV};
    std::string git_tag{GIT_TAG};
    std::string git_branch{GIT_BRANCH};
    std::string branch = !git_tag.empty()      ? git_tag + ", "
                         : !git_branch.empty() ? git_branch + ", "
                                               : "";
    std::string git    = git_rev.empty()
                             ? "unknown"
                             : git_rev + " (" + branch + GIT_DATE + ")";
    std::cout << "AGE git revision: " << git << std::endl;
    std::cout << std::endl;

    std::cout << "Usage:" << std::endl;
    std::cout << "  " << invoked_program << " [options] [gameboy-test-roms path]" << std::endl;
    std::cout << std::endl;

    std::cout << "  If no gameboy-test-roms path is specified" << std::endl;
    std::cout << "  the current working directory is used." << std::endl;
    std::cout << std::endl;

    std::cout << "Options:" << std::endl;
    print_options(cmd_options());
    std::cout << std::endl;

    std::cout << "Use the following category options to run only a subset of test roms." << std::endl;
    std::cout << "Multiple categories can be picked simultaneously." << std::endl;
    std::cout << "If no category is specified all tests will run." << std::endl;
    std::vector<age_tr_cmd_option> module_options;
    std::transform(begin(modules),
                   end(modules),
                   std::back_inserter(module_options),
                   [](const auto& mod) {
                       return mod.cmd_option();
                   });
    print_options(module_options);
    std::cout << std::endl;
}



age::tr::options age::tr::parse_arguments(const std::vector<char*>&   args,
                                          std::vector<age_tr_module>& modules)
{
    // loosely based on
    // https://en.wikipedia.org/wiki/Getopt#Using_GNU_extension_getopt_long

    // create a list of all options
    std::vector<age_tr_cmd_option> all_cmd_options = cmd_options();
    std::transform(begin(modules),
                   end(modules),
                   std::back_inserter(all_cmd_options),
                   [](const auto& mod) {
                       return mod.cmd_option();
                   });
    std::sort(begin(all_cmd_options), end(all_cmd_options));

    // create option-list for getopt_long()
    std::vector<option> long_options;
    std::transform(begin(all_cmd_options),
                   end(all_cmd_options),
                   std::back_inserter(long_options),
                   [](const auto& opt) {
                       return option{
                           .name    = opt.opt_long_name().c_str(),
                           .has_arg = opt.argument_required() ? required_argument : no_argument,
                           .flag    = nullptr,
                           .val     = opt.opt_short_name(),
                       };
                   });
    long_options.emplace_back(option{
        .name    = nullptr,
        .has_arg = 0,
        .flag    = nullptr,
        .val     = 0,
    });

    // create options-string for getopt_long()
    std::string optstring = ":";
    for (const auto& cmd_opt : all_cmd_options)
    {
        optstring += cmd_opt.opt_short_name();
        if (cmd_opt.argument_required())
        {
            optstring += ":";
        }
    }

    // no getopt_long() error message on standard error
    opterr = 0;

    age::tr::options options;
    int              c         = 0;
    int              longindex = 0;

    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    while ((c = getopt_long(static_cast<int>(args.size()),
                            args.data(),
                            optstring.c_str(),
                            long_options.data(),
                            &longindex))
           != -1)
    {
        // check for module option
        age_tr_module* mod = nullptr;
        for (auto& m : modules)
        {
            if (m.cmd_option().opt_short_name() == c)
            {
                mod = &m;
                break;
            }
        }
        if (mod != nullptr)
        {
            mod->enable_module(true);
            continue;
        }

        // remaining options
        switch (c)
        {
            case opt_cgb_only:
                options.m_cgb_only = true;
                break;

            case opt_dmg_only:
                options.m_dmg_only = true;
                break;

            case opt_print_failed:
                options.m_print_failed = true;
                break;

            case opt_help:
                options.m_help = true;
                break;

            case opt_write_logs:
                options.m_write_logs = true;
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
                    options.m_unknown_options.emplace_back(args[optind - 1]);
                }
                else
                {
                    options.m_unknown_options.emplace_back(1, static_cast<char>(optopt));
                }
                break;

            case ':':
                options.m_invalid_arg_options.emplace_back(1, static_cast<char>(optopt));
                break;

            default:
                break;
        }
    }

    options.m_test_suite_path = std::filesystem::current_path();
    if (optind < args.size())
    {
        options.m_test_suite_path /= args[optind];
    }

    //! \todo adjust & evaluate -l parameter
    options.m_log_categories = {
        gb_log_category::lc_clock,
        gb_log_category::lc_cpu,
        // gb_log_category::lc_events,
        gb_log_category::lc_hdma,
        gb_log_category::lc_interrupts,
        gb_log_category::lc_lcd,
        gb_log_category::lc_lcd_oam,
        gb_log_category::lc_lcd_oam_dma,
        gb_log_category::lc_lcd_registers,
        gb_log_category::lc_lcd_vram,
        gb_log_category::lc_memory,
        gb_log_category::lc_serial,
        gb_log_category::lc_sound,
        gb_log_category::lc_sound_registers,
        gb_log_category::lc_timer,
    };

    return options;
}
