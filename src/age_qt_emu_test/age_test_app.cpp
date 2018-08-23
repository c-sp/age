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

#include <cmath>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include "age_test_app.hpp"





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
    m_test_performance = std::make_shared<test_performance>();
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
        for (QString file : files)
        {
            QString result_file;
            test_method method;
            int scheduled = 0;

            switch(m_type)
            {
                case test_type::blargg_test:
                    method = blargg_dmg_test(file, result_file);
                    scheduled += schedule_test(file, method, result_file);
                    method = blargg_cgb_test(file, result_file);
                    scheduled += schedule_test(file, method, result_file);
                    break;

                case test_type::mooneye_test:
                    method = mooneye_test_method(file);
                    scheduled += schedule_test(file, method);
                    break;

                case test_type::gambatte_test:
                    method = gambatte_dmg_test(file, result_file);
                    scheduled += schedule_test(file, method, result_file);
                    method = gambatte_cgb_test(file, result_file);
                    scheduled += schedule_test(file, method, result_file);
                    break;
            }

            if (scheduled < 1)
            {
                m_no_test_method_found.append(file);
            }
        }

        qInfo("scheduled %d test(s) for execution", m_tests_running);
        qInfo("thread pool has %d thread(s)", m_thread_pool.maxThreadCount());

        // if we did not schedule any tests, exist immediately
        exit_app_on_finish();
    }
}

void age::test_application::about_to_quit()
{
    // remove all tests that are still queued to let the application
    // terminate as fast as possible
    m_thread_pool.clear();

    if (m_tests_running > 0)
    {
        qInfo("cancelled %d queued tests", m_tests_running);
    }
}



void age::test_application::test_passed(QString test_file, QString pass_message)
{
    // save the result
    m_pass_messages.append(test_message(test_file, pass_message));
    --m_tests_running;

    // check if all tests are finished
    exit_app_on_finish();
}

void age::test_application::test_failed(QString test_file, QString fail_message)
{
    // save the result
    m_fail_messages.append(test_message(test_file, fail_message));
    --m_tests_running;

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
                    // trim end-of-line comments
                    auto idx = line.indexOf('#');
                    if (idx >= 0) {
                        line = line.left(idx).trimmed();
                    }

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
    print_list("ignoring the following file(s):", ignored_files);

    return success;
}



int age::test_application::schedule_test(const QString &file_name, test_method method, const QString &result_file_name)
{
    int result = 0;
    if (method)
    {
        test_runner *test = new test_runner(file_name, result_file_name, method, m_test_performance);
        test->setAutoDelete(true); // let QThreadPool clean this up

        // connect via QueuedConnection since the signal is emitted across thread boundaries
        // (the respective slot will be executed by this thread after the current method returns)
        connect(test, SIGNAL(test_passed(QString,QString)), this, SLOT(test_passed(QString,QString)), Qt::QueuedConnection);
        connect(test, SIGNAL(test_failed(QString,QString)), this, SLOT(test_failed(QString,QString)), Qt::QueuedConnection);

        m_thread_pool.start(test);
        ++m_tests_running;
        result = 1;
    }
    return result;
}



void age::test_application::exit_app_on_finish()
{
    // if all tests finished, we can exit the application
    if (m_tests_running <= 0)
    {
        int tests_executed = m_pass_messages.size() + m_fail_messages.size();
        int tests = tests_executed + m_no_test_method_found.size();

        qInfo("%d test(s) executed", tests_executed);

        // sort results alphabetically for better readability
        m_pass_messages.sort(Qt::CaseInsensitive);
        m_fail_messages.sort(Qt::CaseInsensitive);
        m_no_test_method_found.sort(Qt::CaseInsensitive);

        // print a test summary
        //print_list(number_of_tests_message("passed", m_pass_messages.size(), tests), m_pass_messages);
        print_list(number_of_tests_message("failed", m_fail_messages.size(), tests), m_fail_messages);
        print_list(number_of_tests_message("failed with unknown type", m_no_test_method_found.size(), tests), m_no_test_method_found);

        int length = 1 + static_cast<int>(std::log10(tests));
        QString format;
        QTextStream(&format) << "%16s: %" << length << "d of %" << length << "d, %3d%%";

        qInfo("\ntest summary:");
        qInfo(qPrintable(format), "passed", m_pass_messages.size(), tests, percent(m_pass_messages.size(), tests));
        qInfo(qPrintable(format), "failed", m_fail_messages.size(), tests, percent(m_fail_messages.size(), tests));
        qInfo(qPrintable(format), "unknown type", m_no_test_method_found.size(), tests, percent(m_no_test_method_found.size(), tests));

        if (m_test_performance->summary_available())
        {
            m_test_performance->print_summary();
        }

        // calculate the return code and trigger exiting the application
        bool success = m_fail_messages.isEmpty() && m_no_test_method_found.isEmpty();
        QCoreApplication::exit(success ? EXIT_SUCCESS : EXIT_FAILURE);
    }
}



QString age::test_application::test_message(const QString &test_file, const QString &message) const
{
    // don't append a white space to test_file if there is no message
    return message.isEmpty() ? test_file : (test_file + ' ' + message);
}

QString age::test_application::number_of_tests_message(QString message, int number_of_tests, int total) const
{
    QString result;
    QTextStream(&result)
           << "\n" << number_of_tests << " of " << total
           << ((number_of_tests == 1) ? " test " : " tests ")
           << message
           << " (" << percent(number_of_tests, total) << "%):";

    return result;
}

void age::test_application::print_list(const QString &first_line, const QStringList &message_list) const
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

int age::test_application::percent(int value, int total)
{
    return (0 == total) ? 0 : value * 100 / total;
}
