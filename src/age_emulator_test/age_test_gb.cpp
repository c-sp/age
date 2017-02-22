//
// Copyright (c) 2010-2017 Christoph Sprenger
//
// This file is part of AGE ("Another Gameboy Emulator").
// <https://gitlab.com/csprenger/AGE>
//
// AGE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// AGE is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with AGE.  If not, see <http://www.gnu.org/licenses/>.
//

#include "age_test_gb.hpp"



age::test_result age::create_gb_test_result(const gb_emulator &emulator, const QString &error_message)
{
    age::test_result result;
    result.m_cycles_emulated = emulator.get_emulated_ticks();
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
    return [](const uint8_vector &test_rom) {

        constexpr uint cycles_per_step = gb_cycles_per_second >> 8;
        constexpr uint max_cycles = gb_cycles_per_second * 120;

        // create emulator
        std::shared_ptr<gb_emulator> emulator = std::make_shared<gb_emulator>(test_rom, false);

        // run the test
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
