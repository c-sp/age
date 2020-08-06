//
// Copyright 2020 Christoph Sprenger
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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTimer>

#include "age_test_app.hpp"

constexpr const char *arg_ignore_list = "ignore-list";
constexpr const char *arg_threads = "threads";
constexpr const char *arg_test_type = "test-type";
constexpr const char *arg_test_file = "test-file";



//---------------------------------------------------------
//
//   utility methods used by main()
//
//---------------------------------------------------------

age::optional<age::test_type> parse_test_type(const QString &type_string)
{
    age::optional<age::test_type> result;

    if (type_string.compare(age::blargg) == 0)
    {
        result.set(age::test_type::blargg_test);
    }
    else if (type_string.compare(age::mooneye_gb) == 0)
    {
        result.set(age::test_type::mooneye_test);
    }
    else if (type_string.compare(age::gambatte) == 0)
    {
        result.set(age::test_type::gambatte_test);
    }

    return result;
}



void prepare_parser(QCommandLineParser &parser)
{
    parser.setApplicationDescription(QString("The ") + age::project_name +
                " test runner can execute Gameboy test roms"
                " and validate the test result."
                );

    parser.addPositionalArgument(
                arg_test_type,
                "The type of the specified test(s)."
                " This must be one of: blargg, mooneye, gambatte."
                );

    parser.addPositionalArgument(
                arg_test_file,
                "The test file to run."
                " This must be a runnable Gameboy rom file or a directory to"
                " be searched for Gameboy rom files."
                );

    parser.addOptions({{
                           arg_ignore_list,
                           "A file containing names of test files to be"
                           " ignored.",
                           "file"
                       }});

    parser.addOptions({{
                           arg_threads,
                           "The number of threads to use for running tests."
                           " If this is not specified the number of processor"
                           " cores will be used.",
                           "threads"
                       }});

    parser.addHelpOption();
}



[[ noreturn ]] void exit_with_failure(const QCommandLineParser &parser,
                                      const QString &error_message)
{
    fprintf(stderr, "%s\n", qPrintable(error_message));
    fprintf(stdout, "%s", qPrintable(parser.helpText()));
    ::exit(EXIT_FAILURE);
}



void validate_arguments(const QCommandLineParser &parser,
                        age::test_type &test_type,
                        int &num_threads)
{
    // if there is no positional argument available,
    // the user did not specify the test-type
    if (parser.positionalArguments().size() < 1)
    {
        auto msg = QString("no ").append(arg_test_type).append(" specified");
        exit_with_failure(parser, msg);
    }

    // parse the test-type
    QString test_type_string = parser.positionalArguments().at(0);
    age::optional<age::test_type> type = parse_test_type(test_type_string);
    if (!type.is_set())
    {
        auto msg = QString("invalid ").append(arg_test_type)
                .append(" '").append(test_type_string).append("'");
        exit_with_failure(parser, msg);
    }

    // if there is only one positional argument available,
    // the user did not specify the test(s) to execute
    if (parser.positionalArguments().size() < 2)
    {
        auto msg = QString("no ").append(arg_test_file).append(" specified");
        exit_with_failure(parser, msg);
    }

    // parse the number of threads, if specified
    int threads = 0;
    QString threads_string = parser.value(arg_threads);
    if (threads_string.length() > 0)
    {
        bool parsed = false;
        threads = threads_string.toInt(&parsed, 10);

        if (!parsed)
        {
            auto msg = QString("invalid: --threads ").append(threads_string);
            exit_with_failure(parser, msg);
            threads = 0;
        }
    }

    // the default value is never used as the test_type argument is mandatory
    test_type = type.get(age::test_type::blargg_test);
    num_threads = threads;
}



//---------------------------------------------------------
//
//   main method
//
//---------------------------------------------------------

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QString(age::project_name) + " test runner");

    // parse & validate arguments
    QCommandLineParser parser;
    prepare_parser(parser);
    parser.process(app);

    age::test_type test_type;
    int num_threads;
    validate_arguments(parser, test_type, num_threads);

    // create & connect test runner application
    age::test_application test_runner = {
        parser.positionalArguments().at(1),
        parser.value(arg_ignore_list),
        test_type,
        num_threads
    };
    QObject::connect(
                &app, SIGNAL(aboutToQuit()),
                &test_runner, SLOT(about_to_quit())
                );

    // "jump start" test runner application with a timer event
    QTimer::singleShot(1, &test_runner, SLOT(schedule_tests()));
    return app.exec();
}
