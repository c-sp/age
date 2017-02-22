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

#include "age_test.hpp"





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
    qInfo("\nperformance:                    +--------------+--------------+--------------+");
    qInfo("                                |      average |          min |          max |");
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

    qInfo("    | %25s | %12lld | %12lld | %12lld |", qPrintable(task), avg, min, max);
}





//---------------------------------------------------------
//
//   test_runner
//
//---------------------------------------------------------

age::test_runner::test_runner(const QString &test_file_name, test_method method, std::shared_ptr<test_performance> performance)
    : m_test_file_name(test_file_name),
      m_test_performance(performance),
      m_test_method(method)
{
}



void age::test_runner::run()
{
    test_result result;
    QElapsedTimer timer;

    // read the test file
    timer.start();
    result.m_error_message = read_test_file();

    // continue only if the file was sucessfully read
    if (result.m_error_message.isEmpty())
    {
        // store the time required for loading the file
        if (m_test_performance != nullptr)
        {
            m_test_performance->file_loaded(timer.nsecsElapsed());
        }

        // execute the test
        timer.start();
        result = m_test_method(m_test_file);

        // store the time required for executing the test
        if (m_test_performance != nullptr)
        {
            m_test_performance->test_executed(timer.nsecsElapsed(), result.m_cycles_emulated);
        }
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



QString age::test_runner::read_test_file()
{
    QString error_message;

    // open the test file
    QFile file(m_test_file_name);
    if (!file.open(QIODevice::ReadOnly))
    {
        error_message = "could not open file";
    }

    // read the whole test file
    else
    {
        const qint64 bytes_total = file.size();
        qint64 bytes_read = 0;

        m_test_file = uint8_vector(bytes_total, 0);
        char *buffer = reinterpret_cast<char*>(m_test_file.data());

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
            error_message = "could not read file";
        }
    }

    return error_message;
}
