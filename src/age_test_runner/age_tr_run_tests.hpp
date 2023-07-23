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

#ifndef AGE_TR_RUN_TESTS_HPP
#define AGE_TR_RUN_TESTS_HPP

#include "age_tr_arguments.hpp"
#include "modules/age_tr_module.hpp"

#include <chrono>
#include <string>
#include <vector>



namespace age::tr
{
    struct test_result
    {
        bool                     m_test_passed;
        std::string              m_test_name;
        std::chrono::nanoseconds m_init_duration;
        std::chrono::nanoseconds m_run_duration;
        std::chrono::nanoseconds m_evaluation_duration;
        std::chrono::nanoseconds m_write_logs_duration;
        int64_t                  m_emulated_cycles;
        double                   m_cycles_per_second;
    };

    struct test_run_results
    {
        std::vector<test_result> m_test_results;
        int                      m_rom_count = 0;
        std::chrono::nanoseconds m_total_duration;
    };

    test_run_results run_tests(const options&                    opts,
                               const std::vector<age_tr_module>& modules,
                               unsigned                          threads);

} // namespace age::tr



#endif // AGE_TR_RUN_TESTS_HPP
