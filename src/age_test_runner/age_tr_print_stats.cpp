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

#include "age_tr_print.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>



namespace
{
    void print_cycles_per_second(const age::tr::age_tr_test_run_results& test_run_results)
    {
        if (test_run_results.m_test_results.empty())
        {
            return;
        }

        double  cps_mean = 0;
        int64_t cps_min  = std::numeric_limits<int64_t>::max();
        int64_t cps_max  = std::numeric_limits<int64_t>::min();
        for (const auto& tr : test_run_results.m_test_results)
        {
            cps_mean += static_cast<double>(tr.m_cycles_per_second);
            cps_min = std::min(cps_min, tr.m_cycles_per_second);
            cps_max = std::max(cps_max, tr.m_cycles_per_second);
        }
        cps_mean /= static_cast<double>(test_run_results.m_test_results.size());

        double sd2 = 0;
        for (const auto& tr : test_run_results.m_test_results)
        {
            auto diff = static_cast<double>(tr.m_cycles_per_second) - cps_mean;
            sd2 += diff * diff;
        }
        sd2 /= static_cast<double>(test_run_results.m_test_results.size());
        auto sd = std::sqrt(sd2);

        std::cout << "emulated cycles per second: " << static_cast<int64_t>(cps_mean)
                  << " (sd " << static_cast<int64_t>(sd)
                  << ", min " << cps_min
                  << ", max " << cps_max
                  << ")" << std::endl;
    }

    void print_cps_table(std::vector<age::tr::age_tr_test_result>::const_iterator begin,
                         std::vector<age::tr::age_tr_test_result>::const_iterator end)
    {

        std::size_t col1_size = 0;
        std::size_t col2_size = 0;
        for (auto b = begin; b != end; b++)
        {
            auto col1 = b->m_test_name;
            auto col2 = std::to_string(b->m_cycles_per_second);
            col1_size = std::max(col1_size, col1.length());
            col2_size = std::max(col2_size, col2.length());
        }

        for (auto b = begin; b != end; b++)
        {
            std::string col1 = b->m_test_name;
            col1.resize(col1_size, ' ');

            std::string col2 = std::to_string(b->m_cycles_per_second);
            std::string col2_pad;
            col2_pad.resize(col2_size - col2.length(), ' ');

            std::cout << "  " << col1 << "  " << col2_pad << col2 << std::endl;
        }
    }

    void print_slowest_fastest_tests(const age::tr::age_tr_test_run_results& test_run_results)
    {
        auto test_results = test_run_results.m_test_results;
        std::sort(begin(test_results),
                  end(test_results),
                  [](const auto& lhs, const auto& rhs) {
                      return lhs.m_cycles_per_second > rhs.m_cycles_per_second;
                  });

        constexpr std::size_t print_count = 5;
        if (test_results.size() <= print_count)
        {
            std::cout << "cycles per second by test:" << std::endl;
            print_cps_table(begin(test_results), end(test_results));
            return;
        }

        auto highest_count = test_results.size() >= 2 * print_count
                                 ? print_count
                                 : print_count - test_results.size() / 2;
        std::cout << "highest cycles per second:" << std::endl;
        print_cps_table(begin(test_results), begin(test_results) + static_cast<int64_t>(highest_count));

        auto lowest_count = test_results.size() >= 2 * print_count
                                ? print_count
                                : test_results.size() / 2;
        std::cout << "lowest cycles per second:" << std::endl;
        print_cps_table(end(test_results) - static_cast<int64_t>(lowest_count), end(test_results));
    }

} // namespace



void age::tr::print_stats(const age::tr::age_tr_test_run_results& test_run_results)
{
    std::chrono::duration<double> total_duration_sec = test_run_results.m_total_duration;
    std::cout << "finished test run after "
              << total_duration_sec.count() << " seconds"
              << std::endl;

    print_cycles_per_second(test_run_results);
    print_slowest_fastest_tests(test_run_results);
}
