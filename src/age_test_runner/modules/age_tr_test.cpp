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

#include "age_tr_test.hpp"
#include "age_tr_write_log.hpp"

#include <gfx/age_png.hpp>

#include <iostream>
#include <memory>
#include <utility>



namespace
{
    std::string log_file_extension(age::gb_device_type device_type)
    {
        switch (device_type)
        {
            case age::gb_device_type::dmg:
                return ".dmg.log";
            case age::gb_device_type::cgb_abcd:
                return ".cgb-abcd.log";
            case age::gb_device_type::cgb_e:
                return ".cgb-e.log";
            default:
                return ".log";
        }
    }

} // namespace



age::tr::age_tr_test::age_tr_test(std::filesystem::path               rom_path,
                                  std::shared_ptr<const uint8_vector> rom,
                                  gb_device_type                      device_type)
    : age_tr_test(std::move(rom_path),
                  std::move(rom),
                  device_type,
                  finished_after_ld_b_b(),
                  succeeded_with_fibonacci_regs())
{}

age::tr::age_tr_test::age_tr_test(std::filesystem::path                        rom_path,
                                  std::shared_ptr<const uint8_vector>          rom,
                                  age::gb_device_type                          device_type,
                                  std::function<bool(const age::gb_emulator&)> test_finished,
                                  std::function<bool(const age::gb_emulator&)> test_succeeded)
    : age_tr_test(
        std::move(rom_path),
        std::move(rom),
        device_type,
        [=](age::gb_emulator& emulator) {
            int cycles_per_iteration = emulator.get_cycles_per_second() / 256;
            while (!test_finished(emulator))
            {
                emulator.emulate(cycles_per_iteration);
            }
        },
        test_succeeded)
{}

age::tr::age_tr_test::age_tr_test(std::filesystem::path                        rom_path,
                                  std::shared_ptr<const uint8_vector>          rom,
                                  age::gb_device_type                          device_type,
                                  std::function<void(age::gb_emulator&)>       run_test,
                                  std::function<bool(const age::gb_emulator&)> test_succeeded)
    : age_tr_test(std::move(rom_path),
                  std::move(rom),
                  device_type,
                  {},
                  run_test,
                  test_succeeded)
{}

age::tr::age_tr_test::age_tr_test(std::filesystem::path                        rom_path,
                                  std::shared_ptr<const uint8_vector>          rom,
                                  age::gb_device_type                          device_type,
                                  std::string                                  additional_info,
                                  std::function<void(age::gb_emulator&)>       run_test,
                                  std::function<bool(const age::gb_emulator&)> test_succeeded)
    : age_tr_test(std::move(rom_path),
                  std::move(rom),
                  device_type,
                  device_type == gb_device_type::dmg
                      ? gb_colors_hint::dmg_greyscale
                      : gb_colors_hint::cgb_acid2,
                  std::move(additional_info),
                  run_test,
                  test_succeeded)
{}

age::tr::age_tr_test::age_tr_test(std::filesystem::path                        rom_path,
                                  std::shared_ptr<const uint8_vector>          rom,
                                  age::gb_device_type                          device_type,
                                  gb_colors_hint                               colors_hint,
                                  std::string                                  additional_info,
                                  std::function<void(age::gb_emulator&)>       run_test,
                                  std::function<bool(const age::gb_emulator&)> test_succeeded)
    : m_rom_path(std::move(rom_path)),
      m_rom(std::move(rom)),
      m_device_type(device_type),
      m_colors_hint(colors_hint),
      m_additional_info(std::move(additional_info)),
      m_run_test(std::move(run_test)),
      m_test_succeeded(std::move(test_succeeded))
{}



age::gb_device_type age::tr::age_tr_test::device_type() const
{
    return m_device_type;
}

std::string age::tr::age_tr_test::test_name() const
{
    std::string tn = m_rom_path;
    if (!m_additional_info.empty())
    {
        tn += ", " + m_additional_info;
    }
    return tn + " " + get_device_type_string(m_device_type);
}

void age::tr::age_tr_test::init_test(const gb_log_categories& log_categories)
{
    if (m_emulator == nullptr)
    {
        m_emulator = std::make_shared<gb_emulator>(*m_rom, m_device_type, m_colors_hint, log_categories);
    }
}

void age::tr::age_tr_test::run_test()
{
    m_run_test(*m_emulator);
}

bool age::tr::age_tr_test::test_succeeded()
{
    return m_test_succeeded(*m_emulator);
}

void age::tr::age_tr_test::write_logs()
{
    std::filesystem::path log_path = m_rom_path;
    log_path.replace_extension(log_file_extension(m_device_type));
    write_log(log_path, m_emulator->get_and_clear_log_entries(), m_rom_path, m_device_type);
}



std::function<bool(const age::gb_emulator&)> age::tr::finished_after_milliseconds(age::int64_t milliseconds)
{
    return [=](const age::gb_emulator& emulator) {
        auto cycles = milliseconds * emulator.get_cycles_per_second() / 1000;
        return emulator.get_emulated_cycles() >= cycles;
    };
}

std::function<bool(const age::gb_emulator&)> age::tr::finished_after_ld_b_b()
{
    return [](const age::gb_emulator& emulator) {
        int max_cycles = emulator.get_cycles_per_second() * 120;
        return emulator.get_test_info().m_ld_b_b
               || emulator.get_emulated_cycles() >= max_cycles;
    };
}

std::function<bool(const age::gb_emulator&)> age::tr::succeeded_with_screenshot(const std::filesystem::path& screenshot_path)
{
    return [=](const age::gb_emulator& emulator) {
        // load png
        auto screenshot = read_png_file(screenshot_path,
                                        emulator.get_screen_width(),
                                        emulator.get_screen_height());
        if (screenshot.empty())
        {
            std::cout << "could not load screenshot: " << screenshot_path.string() << std::endl;
            return false;
        }

        // compare screen to screenshot
        const auto& screen = emulator.get_screen_front_buffer();
        if (screenshot.size() != screen.size())
        {
            std::cout << "screenshot size mismatch (expected "
                      << screen.size()
                      << " pixel, got "
                      << screenshot.size()
                      << "pixel): " << screenshot_path.string() << std::endl;
            return false;
        }

        bool screenshot_matches = screen == screenshot;
        if (!screenshot_matches)
        {
            auto png_path = screenshot_path;
            png_path.replace_extension();
            png_path += "_actual.png";

            write_png_file(emulator.get_screen_front_buffer(),
                           emulator.get_screen_width(),
                           emulator.get_screen_height(),
                           png_path.string());
        }

        return screenshot_matches;
    };
}

std::function<bool(const age::gb_emulator&)> age::tr::succeeded_with_fibonacci_regs()
{
    return [](const age::gb_emulator& emulator) {
        age::gb_test_info info = emulator.get_test_info();
        return (3 == info.m_b)
               && (5 == info.m_c)
               && (8 == info.m_d)
               && (13 == info.m_e)
               && (21 == info.m_h)
               && (34 == info.m_l);
    };
}
