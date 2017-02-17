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
//   object creation
//
//---------------------------------------------------------

age::gb_emulator_test::gb_emulator_test(const QString &test_file_name)
    : m_test_file_name(test_file_name)
{
}



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

void age::gb_emulator_test::run()
{
    // read the test file
    QString error_cause = read_test_file();

    // run the test, if the test file was sucessfully read
    optional<bool> is_cgb;
    if (error_cause.isEmpty())
    {
        std::shared_ptr<gb_simulator> emulator = std::allocate_shared<gb_simulator>(std::allocator<gb_simulator>(), m_test_file, false);
        is_cgb.set(emulator->is_cgb());
        error_cause = run_test(*emulator);
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



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

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
