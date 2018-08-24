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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTimer>

#include "age_test_app.hpp"

constexpr const char *arg_ignore_list = "ignore-list";
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

    if (type_string.compare("blargg") == 0)
    {
        result.set(age::test_type::blargg_test);
    }
    else if (type_string.compare("mooneye") == 0)
    {
        result.set(age::test_type::mooneye_test);
    }
    else if (type_string.compare("gambatte") == 0)
    {
        result.set(age::test_type::gambatte_test);
    }

    return result;
}



void prepare_parser(QCommandLineParser &parser)
{
    parser.setApplicationDescription(
                "The AGE emulator test runner can execute Gameboy test roms"
                " and validate the test result."
                );

    parser.addPositionalArgument(
                arg_test_type,
                "The type of the specified test(s)."
                " This must be one of: blargg, mooneye, gambatte."
                );

    parser.addPositionalArgument(
                arg_test_file,
                "The test file to run. This must be a runnable Gameboy rom file"
                " or a directory to be searched for Gameboy rom files."
                );

    parser.addOptions({{
                           arg_ignore_list,
                           "A file containing names of test files to be ignored.",
                           "file"
                       }});

    parser.addHelpOption();
}



void exit_with_failure(const QCommandLineParser &parser, const QString &error_message)
{
    fprintf(stderr, qPrintable(error_message));
    fprintf(stderr, "\n");
    fprintf(stdout, qPrintable(parser.helpText()));
    ::exit(EXIT_FAILURE);
}



age::test_type validate_arguments(const QCommandLineParser &parser)
{
    // if there is no positional argument available,
    // the user did not specify the test-type
    if (parser.positionalArguments().size() < 1)
    {
        exit_with_failure(parser, QString("no ").append(arg_test_type).append(" specified"));
    }

    // parse the test-type
    QString test_type_string = parser.positionalArguments().at(0);
    age::optional<age::test_type> test_type = parse_test_type(test_type_string);
    if (!test_type.is_set())
    {
        exit_with_failure(parser, QString("invalid ").append(arg_test_type).append(" '").append(test_type_string).append("'"));
    }

    // if there is only one positional argument available,
    // the user did not specify the test(s) to execute
    if (parser.positionalArguments().size() < 2)
    {
        exit_with_failure(parser, QString("no ").append(arg_test_file).append(" specified"));
    }

    return test_type.get(age::test_type::blargg_test); // the default is never used here
}



//---------------------------------------------------------
//
//   main method
//
//---------------------------------------------------------

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("AGE emulator test runner");

    // parse & validate arguments
    QCommandLineParser parser;
    prepare_parser(parser);
    parser.process(app);

    age::test_type test_type = validate_arguments(parser);

    // create & connect test runner application
    age::test_application test_runner = {
        parser.positionalArguments().at(1),
        parser.value(arg_ignore_list),
        test_type
    };
    QObject::connect(&app, SIGNAL(aboutToQuit()), &test_runner, SLOT(about_to_quit()));

    // "jump start" test runner application with a timer event
    QTimer::singleShot(1, &test_runner, SLOT(schedule_tests()));
    return app.exec();
}
