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

#ifndef AGE_TEST_APP_HPP
#define AGE_TEST_APP_HPP

//!
//! \file
//!

#include <atomic>
#include <limits>
#include <memory> // std::shared_ptr

#include <QFileInfo>
#include <QObject>
#include <QRegExp>
#include <QRunnable>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QThreadPool>

#include <age_types.hpp>

#include "age_test_gb.hpp"



namespace age
{

enum class test_type : int
{
    screenshot_test, // this is the default
    mooneye_test,
    gambatte_test
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



class test_application : public QObject, non_copyable
{
    Q_OBJECT
public:

    test_application(const QString &test, const QString &ignore_file, test_type type);
    ~test_application() override;

public slots:

    void schedule_tests();
    void about_to_quit();

    void test_passed(QString test_file, QString pass_message);
    void test_failed(QString test_file, QString fail_message);

private:

    bool find_files(QSet<QString> &files) const;
    void find_files(const QFileInfo &file_info, QSet<QString> &files) const;
    bool ignore_files(QSet<QString> &files) const;
    int schedule_test(const QString &file_name, test_method method, const QString &result_file_name = {});

    void exit_app_on_finish();

    QString test_message(const QString &test_file, const QString &message) const;
    QString number_of_tests_message(QString message, int number_of_tests, int total) const;
    void print_list(const QString &first_line, const QStringList &message_list) const;
    static int percent(int value, int total);

    const QRegExp m_test_file_pattern;
    const QString m_test;
    const QString m_ignore_file;
    const test_type m_type;

    std::shared_ptr<test_performance> m_test_performance = nullptr;
    QThreadPool m_thread_pool;
    int m_tests_running = 0;
    QStringList m_pass_messages;
    QStringList m_fail_messages;
    QStringList m_no_test_method_found;
};



class test_runner : public QObject, public QRunnable
{
    Q_OBJECT
public:

    virtual ~test_runner() = default;

    test_runner(const QString &test_file_name,
                const QString &result_file_name,
                test_method method,
                std::shared_ptr<test_performance> performance = nullptr);

    void run() override;

signals:

    void test_passed(QString test_file_name, QString pass_message);
    void test_failed(QString test_file_name, QString fail_message);

private:

    bool read_file(const QString &file_name, uint8_vector &destination, QString &error_message);

    const QString m_test_file_name;
    const QString m_result_file_name;
    uint8_vector m_test_file;
    uint8_vector m_result_file;

    std::shared_ptr<test_performance> m_test_performance = nullptr;
    test_method m_test_method;
};

} // namespace age



#endif // AGE_TEST_APP_HPP
