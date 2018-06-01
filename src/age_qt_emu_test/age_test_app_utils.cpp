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

#include <QFile>
#include <QElapsedTimer>

#include "age_test_app.hpp"





//---------------------------------------------------------
//
//   test_performance
//
//---------------------------------------------------------

void age::test_performance::measurement::add_run(qint64 value)
{
    AGE_ASSERT(value >= 0);
    ++m_runs;
    m_sum += value;

    qint64 old;
    while ((old = m_min) > value)
    {
        if (m_min.compare_exchange_strong(old, value))
        {
            break;
        }
    }

    while ((old = m_max) < value)
    {
        if (m_max.compare_exchange_strong(old, value))
        {
            break;
        }
    }
}



void age::test_performance::file_loaded(qint64 duration_nanos)
{
    if (duration_nanos >= 0)
    {
        m_file_load_nanos.add_run(duration_nanos);
    }
}

void age::test_performance::test_executed(qint64 duration_nanos, uint64 emulated_cycles)
{
    if (duration_nanos >= 0)
    {
        m_test_execution_nanos.add_run(duration_nanos);
        m_emulation_cycles.add_run(emulated_cycles);
    }
}



bool age::test_performance::summary_available() const
{
    return (m_file_load_nanos.m_runs > 0) || (m_emulation_cycles.m_runs > 0);
}

void age::test_performance::print_summary() const
{
    qInfo("\ntest performance:               +--------------+--------------+--------------+");
    qInfo("                                |          min |      average |          max |");
    qInfo("    +---------------------------+--------------+--------------+--------------+");

    print_measurement(m_file_load_nanos, "file loading (us)", 1000);
    print_measurement(m_test_execution_nanos, "test execution (us)", 1000);
    print_measurement(m_emulation_cycles, "emulated cycles per test", 1);

    qInfo("    +---------------------------+--------------+--------------+--------------+");
}

void age::test_performance::print_measurement(const measurement &m, const QString &task, qint64 divisor)
{
    qint64 runs = m.m_runs;
    qint64 avg = (runs < 1) ? 0 : m.m_sum / runs / divisor;
    qint64 min = (runs < 1) ? 0 : m.m_min / divisor;
    qint64 max = (runs < 1) ? 0 : m.m_max / divisor;

    qInfo("    | %25s | %12lld | %12lld | %12lld |", qPrintable(task), min, avg, max);
}





//---------------------------------------------------------
//
//   test_runner
//
//---------------------------------------------------------

age::test_runner::test_runner(const QString &test_file_name,
                              const QString &result_file_name,
                              test_method method,
                              std::shared_ptr<test_performance> performance)

    : m_test_file_name(test_file_name),
      m_result_file_name(result_file_name),
      m_test_performance(performance),
      m_test_method(method)
{
}



void age::test_runner::run()
{
    QString error_message;

    // read the test file
    if (!read_file(m_test_file_name, m_test_file, error_message))
    {
        emit test_failed(m_test_file_name, error_message);
    }
    // read the result file, if there is one
    else if (!m_result_file_name.isEmpty() && !read_file(m_result_file_name, m_result_file, error_message))
    {
        emit test_failed(m_test_file_name, error_message);
    }
    // execute the test
    else
    {
        QElapsedTimer timer;
        timer.start();

        test_result result = m_test_method(m_test_file, m_result_file);

        // store the time required for executing the test
        if (m_test_performance != nullptr)
        {
            m_test_performance->test_executed(timer.nsecsElapsed(), result.m_cycles_emulated);
        }

        // emit the result signal
        if (result.m_error_message.isEmpty())
        {
            emit test_passed(m_test_file_name, result.m_additional_message);
        }
        else
        {
            QString message = result.m_additional_message.isEmpty()
                    ? result.m_error_message
                    : result.m_additional_message + ' ' + result.m_error_message;

            emit test_failed(m_test_file_name, message);
        }
    }
}



bool age::test_runner::read_file(const QString &file_name, uint8_vector &destination, QString &error_message)
{
    bool result = true;

    // start timer for measuring the time spent on loading the file
    QElapsedTimer timer;
    timer.start();

    // open the test file
    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly))
    {
        error_message = QString{"could not open file: "} + file_name;
        result = false;
    }

    // read the whole test file
    else
    {
        const qint64 bytes_total = file.size();
        qint64 bytes_read = 0;

        destination.resize(bytes_total, 0);
        char *buffer = reinterpret_cast<char*>(destination.data());

        // read until the buffer is filled
        while (bytes_read < bytes_total)
        {
            qint64 remaining = bytes_total - bytes_read;
            qint64 read = file.read(buffer + bytes_read, remaining);

            // break the loop, if an error occurred
            if (read < 0)
            {
                break;
            }
            bytes_read += read;
        }

        // check if we read everything
        if (bytes_read < bytes_total)
        {
            error_message = QString{"could not read file: "} + file_name;
            result = false;
        }
    }

    // store the time required for loading the file on success
    if (result && (m_test_performance != nullptr))
    {
        m_test_performance->file_loaded(timer.nsecsElapsed());
    }

    return result;
}
