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

#ifndef AGE_TESTER_RUN_TESTS_HPP
#define AGE_TESTER_RUN_TESTS_HPP

#include "age_tester_arguments.hpp"

#include <string>
#include <vector>



namespace age::tester
{
    struct test_result
    {
        std::string m_test_name;
        bool m_test_passed;

        test_result(std::string test_name, bool test_passed)
            : m_test_name(std::move(test_name)),
              m_test_passed(test_passed)
        {}
    };

    std::vector<test_result> run_tests(const options &opts);

} // namespace age::tester



#endif // AGE_TESTER_RUN_TESTS_HPP
