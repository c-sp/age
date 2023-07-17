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

#ifndef AGE_TR_TEST_HPP
#define AGE_TR_TEST_HPP

#include <age_types.hpp>
#include <emulator/age_gb_emulator.hpp>

#include <filesystem>
#include <functional>
#include <memory>
#include <string>



namespace age::tr
{
    class age_tr_test
    {
    public:
        age_tr_test(std::filesystem::path                        rom_path,
                    std::shared_ptr<const uint8_vector>          rom,
                    gb_device_type                               device_type,
                    std::function<bool(const age::gb_emulator&)> test_finished,
                    std::function<bool(const age::gb_emulator&)> test_succeeded);

        gb_device_type device_type() const;
        std::string    test_name() const;

        void init_test(const gb_log_categories& log_categories);
        void run_test();
        bool test_succeeded();
        void write_logs();

    private:
        std::filesystem::path               m_rom_path;
        std::shared_ptr<const uint8_vector> m_rom;
        gb_device_type                      m_device_type;
        gb_colors_hint                      m_colors_hint;
        std::string                         m_additional_info;

        std::function<bool(const age::gb_emulator&)> m_test_finished;
        std::function<bool(const age::gb_emulator&)> m_test_succeeded;

        std::shared_ptr<gb_emulator> m_emulator;
    };



    std::function<bool(const age::gb_emulator&)> finished_after_cycle(int64_t cycle);
    std::function<bool(const age::gb_emulator&)> finished_after_ld_b_b();

    std::function<bool(const age::gb_emulator&)> succeeded_with_screenshot(const std::filesystem::path& screenshot_path);
    std::function<bool(const age::gb_emulator&)> succeeded_with_fibonacci_regs();

} // namespace age::tr



#endif // AGE_TR_TEST_HPP
