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

#include "age_test.hpp"



//---------------------------------------------------------
//
//   object creation
//
//---------------------------------------------------------

age::gb_emulator_test::gb_emulator_test(const QString &test_file_name)
    : m_test_file_name(test_file_name)
{
}



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

void age::gb_emulator_test::run()
{
    QString error_cause;
    optional<bool> is_cgb;

    // load test file and create emulator
    if (!load_test_file())
    {
        error_cause = "could not load test file";
    }

    // run the test
    else
    {
        std::shared_ptr<gb_simulator> emulator = std::allocate_shared<gb_simulator>(std::allocator<gb_simulator>(), m_test_file, false);
        is_cgb.set(emulator->is_cgb());
        //error_cause = run_test(*emulator);
    }

    // append a CGB/DMG marker to the test file name if we created an emulator
    QString gb_type = is_cgb.is_set()
            ? (is_cgb.get(false) ? "(CGB)" : "(DMG)")
            : "";

    // emit the result signal
    if (error_cause.isEmpty())
    {
        emit test_passed(m_test_file_name, gb_type);
    }
    else
    {
        emit test_failed(m_test_file_name, (gb_type.isEmpty() ? error_cause : gb_type + ' ' + error_cause));
    }
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

bool age::gb_emulator_test::load_test_file()
{
    //! \todo load binary file
    return true;
}
