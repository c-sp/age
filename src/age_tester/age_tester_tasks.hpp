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

#ifndef AGE_TESTER_TASK_HPP
#define AGE_TESTER_TASK_HPP

#include "age_tester_arguments.hpp"

#include <age_types.hpp>
#include <emulator/age_gb_emulator.hpp>

#include <filesystem>
#include <memory> // shared_ptr
#include <string>



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



    typedef std::function<bool(age::gb_emulator&)> run_test_t;
    typedef std::function<void(std::shared_ptr<age::uint8_vector>, age::gb_hardware, run_test_t)> schedule_test_t;

    bool schedule_rom_gambatte(const std::filesystem::path &rom_path, const schedule_test_t &schedule);
    bool schedule_rom_mooneye_gb(const std::filesystem::path &rom_path, const schedule_test_t &schedule);

    std::shared_ptr<age::uint8_vector> load_rom_file(const std::filesystem::path &rom_path);

} // namespace age::tester



#endif // AGE_TESTER_TASK_HPP
