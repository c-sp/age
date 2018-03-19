//
// Copyright (c) 2010-2018 Christoph Sprenger
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
#include <string>
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
//! using std::cout (provided all threads use concurrent_cout).
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
