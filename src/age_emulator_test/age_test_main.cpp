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

constexpr const char *arg_type = "type";
constexpr const char *arg_ignore_list = "ignore-list";



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
