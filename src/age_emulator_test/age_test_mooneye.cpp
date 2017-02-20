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

#include "age_test_mooneye.hpp"



QString age::gb_emulator_test_mooneye::run_test(gb_simulator &emulator)
{
    // this method is based on mooneye-gb/src/acceptance_tests/fixture.rs

    constexpr uint cycles_per_step = gb_cycles_per_second >> 8;
    constexpr uint max_cycles = gb_cycles_per_second * 120;

    // run the test
    uint cycles;
    for (cycles = 0; cycles < max_cycles; cycles += cycles_per_step)
    {
        emulator.simulate(cycles_per_step);

        // if an invalid opcode was encountered, the test is finished
        if (emulator.get_test_info().m_found_invalid_opcode)
        {
            break;
        }
    }
    AGE_LOG(emulator.get_simulated_ticks() << " = " << cycles);

    // evaluate the test result
    gb_test_info info = emulator.get_test_info();
    bool pass = (3 == info.m_b)
            && (5 == info.m_c)
            && (8 == info.m_d)
            && (13 == info.m_e)
            && (21 == info.m_h)
            && (34 == info.m_l)
            ;

    // return an error message, if the test failed
    return pass ? "" : "failed";
}
