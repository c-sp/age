
#ifndef AGE_DEBUG_HPP
#define AGE_DEBUG_HPP

//!
//! \file
//!

//
// Compile for debugging?
// This will enable e.g. logging macros.
//
#if defined _DEBUG || defined DEBUG
#define AGE_DEBUG
#endif



#ifdef AGE_DEBUG

// we skip the following includes for doxygen output since they would bloat the include graphs
//! \cond

#include <cassert>
#include <chrono>
#include <iostream> // std::cout
#include <iomanip> // std::quoted
#include <sstream> // std::stringstream

#include <thread>
#include <mutex>

//! \endcond

namespace age
{

//!
//! Utility class to handle concurrent logging to std::cout.
//! It uses the usual fluent interface for passing values to an std::ostream using operator<<
//! but collects the logging output to some buffer until we explicitly tell it to pass it to std::cout.
//! Streaming to std::cout is done after locking a mutex, so that there is always only one thread
//! using st::cout (provided all threads use concurrent_cout).
//!
class concurrent_cout
{
public:

    template<class T>
    concurrent_cout& operator<<(const T &t)
    {
        m_ss << t;
        return *this;
    }

    void log_line();

private:

    static std::mutex M_mutex;
    std::stringstream m_ss;
};

//!
//! \brief Utility class for creating an std::string containing a human readable form of the current time.
//!
class age_log_time : public std::string
{
public:

    age_log_time();

private:

    static std::string get_timestamp();
};

} // namespace age



#define AGE_ASSERT(x) assert(x)
#define AGE_LOG(x) (age::concurrent_cout() << age::age_log_time() << " thread " << std::this_thread::get_id() << " " << __func__ << "  -  " << x).log_line()

#else // #ifdef AGE_DEBUG

#define AGE_ASSERT(x)
#define AGE_LOG(x)

#endif // #ifdef AGE_DEBUG



#endif // AGE_DEBUG_HPP
