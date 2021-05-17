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

#include <age_types.hpp>
#include <emulator/age_gb_emulator.hpp>

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>



namespace age::tester
{
    typedef std::function<bool(age::gb_emulator &)> run_test_t;
    typedef std::function<void(const std::shared_ptr<age::uint8_vector> &, age::gb_hardware, age::gb_colors_hint colors_hint, run_test_t)> schedule_test_t;

    void schedule_rom_acid2_cgb(const std::filesystem::path &rom_path, const schedule_test_t &schedule);
    void schedule_rom_acid2_dmg(const std::filesystem::path &rom_path, const schedule_test_t &schedule);
    void schedule_rom_blargg(const std::filesystem::path &rom_path, const schedule_test_t &schedule);
    void schedule_rom_gambatte(const std::filesystem::path &rom_path, const schedule_test_t &schedule);
    void schedule_rom_mooneye_gb(const std::filesystem::path &rom_path, const schedule_test_t &schedule);



    std::shared_ptr<age::uint8_vector> load_rom_file(const std::filesystem::path &rom_path);

    typedef std::function<bool(const age::gb_emulator &)> test_finished_t;

    run_test_t new_screenshot_test(const std::filesystem::path &screenshot_png_path,
                                   const test_finished_t &test_finished);

} // namespace age::tester



#endif // AGE_TESTER_TASK_HPP
