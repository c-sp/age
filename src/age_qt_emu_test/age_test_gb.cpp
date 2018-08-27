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

#include <algorithm> // std::min
#include <memory> // std::shared_ptr

#include <QImage>
#include <QTextStream>

#include "age_test_gb.hpp"



age::test_result age::create_gb_test_result(const gb_emulator &emulator, const QString &error_message)
{
    age::test_result result;
    result.m_cycles_emulated = emulator.get_emulated_cycles();
    result.m_error_message = error_message;

    switch (emulator.get_test_info().m_mode)
    {
        case gb_mode::dmg:
            result.m_additional_message = "(DMG)";
            break;

        case gb_mode::dmg_on_cgb:
            result.m_additional_message = "(DMG on CGB)";
            break;

        case gb_mode::cgb:
            result.m_additional_message = "(CGB)";
            break;
    }

    return result;
}



void age::gb_emulate(gb_emulator &emulator, uint64 cycles_to_emulate)
{
    uint64 cycles_per_second = emulator.get_cycles_per_second();

    while (cycles_to_emulate > 0)
    {
        uint64 cycles = std::min(cycles_to_emulate, cycles_per_second);
        emulator.emulate(cycles);

        cycles_to_emulate -= cycles;
    }
}



//---------------------------------------------------------
//
//   screenshot-based test
//
//---------------------------------------------------------

age::test_method age::screenshot_test_png(bool force_dmg, bool dmg_green, uint64 millis_to_emulate)
{
    return [=](const age::uint8_vector &test_rom, const age::uint8_vector &screenshot) {

        // create emulator & run test
        age::gb_hardware hardware = force_dmg ? age::gb_hardware::dmg : age::gb_hardware::auto_detect;
        std::shared_ptr<gb_emulator> emulator = std::make_shared<gb_emulator>(test_rom, hardware, dmg_green);

        uint64 cycles_to_emulate = millis_to_emulate * emulator->get_cycles_per_second() / 1000;
        gb_emulate(*emulator, cycles_to_emulate);

        // load the screenshot image data
        QString error_message;
        QImage image = QImage::fromData(screenshot.data(), screenshot.size()).rgbSwapped();

        // is the data format supported?
        if (image.isNull())
        {
            error_message = "data format of screenshot not supported";
        }

        // check the screenshot size
        else if ((emulator->get_screen_width() != image.width())
                || (emulator->get_screen_height() != image.height())) {

            QTextStream(&error_message)
                    << "screen size ("
                    << emulator->get_screen_width() << "x" << emulator->get_screen_height()
                    << ") does not match screenshot size ("
                    << image.width() << "x" << image.height() << ")";
        }

        // check the screenshot depth
        else if ((sizeof(pixel) * 8) != image.depth())
        {
            QTextStream(&error_message)
                    << "screen depth (" << (sizeof(pixel) * 8)
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
                            << ": expected 0x" << hex << screenshot->m_color << ", found 0x" << screen->m_color;
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
