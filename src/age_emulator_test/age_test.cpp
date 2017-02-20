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

void age::test_performance::file_loaded(qint64 duration_nanos)
{
    if (duration_nanos >= 0)
    {
        m_file_load_nanos.add_run(duration_nanos);
    }
}

void age::test_performance::test_emulated(qint64 duration_nanos, uint64 emulated_ticks)
{
    if ((duration_nanos >= 0) && (emulated_ticks > 0))
    {
        m_emulation_nanos.add_run(duration_nanos);
        m_emulation_ticks.add_run(emulated_ticks);
    }
}



void age::test_performance::print_summary() const
{
    qInfo("\nperformance:");

    qInfo("    +----------------------+--------------+--------------+--------------+");
    qInfo("    |                 task |      average |          min |          max |");
    qInfo("    +----------------------+--------------+--------------+--------------+");

    qInfo("    | %20s | %12lld | %12lld | %12lld |",
          "file loading (us)",
          (m_file_load_nanos.m_sum / m_file_load_nanos.m_runs) / 1000,
          m_file_load_nanos.m_min / 1000,
          m_file_load_nanos.m_max / 1000
          );

    qInfo("    | %20s | %12lld | %12lld | %12lld |",
          "emulation (us)",
          (m_emulation_nanos.m_sum / m_emulation_nanos.m_runs) / 1000,
          m_emulation_nanos.m_min / 1000,
          m_emulation_nanos.m_max / 1000
          );

    qInfo("    | %20s | %12lu | %12lu | %12lu |",
          "emulation (ticks)",
          m_emulation_ticks.m_sum / m_emulation_ticks.m_runs,
          m_emulation_ticks.m_min.load(),
          m_emulation_ticks.m_max.load()
          );

    qInfo("    +----------------------+--------------+--------------+--------------+");

    qInfo("    %llu emulator ticks per second",
          m_emulation_ticks.m_sum * 1000000000 / m_emulation_nanos.m_sum
          );
}





//---------------------------------------------------------
//
//   gb_emulator_test
//
//---------------------------------------------------------

age::gb_emulator_test::gb_emulator_test(const QString &test_file_name, std::shared_ptr<test_performance> performance)
    : m_test_file_name(test_file_name),
      m_test_performance(performance)
{
}



void age::gb_emulator_test::run()
{
    QElapsedTimer timer;

    // read the test file
    timer.start();
    QString error_cause = read_test_file();

    // continue only if the file was sucessfully read
    optional<bool> is_cgb;
    if (error_cause.isEmpty())
    {
        // store the time required for loading the file
        if (m_test_performance != nullptr)
        {
            m_test_performance->file_loaded(timer.nsecsElapsed());
        }

        // create a new emulator running the test file
        std::shared_ptr<gb_simulator> emulator = std::make_shared<gb_simulator>(m_test_file, false);
        is_cgb.set(emulator->is_cgb());

        // execute the test
        timer.start();
        error_cause = run_test(*emulator);

        // store the time required for executing the test
        if (m_test_performance != nullptr)
        {
            m_test_performance->test_emulated(timer.nsecsElapsed(), emulator->get_simulated_ticks());
        }
    }

    // create a CGB/DMG marker to be used within the result message
    QString gb_type = is_cgb.is_set()
            ? (is_cgb.get(false) ? "(CGB)" : "(DMG)")
            : "";

    // emit the result signal
    if (error_cause.isEmpty())
    {
        emit test_passed(m_test_file_name, gb_type);
    }
    else
    {
        emit test_failed(m_test_file_name, (gb_type.isEmpty() ? error_cause : gb_type + ' ' + error_cause));
    }
}



QString age::gb_emulator_test::read_test_file()
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
