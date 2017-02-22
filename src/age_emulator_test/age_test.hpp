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
#include <functional>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
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



//!
//! \brief A utility class for collection performance information about executed tests.
//!
//! With this class the following performance numbers can be collected for executed tests:
//! - The average, minimal and maximal time spent on file operations (e.g. loading test files).
//! - The average, minimal and maximal time spent on executing a test.
//! - The average, minimal and maximal number of cycles emulated for executing a test.
//! - The average number of cycles emulated per second.
//!
//! This class is thread safe.
//!
class test_performance
{
public:

    void file_loaded(qint64 duration_nanos);
    void test_executed(qint64 duration_nanos, uint64 emulated_cycles);

    bool summary_available() const;
    void print_summary() const;

private:

    struct measurement
    {
        void add_run(qint64 value);

        std::atomic<qint64> m_runs = {0};
        std::atomic<qint64> m_sum = {0};
        std::atomic<qint64> m_min = {std::numeric_limits<qint64>::max()};
        std::atomic<qint64> m_max = {std::numeric_limits<qint64>::min()};
    };

    static void print_measurement(const measurement &m, const QString &task, qint64 divisor);

    measurement m_file_load_nanos;
    measurement m_test_execution_nanos;
    measurement m_emulation_cycles;
};



struct test_result
{
    uint64 m_cycles_emulated = 0;
    QString m_error_message;
    QString m_additional_message;
};

typedef std::function<test_result(const uint8_vector &test_rom)> test_method;



class test_runner : public QObject, public QRunnable
{
    Q_OBJECT
public:

    virtual ~test_runner() = default;

    test_runner(const QString &test_file_name,
                test_method method,
                std::shared_ptr<test_performance> performance = nullptr);

    void run() override;

signals:

    void test_passed(QString test_file_name, QString pass_message);
    void test_failed(QString test_file_name, QString fail_message);

private:

    QString read_test_file();

    const QString m_test_file_name;
    std::shared_ptr<test_performance> m_test_performance = nullptr;
    test_method m_test_method;
    uint8_vector m_test_file;
};

} // namespace age



#endif // AGE_TEST_HPP
