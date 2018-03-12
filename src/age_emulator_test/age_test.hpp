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

#ifndef AGE_TEST_HPP
#define AGE_TEST_HPP

//!
//! \file
//!

// we skip the following includes for doxygen output since they would bloat the include graphs
//! \cond

#include <atomic>
#include <cmath>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QRegExp>
#include <QRunnable>
#include <QSet>
#include <QTextStream>
#include <QThreadPool>
#include <QTimer>

//! \endcond

#include <age_gb_emulator.hpp>



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
    uint64 m_cycles_emulated = 0;
    QString m_error_message;
    QString m_additional_message;
};

typedef std::function<test_result(const uint8_vector &test_rom, const uint8_vector &result_file)> test_method;

} // namespace age



#endif // AGE_TEST_HPP
