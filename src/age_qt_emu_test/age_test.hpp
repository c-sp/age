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

#ifndef AGE_TEST_HPP
#define AGE_TEST_HPP

//!
//! \file
//!

#include <functional>

#include <QString>

#include <age_types.hpp>


namespace age
{

//!
//! This template was created due to the lack of C++17 features.
//! It's only purpose is to keep a flag next to a value to remember
//! if that value has been explicitly set.
//!
template<typename _T>
class optional
{
public:

    bool is_set() const
    {
        return m_value_set;
    }

    _T get(_T default_value) const
    {
        return m_value_set ? m_value : default_value;
    }

    void set(_T t)
    {
        m_value = t;
        m_value_set = true;
    }

private:

    bool m_value_set = false;
    _T m_value;
};



struct test_result
{
    qint64 m_cycles_emulated = 0;
    QString m_error_message;
    QString m_additional_message;
};

typedef std::function<test_result(const uint8_vector &test_rom, const uint8_vector &result_file)> test_method;

} // namespace age



#endif // AGE_TEST_HPP
