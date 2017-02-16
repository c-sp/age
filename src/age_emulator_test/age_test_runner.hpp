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

#ifndef AGE_TEST_RUNNER_HPP
#define AGE_TEST_RUNNER_HPP

//!
//! \file
//!

#include "age_test.hpp"



namespace age
{

enum class test_type : int
{
    screenshot_test, // this is the default
    mooneye_test,
    gambatte_test
};



class test_runner_application : public QObject, non_copyable
{
    Q_OBJECT
public:

    test_runner_application(const QString &test, const QString &ignore_file, test_type type);
    ~test_runner_application() override;

public slots:

    void schedule_tests();
    void about_to_quit();

    void test_passed(QString test_file, QString pass_message);
    void test_failed(QString test_file, QString fail_message);

private:

    bool find_files(QSet<QString> &files) const;
    void find_files(const QFileInfo &file_info, QSet<QString> &files) const;
    bool ignore_files(QSet<QString> &files) const;

    QString create_message(const QString &test_file, const QString &message) const;
    void exit_app_on_finish();
    void print_message_list(const QString &first_line, const QStringList &message_list) const;

    const QRegExp m_test_file_pattern;
    const QString m_test;
    const QString m_ignore_file;
    const test_type m_type;

    QThreadPool m_thread_pool;
    QSet<QString> m_tests_running;
    QStringList m_pass_messages;
    QStringList m_fail_messages;
};

} // namespace age



#endif // AGE_TEST_RUNNER_HPP
