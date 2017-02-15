#ifndef AGE_TEST_RUNNER_HPP
#define AGE_TEST_RUNNER_HPP

//!
//! \file
//!

#include "age_test.hpp"



namespace age
{

enum class test_type : int
{
    screenshot_test, // this is the default
    mooneye_test,
    gambatte_test
};



template<typename _T>
class optional
{
public:

    bool is_set() const
    {
        return m_value_set;
    }

    _T get(_T default_value) const
    {
        return m_value_set ? m_value : default_value;
    }

    void set(_T t)
    {
        m_value = t;
        m_value_set = true;
    }

private:

    bool m_value_set = false;
    _T m_value;
};



class test_runner_application : public QObject, non_copyable
{
    Q_OBJECT
public:

    test_runner_application(const QString &test, const QString &ignore_file, test_type type);
    ~test_runner_application() override;

public slots:

    void schedule_tests();
    void test_passed(QString test_file);
    void test_failed(QString test_file, QString reason);
    void about_to_quit();

private:

    bool find_files();
    void find_files(const QFileInfo &file_info);
    bool ignore_files();

    void check_finish();

    const QRegExp m_test_file_pattern;
    const QString m_test;
    const QString m_ignore_file;
    const test_type m_type;

    QThreadPool m_thread_pool;
    QSet<QString> m_test_files;
};

} // namespace age



#endif // AGE_TEST_RUNNER_HPP
