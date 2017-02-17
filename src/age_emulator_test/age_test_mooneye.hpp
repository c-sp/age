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

#ifndef AGE_TEST_MOONEYE_HPP
#define AGE_TEST_MOONEYE_HPP

//!
//! \file
//!

#include "age_test.hpp"



namespace age
{

//!
//! \brief A mooneye-gb test runner.
//!
//! Mooneye-gb tests usually signal a finished test with an invalid opcode.
//! The test success can then be verified by checking the contents of Gameboy CPU registers.
//!
class gb_emulator_test_mooneye : public gb_emulator_test
{
    Q_OBJECT
public:

    using gb_emulator_test::gb_emulator_test;

    virtual ~gb_emulator_test_mooneye() = default;

protected:

    QString run_test(gb_simulator &emulator) override;
};

}



#endif // AGE_TEST_MOONEYE_HPP
