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
#ifndef NDEBUG
#define AGE_DEBUG
#endif



#ifdef AGE_DEBUG

// we skip the following includes for doxygen output since they would bloat the include graphs
//! \cond

#include <bitset>
#include <cassert>
#include <iomanip> // std::quoted, etc.
#include <ios>     // std::hex
#include <sstream> // std::stringstream
#include <string>

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
        template<typename Value>
        concurrent_cout& operator<<(const Value& t)
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



#define AGE_ASSERT(x)             assert(x);
#define AGE_ASSERT_ONE_BIT_SET(x) AGE_ASSERT(((x) > 0) && (((x) & ((x) -1)) == 0));

#define AGE_LOG(x)        (age::concurrent_cout() << age::age_log_time() << " " << x).log_line(); // NOLINT(bugprone-macro-parentheses)
#define AGE_LOG_QUOTED(x) std::quoted(x)

#define AGE_LOG_DEC(x)  static_cast<int64_t>(x)
#define AGE_LOG_DEC8(x) std::setw(8) << AGE_LOG_DEC(x) << std::setw(0)

#define AGE_LOG_HEX_X(x, width)                                                \
    "0x"                                                                       \
        << std::hex << std::uppercase << std::setw(width) << std::setfill('0') \
        << static_cast<uint64_t>(x)                                            \
        << std::setfill(' ') << std::setw(0) << std::nouppercase << std::dec   \
        << " (" << AGE_LOG_DEC(x) << ")"

#define AGE_LOG_HEX(x)   AGE_LOG_HEX_X(x, 0)
#define AGE_LOG_HEX8(x)  AGE_LOG_HEX_X(x, 2)
#define AGE_LOG_HEX16(x) AGE_LOG_HEX_X(x, 4)
#define AGE_LOG_HEX32(x) AGE_LOG_HEX_X(x, 8)

#define AGE_LOG_BIN16(x)                                        \
    std::bitset<4>{static_cast<unsigned>(x) >> 12}              \
        << "'" << std::bitset<4>{static_cast<unsigned>(x) >> 8} \
        << "'" << std::bitset<4>{static_cast<unsigned>(x) >> 4} \
        << "'" << std::bitset<4> { static_cast<unsigned>(x) }

#else // #ifdef AGE_DEBUG

#define AGE_ASSERT(x)
#define AGE_ASSERT_ONE_BIT_SET(x)

#define AGE_LOG(x)
#define AGE_LOG_QUOTED(x)

#define AGE_LOG_DEC(x)
#define AGE_LOG_DEC8(x)

#define AGE_LOG_HEX(x)
#define AGE_LOG_HEX8(x)
#define AGE_LOG_HEX16(x)
#define AGE_LOG_HEX32(x)

#define AGE_LOG_BIN16(x)

#endif // #ifdef AGE_DEBUG



#endif // AGE_DEBUG_HPP
