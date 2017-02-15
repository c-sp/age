
#include "age_test.hpp"



age::emulator_test::emulator_test(const QString &test_file)
    : m_test_file(test_file)
{
}



void age::emulator_test::run()
{
    QString error_cause;

    // load test file and create emulator

    // run the test

    // emit result event
    if (error_cause.isEmpty())
    {
        emit test_passed(m_test_file);
    }
    else
    {
        emit test_failed(m_test_file, error_cause);
    }
}
