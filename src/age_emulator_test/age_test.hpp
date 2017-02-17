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
//! \brief This is the base class for all Gameboy emulator tests.
//!
//! The base class will load the test file and create an emulator using that file.
//! The actual test execution has to be implemented by deriving classes with run_test().
//! Depending on the test result, this class will emit either a test_passed() or a test_failed() signal.
//!
//! This class derives from QRunnable to allow for asynchronously running the test in a QThreadPool.
//!
class gb_emulator_test : public QObject, public QRunnable
{
    Q_OBJECT
public:

    //!
    //! \brief Construct a gb_emulator_test based on the specified test file.
    //! \param test_file_name The test file to be loaded when running the test.
    //!
    gb_emulator_test(const QString &test_file_name);

    virtual ~gb_emulator_test() = default;

    //!
    //! \brief Run the test.
    //!
    //! This method will load the test file, create a gb_simulator with that file
    //! and call the run_test() method.
    //! If loading the file or run_test() fails, the test_failed() signal is emitted.
    //! If the test passes, the test_passed() signal is emitted.
    //!
    void run() override;

signals:

    //!
    //! \brief This signal is emitted after the test passes.
    //! \param test_file_name The test file passed to the constructor.
    //! \param pass_message Additional human readable information on this test.
    //!
    void test_passed(QString test_file_name, QString pass_message);

    //!
    //! \brief This signal is emitted after the test fails.
    //! \param test_file_name The test file passed to the constructor.
    //! \param pass_message Additional human readable information on the failure's cause.
    //!
    void test_failed(QString test_file_name, QString fail_message);

protected:

    //!
    //! \brief the actual test execution
    //! \param emulator The gb_simulator instance loaded with the test file.
    //! \return A QString describing the test failure or an empty QString if the test passes.
    //!
    virtual QString run_test(gb_simulator &emulator) = 0;

private:

    QString read_test_file();

    const QString m_test_file_name;
    uint8_vector m_test_file;
};

} // namespace age



#endif // AGE_TEST_HPP
