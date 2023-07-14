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
    using run_test_t = std::function<bool(age::gb_emulator&)>;

    bool run_common_test(age::gb_emulator& emulator);



    bool has_executed_ld_b_b(const age::gb_emulator& emulator);

    run_test_t run_until(const std::function<bool(const age::gb_emulator&)>& test_finished);

    run_test_t new_screenshot_test(const std::filesystem::path& screenshot_png_path,
                                   const run_test_t&            run_test);



    struct schedule_test_opts
    {
        schedule_test_opts(std::shared_ptr<const uint8_vector> rom,
                           gb_device_type                      device_type,
                           run_test_t                          run_test)
            : schedule_test_opts(std::move(rom), device_type, std::move(run_test), "") {}

        schedule_test_opts(std::shared_ptr<const uint8_vector> rom,
                           gb_device_type                      device_type,
                           run_test_t                          run_test,
                           std::string                         info)
            : schedule_test_opts(std::move(rom),
                                 device_type,
                                 device_type == gb_device_type::dmg
                                     ? gb_colors_hint::dmg_greyscale
                                     : gb_colors_hint::cgb_acid2,
                                 std::move(run_test),
                                 std::move(info)) {}

        schedule_test_opts(std::shared_ptr<const uint8_vector> rom,
                           gb_device_type                      device_type,
                           gb_colors_hint                      colors_hint,
                           run_test_t                          run_test,
                           std::string                         info = "")
            : m_rom(std::move(rom)),
              m_device_type(device_type),
              m_colors_hint(colors_hint),
              m_run_test(std::move(run_test)),
              m_info(std::move(info)) {}

        std::shared_ptr<const uint8_vector> m_rom;
        gb_device_type                      m_device_type;
        gb_colors_hint                      m_colors_hint;
        run_test_t                          m_run_test;
        std::string                         m_info;
    };

    using schedule_test_t = std::function<void(schedule_test_opts)>;

    void schedule_rom_acid2_cgb(const std::filesystem::path& rom_path, const schedule_test_t& schedule);
    void schedule_rom_acid2_dmg(const std::filesystem::path& rom_path, const schedule_test_t& schedule);
    void schedule_rom_age(const std::filesystem::path& rom_path, const schedule_test_t& schedule);
    void schedule_rom_blargg(const std::filesystem::path& rom_path, const schedule_test_t& schedule);
    void schedule_rom_gambatte(const std::filesystem::path& rom_path, const schedule_test_t& schedule);
    void schedule_rom_little_things(const std::filesystem::path& rom_path, const schedule_test_t& schedule);
    void schedule_rom_mealybug(const std::filesystem::path& rom_path, const schedule_test_t& schedule);
    void schedule_rom_mooneye(const std::filesystem::path& rom_path, const schedule_test_t& schedule);
    void schedule_rom_mooneye_wilbertpol(const std::filesystem::path& rom_path, const schedule_test_t& schedule);
    void schedule_rom_rtc3test(const std::filesystem::path& rom_path, const schedule_test_t& schedule);
    void schedule_rom_same_suite(const std::filesystem::path& rom_path, const schedule_test_t& schedule);



    std::string                        normalize_path_separator(const std::string& path);
    std::shared_ptr<age::uint8_vector> load_rom_file(const std::filesystem::path& rom_path);

} // namespace age::tester



#endif // AGE_TESTER_TASK_HPP
