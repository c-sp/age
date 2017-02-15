
#include "age_test_runner.hpp"



//---------------------------------------------------------
//
//   object creation & destruction
//
//---------------------------------------------------------

age::test_runner_application::test_runner_application(const QString &test, const QString &ignore_file, test_type type)
    : m_test_file_pattern(".*\\.gb[c]?", Qt::CaseInsensitive),
      m_test(test),
      m_ignore_file(ignore_file),
      m_type(type)
{
}

age::test_runner_application::~test_runner_application()
{
    m_thread_pool.clear();
}





//---------------------------------------------------------
//
//   public methods & slots
//
//---------------------------------------------------------

void age::test_runner_application::schedule_tests()
{
    // look for test files to execute
    if (!find_files())
    {
        fprintf(stderr, "%s is neither a file nor a directory\n", qPrintable(m_test));
        QCoreApplication::exit(EXIT_FAILURE);
    }

    // filter test files to ignore
    else if (!ignore_files())
    {
        fprintf(stderr, "could not read ignore list from file %s\n", qPrintable(m_ignore_file));
        QCoreApplication::exit(EXIT_FAILURE);
    }

    // terminate, if there are no tests to execute
    else if (m_test_files.isEmpty())
    {
        qInfo("no tests found, terminating");
        QCoreApplication::exit(EXIT_SUCCESS);
    }

    // schedule tests
    else
    {
        qInfo("scheduling %d test(s) for execution", m_test_files.size());

        for (QString file : m_test_files)
        {
            emulator_test *test = new emulator_test(file);
            test->setAutoDelete(true); // let QThreadPool clean this up

            // connect via QueuedConnection since the signal is emitted across thread boundaries
            connect(test, SIGNAL(test_passed(QString)), this, SLOT(test_passed(QString)), Qt::QueuedConnection);
            connect(test, SIGNAL(test_failed(QString,QString)), this, SLOT(test_failed(QString,QString)), Qt::QueuedConnection);

            m_thread_pool.start(test);
        }
    }
}



void age::test_runner_application::test_passed(QString test_file)
{
    qInfo("test passed:  %s", qPrintable(test_file));
    m_test_files.remove(test_file);
    check_finish();
}

void age::test_runner_application::test_failed(QString test_file, QString reason)
{
    qInfo("test failed:  %s, %s", qPrintable(test_file), qPrintable(reason));
    m_test_files.remove(test_file);
    check_finish();
}

void age::test_runner_application::about_to_quit()
{
    m_thread_pool.clear();

    int remaining = m_test_files.size();
    if (remaining > 0)
    {
        qInfo("cancelled %d queued tests", remaining);
    }
}





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

bool age::test_runner_application::find_files()
{
    bool success = true;

    // if a test file has been specified, use it
    // (if the user specified a file and not a directory, do not
    // match the file name against m_test_file_pattern)
    QFileInfo file_info(m_test);
    if (file_info.isFile())
    {
        m_test_files.insert(m_test);
    }

    // if a directory was specified, search it for test files
    else if (file_info.isDir())
    {
        qInfo("looking for test files in directory %s", qPrintable(m_test));
        find_files(file_info);
    }

    // otherwise fail
    else
    {
        success = false;
    }

    return success;
}

void age::test_runner_application::find_files(const QFileInfo &file_info)
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
                find_files(entry);
            }
        }
    }

    // file match?
    else if (file_info.isFile() && m_test_file_pattern.exactMatch(file_info.absoluteFilePath()))
    {
        m_test_files.insert(file_info.absoluteFilePath());
    }
}



bool age::test_runner_application::ignore_files()
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
                files_to_ignore.append(line.trimmed());
            }
        }
    }

    // remove ignored files, if there was no error reading the ignore list
    QStringList ignored_files;
    if (!files_to_ignore.isEmpty() && success)
    {
        QMutableSetIterator<QString> itr(m_test_files);
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
    if (!ignored_files.isEmpty())
    {
        qInfo("ignoring the following file(s):");
        for (QString file : ignored_files)
        {
            qInfo("    %s", qPrintable(file));
        }
    }

    return success;
}



void age::test_runner_application::check_finish()
{
    if (m_test_files.isEmpty())
    {
        qInfo("all tests executed, terminating");
        QCoreApplication::exit(EXIT_SUCCESS);
    }
}
