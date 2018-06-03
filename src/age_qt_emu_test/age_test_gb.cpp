//
// Copyright 2018 Christoph Sprenger
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

#include <memory> // std::shared_ptr

#include <QImage>
#include <QTextStream>

#include "age_test_gb.hpp"



age::test_result age::create_gb_test_result(const gb_emulator &emulator, const QString &error_message)
{
    age::test_result result;
    result.m_cycles_emulated = emulator.get_emulated_cycles();
    result.m_error_message = error_message;
    result.m_additional_message = emulator.is_cgb() ? "(CGB)" : "(DMG)";
    return result;
}



//---------------------------------------------------------
//
//   mooneye test
//
//---------------------------------------------------------

age::test_method age::mooneye_test_method()
{
    // this method is based on mooneye-gb/src/acceptance_tests/fixture.rs
    return [](const uint8_vector &test_rom, const uint8_vector&) {

        // create emulator
        std::shared_ptr<gb_emulator> emulator = std::make_shared<gb_emulator>(test_rom);

        // run the test
        uint cycles_per_step = emulator->get_cycles_per_second() >> 8;
        uint max_cycles = emulator->get_cycles_per_second() * 120;

        for (uint cycles = 0; cycles < max_cycles; cycles += cycles_per_step)
        {
            emulator->emulate(cycles_per_step);
            // if an invalid opcode was encountered, the test is finished
            if (emulator->get_test_info().m_found_invalid_opcode)
            {
                break;
            }
        }

        // evaluate the test result
        // (a passed test is signalled by a part of the Fibonacci Sequence)
        gb_test_info info = emulator->get_test_info();
        bool pass = (3 == info.m_b)
                && (5 == info.m_c)
                && (8 == info.m_d)
                && (13 == info.m_e)
                && (21 == info.m_h)
                && (34 == info.m_l)
                ;

        // return an error message, if the test failed
        return create_gb_test_result(*emulator, pass ? "" : "failed");
    };
}



//---------------------------------------------------------
//
//   screenshot test
//
//---------------------------------------------------------

age::test_method age::screenshot_test_png(bool force_dmg, bool dmg_green, uint millis_to_emulate)
{
    return [=](const age::uint8_vector &test_rom, const age::uint8_vector &screenshot) {

        // create emulator & run test
        std::shared_ptr<gb_emulator> emulator = std::make_shared<gb_emulator>(test_rom, force_dmg, dmg_green);

        uint cycles_to_emulate = millis_to_emulate * emulator->get_cycles_per_second() / 1000;
        emulator->emulate(cycles_to_emulate);

        // load the screenshot image data
        QString error_message;
        QImage image = QImage::fromData(screenshot.data(), screenshot.size()).rgbSwapped();

        // is the data format supported?
        if (image.isNull())
        {
            error_message = "data format of screenshot not supported";
        }

        // check the screenshot size
        else if ((emulator->get_screen_width() != static_cast<uint>(image.width()))
                || (emulator->get_screen_height() != static_cast<uint>(image.height()))) {

            QTextStream(&error_message)
                    << "screen size ("
                    << emulator->get_screen_width() << "x" << emulator->get_screen_height()
                    << ") does not match screenshot size ("
                    << image.width() << "x" << image.height() << ")";
        }

        // check the screenshot depth
        else if ((sizeof_pixel * 8) != image.depth())
        {
            QTextStream(&error_message)
                    << "screen depth (" << (sizeof_pixel * 8)
                    << " bits) does not match screenshot depth ("
                    << image.depth() << " bits)";
        }

        // compare the gameboy screen with the screenshot
        else
        {
            const pixel *screenshot = reinterpret_cast<pixel*>(image.bits());
            const pixel *screen = emulator->get_screen_front_buffer().data();

            for (uint i = 0, max = emulator->get_screen_width() * emulator->get_screen_height(); i < max; ++i)
            {
                if (*screen != *screenshot)
                {
                    uint x = i % emulator->get_screen_width();
                    uint y = emulator->get_screen_height() - 1 - (i / emulator->get_screen_width());

                    QTextStream(&error_message)
                            << "screen and screenshot differ at position ("
                            << x << ',' << y << ')'
                            << ": expected 0x" << hex << screenshot->m_a8b8g8r8 << ", found 0x" << screen->m_a8b8g8r8;
                    break;
                }
                ++screen;
                ++screenshot;
            }
        }

        // return an error message, if the test failed
        return create_gb_test_result(*emulator, error_message);
    };
}