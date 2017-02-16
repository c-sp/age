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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QRunnable>
#include <QSet>
#include <QTextStream>
#include <QThreadPool>
#include <QTimer>

//! \endcond

#include <age_gb_simulator.hpp>



namespace age
{

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



class gb_emulator_test : public QObject, public QRunnable
{
    Q_OBJECT
public:

    gb_emulator_test(const QString &test_file_name);
    virtual ~gb_emulator_test() = default;

    void run() override;

signals:

    void test_passed(QString test_file_name, QString pass_message);
    void test_failed(QString test_file_name, QString fail_message);

protected:

    virtual QString run_test(gb_simulator &emulator) = 0;

private:

    QString read_test_file();

    const QString m_test_file_name;
    uint8_vector m_test_file;
};

} // namespace age



#endif // AGE_TEST_HPP
