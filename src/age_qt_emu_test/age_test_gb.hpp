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