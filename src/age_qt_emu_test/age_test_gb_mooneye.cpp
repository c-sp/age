//
// Copyright 2020 Christoph Sprenger
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



namespace {

age::test_method mooneye_test(const QString &file_name, bool for_dmg)
{
    // filter CGB tests
    if (for_dmg)
    {
        if (file_name.contains("-cgb") || file_name.contains("-C")) {
            return {};
        }
    }
    // filter DMG tests
    else
    {
        if (file_name.contains("-dmg") || file_name.contains("-G")) {
            return {};
        }
    }

    age::gb_hardware hardware = for_dmg
            ? age::gb_hardware::dmg
            : age::gb_hardware::cgb;

    return [=](const age::uint8_vector &test_rom, const age::uint8_vector&)
    {
        // create emulator
        QSharedPointer<age::gb_emulator> emulator = QSharedPointer<age::gb_emulator>(new age::gb_emulator(test_rom, hardware));

        // based on mooneye-gb/src/acceptance_tests/fixture.rs

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
        age::gb_test_info info = emulator->get_test_info();
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

}



age::test_method age::mooneye_dmg_test(const QString &file_name)
{
    return mooneye_test(file_name, true);
}

age::test_method age::mooneye_cgb_test(const QString &file_name)
{
    return mooneye_test(file_name, false);
}
