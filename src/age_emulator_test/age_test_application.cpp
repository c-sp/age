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

#include "age_test_application.hpp"



//---------------------------------------------------------
//
//   object creation & destruction
//
//---------------------------------------------------------

age::test_application::test_application(const QString &test, const QString &ignore_file, test_type type)
    : m_test_file_pattern(".*\\.gb[c]?", Qt::CaseInsensitive),
      m_test(test),
      m_ignore_file(ignore_file),
      m_type(type)
{
}

age::test_application::~test_application()
{
    m_thread_pool.clear();
}





//---------------------------------------------------------
//
//   public methods & slots
//
//---------------------------------------------------------

void age::test_application::schedule_tests()
{
    // look for test files to execute
    QSet<QString> files;
    if (!find_files(files))
    {
        fprintf(stderr, "%s is neither a file nor a directory\n", qPrintable(m_test));
        QCoreApplication::exit(EXIT_FAILURE);
    }

    // filter test files to ignore
    else if (!ignore_files(files))
    {
        fprintf(stderr, "could not read ignore list from file %s\n", qPrintable(m_ignore_file));
        QCoreApplication::exit(EXIT_FAILURE);
    }

    // terminate, if there are no tests to execute
    else if (files.isEmpty())
    {
        qInfo("no tests found, terminating");
        QCoreApplication::exit(EXIT_SUCCESS);
    }

    // schedule tests
    else
    {
        m_tests_running = files;
        for (QString file : m_tests_running)
        {
            gb_emulator_test *test = create_test(file);
            test->setAutoDelete(true); // let QThreadPool clean this up

            // connect via QueuedConnection since the signal is emitted across thread boundaries
            // (the respective slot will be executed by this thread and thus after this method returns)
            connect(test, SIGNAL(test_passed(QString,QString)), this, SLOT(test_passed(QString,QString)), Qt::QueuedConnection);
            connect(test, SIGNAL(test_failed(QString,QString)), this, SLOT(test_failed(QString,QString)), Qt::QueuedConnection);

            m_thread_pool.start(test);
        }

        qInfo("scheduled %d test(s) for execution", m_tests_running.size());
        qInfo("thread pool has %d thread(s)", m_thread_pool.maxThreadCount());
    }
}

void age::test_application::about_to_quit()
{
    // remove all tests that are still queued to let the application
    // terminate as fast as possible
    m_thread_pool.clear();

    int remaining = m_tests_running.size();
    if (remaining > 0)
    {
        qInfo("cancelled %d queued tests", remaining);
    }
}



void age::test_application::test_passed(QString test_file, QString pass_message)
{
    // save the result
    m_pass_messages.append(test_message(test_file, pass_message));
    m_tests_running.remove(test_file);

    // check if all tests are finished
    exit_app_on_finish();
}

void age::test_application::test_failed(QString test_file, QString fail_message)
{
    // save the result
    m_fail_messages.append(test_message(test_file, fail_message));
    m_tests_running.remove(test_file);

    // check if all tests are finished
    exit_app_on_finish();
}





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

bool age::test_application::find_files(QSet<QString> &files) const
{
    bool success = true;

    // if a test file has been specified, use it
    // (if the user specified a file and not a directory, do not
    // match the file name against m_test_file_pattern)
    QFileInfo file_info(m_test);
    if (file_info.isFile())
    {
        files.insert(m_test);
    }

    // if a directory was specified, search it for test files
    else if (file_info.isDir())
    {
        qInfo("looking for test files in directory %s", qPrintable(m_test));
        find_files(file_info, files);
    }

    // otherwise fail
    else
    {
        success = false;
    }

    return success;
}

void age::test_application::find_files(const QFileInfo &file_info, QSet<QString> &files) const
{
    // traverse directory
    if (file_info.isDir())
    {
        QDir dir = file_info.absoluteFilePath();

        // warn, if this directory is not readable
        if (!dir.isReadable())
        {
            qWarning("directory not readable: %s", qPrintable(dir.absolutePath()));
        }

        // if the directory is readable, check it's contents
        else
        {
            QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
            for (QFileInfo entry : entries)
            {
                find_files(entry, files);
            }
        }
    }

    // file match?
    else if (file_info.isFile() && m_test_file_pattern.exactMatch(file_info.absoluteFilePath()))
    {
        files.insert(file_info.absoluteFilePath());
    }
}



bool age::test_application::ignore_files(QSet<QString> &files) const
{
    QStringList files_to_ignore;
    bool success = true;

    // try reading the ignore list
    if (!m_ignore_file.isEmpty())
    {
        // terminate with an error, if we cannot open the file for reading
        QFile file(m_ignore_file);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            success = false;
        }

        // read the ignore list
        else
        {
            qInfo("reading ignore list from file %s", qPrintable(file.fileName()));

            QTextStream stream(&file);
            while (!stream.atEnd())
            {
                QString line = stream.readLine();
                line = line.trimmed();

                // ignore empty lines and lines beginning with a '#'
                if (!line.isEmpty() && !line.startsWith('#'))
                {
                    files_to_ignore.append(line);
                }
            }

            int entries = files_to_ignore.size();
            qInfo("ignore list contains %d %s", entries, ((entries == 1) ? "entry" : "entries"));
        }
    }

    // remove ignored files, if there was no error reading the ignore list
    QStringList ignored_files;
    if (!files_to_ignore.isEmpty() && success)
    {
        QMutableSetIterator<QString> itr(files);
        while (itr.hasNext())
        {
            QString file = itr.next();
            // check each ignore list entry for this file
            for (QString to_ignore : files_to_ignore)
            {
                if (file.contains(to_ignore))
                {
                    itr.remove();
                    ignored_files.append(file);
                    break;
                }
            }
        }
    }

    // tell the user about ignored files
    ignored_files.sort();
    print_message_list("ignoring the following file(s):", ignored_files);

    return success;
}



age::gb_emulator_test* age::test_application::create_test(const QString &test_file) const
{
    gb_emulator_test* result;

    switch(m_type)
    {
        case test_type::mooneye_test:
            result = new gb_emulator_test_mooneye(test_file);
            break;

        case test_type::gambatte_test:
            qInfo("wrong type, gambatte_test");
            ::exit(1);

        case test_type::screenshot_test:
            qInfo("wrong type, screenshot_test");
            ::exit(1);
    }

    return result;
}



QString age::test_application::test_message(const QString &test_file, const QString &message) const
{
    // don't append a white space to test_file if there is no message
    return message.isEmpty() ? test_file : (test_file + ' ' + message);
}

QString age::test_application::number_of_tests_message(QString message, int number_of_tests, int total) const
{
    QString result = "\n";
    result += QString::number(number_of_tests);
    result += (number_of_tests == 1) ? " test " : " tests ";
    result += message;

    if (total > 0)
    {
        result += " (";
        result += QString::number(number_of_tests * 100 / total);
        result += "%)";
    }

    result += ':';
    return result;
}



void age::test_application::exit_app_on_finish()
{
    // if all tests finished, we can exit the application
    if (m_tests_running.isEmpty())
    {
        qInfo("\rall tests executed                 ");

        // sort results alphabetically for better readability
        m_pass_messages.sort(Qt::CaseInsensitive);
        m_fail_messages.sort(Qt::CaseInsensitive);

        // print a test summary
        int total = m_pass_messages.size() + m_fail_messages.size();
        print_message_list(number_of_tests_message("passed", m_pass_messages.size(), total), m_pass_messages);
        print_message_list(number_of_tests_message("failed", m_fail_messages.size(), total), m_fail_messages);

        // calculate the return code and trigger exiting the application
        int return_code = m_fail_messages.isEmpty() ? EXIT_SUCCESS : EXIT_FAILURE;
        QCoreApplication::exit(return_code);
    }

    // if tests are still running, print the current status
    else
    {
        printf("\r%d tests not yet finished          ", m_tests_running.size());
    }
}

void age::test_application::print_message_list(const QString &first_line, const QStringList &message_list) const
{
    if (!message_list.isEmpty())
    {
        qInfo("%s", qPrintable(first_line));
        for (QString message : message_list)
        {
            qInfo("    %s", qPrintable(message));
        }
    }
}
