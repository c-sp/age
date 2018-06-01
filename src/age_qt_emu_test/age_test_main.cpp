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

constexpr const char *arg_type = "type";
constexpr const char *arg_ignore_list = "ignore-list";



//---------------------------------------------------------
//
//   utility methods used by main()
//
//---------------------------------------------------------

age::optional<age::test_type> parse_test_type(const QString &type_string)
{
    age::optional<age::test_type> result;

    if (type_string.compare("screenshot") == 0)
    {
        result.set(age::test_type::screenshot_test);
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
                "test",
                "The test file to run. This must be a runnable Gameboy rom file"
                " or a directory to be searched for Gameboy rom files."
                );

    parser.addOptions({{
                           arg_type,
                           "The type of the specified test(s)."
                           " This must be one of: screenshot, mooneye, gambatte."
                           " If not specified, the type 'screenshot' is used.",
                           "test_type",
                           "screenshot"
                       },{
                           arg_ignore_list,
                           "A file containing names of test files to be ignored.",
                           "file"
                       }});

    parser.addHelpOption();
}



void validate_arguments(QCommandLineParser &parser)
{
    // if there is no positional argument available,
    // the user did not specify the test(s) to execute
    if (parser.positionalArguments().size() < 1)
    {
        fprintf(stderr, "no test specified\n");
        ::exit(EXIT_FAILURE);
    }

    // make sure the type (if specified) parameter can be parsed
    QString type = parser.value(arg_type);
    if (!type.isEmpty())
    {
        age::optional<age::test_type> parsed_type = parse_test_type(type);

        if (!parsed_type.is_set())
        {
            fprintf(stderr, "invalid test type: %s\n", qPrintable(type));
            ::exit(EXIT_FAILURE);
        }
    }
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

    validate_arguments(parser);

    // create & connect test runner application
    age::test_application test_runner = {
        parser.positionalArguments().at(0),
        parser.value(arg_ignore_list),
        parse_test_type(parser.value(arg_type)).get(age::test_type::screenshot_test)
    };
    QObject::connect(&app, SIGNAL(aboutToQuit()), &test_runner, SLOT(about_to_quit()));

    // "jump start" test runner application with a timer event
    QTimer::singleShot(1, &test_runner, SLOT(schedule_tests()));
    return app.exec();
}
