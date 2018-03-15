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

#ifndef AGE_TEST_GB_HPP
#define AGE_TEST_GB_HPP

//!
//! \file
//!

#include <QString>

#include <age_types.hpp>
#include <emulator/age_gb_emulator.hpp>

#include "age_test.hpp"



namespace age
{

test_result create_gb_test_result(const gb_emulator &emulator, const QString &error_message);

test_method mooneye_test_method();

test_method screenshot_test_png(bool force_dmg, bool dmg_green, uint millis_to_emulate);

test_method gambatte_dmg_test(const QString &test_file_name, QString &result_file_name);

test_method gambatte_cgb_test(const QString &test_file_name, QString &result_file_name);

}



#endif // AGE_TEST_GB_HPP
