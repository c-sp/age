//
// Copyright 2019 Christoph Sprenger
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

#include <QSharedPointer>

#include "age_test_gb.hpp"



age::test_method age::mooneye_test_method(const QString &file_name)
{
    // this method is based on mooneye-gb/src/acceptance_tests/fixture.rs
    return [=](const uint8_vector &test_rom, const uint8_vector&)
    {
        // CGB mooneye-gb test roms are identified by their file name
        age::gb_hardware hardware = age::gb_hardware::dmg;

        if (file_name.endsWith(".gb", Qt::CaseInsensitive))
        {
            QString no_extension = file_name.left(file_name.length() - 3);

            if (no_extension.contains("-cgb", Qt::CaseInsensitive) || no_extension.endsWith("-c", Qt::CaseInsensitive))
            {
                hardware = age::gb_hardware::cgb;
            }
        }

        // create emulator
        QSharedPointer<age::gb_emulator> emulator = QSharedPointer<age::gb_emulator>(new age::gb_emulator(test_rom, hardware));

        // run the test
        int cycles_per_step = emulator->get_cycles_per_second() >> 8;
        int max_cycles = emulator->get_cycles_per_second() * 120;

        for (int cycles = 0; cycles < max_cycles; cycles += cycles_per_step)
        {
            emulator->emulate(cycles_per_step);
            // the test is finished when executing a specific instruction
            if (emulator->get_test_info().m_mooneye_debug_op)
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
