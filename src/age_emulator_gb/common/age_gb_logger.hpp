//
// Copyright 2021 Christoph Sprenger
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

#ifndef AGE_GB_LOGGER_HPP
#define AGE_GB_LOGGER_HPP

//!
//! \file
//!

#include <emulator/age_gb_types.hpp>

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>



// We rely on the compiler to optimize away all logging code
// if AGE_COMPILE_LOGGER is not defined.
//
// To allow the compiler to optimize this during compile time
// and not rely on link time optimization,
// the logging code is header-only.
// (=> optimizations are visible in compiler generated assembly listings)



namespace age
{
    struct log_detail
    {
    };

    struct log_hex8
    {
        explicit log_hex8(int value) : m_value(value) {}
        int m_value;
    };

    struct log_hex16
    {
        explicit log_hex16(int value) : m_value(value) {}
        int m_value;
    };

    class gb_log_message_stream
    {
#ifdef AGE_COMPILE_LOGGER
        std::stringstream m_stream;
#endif

        void stream_hex(int value, int chars)
        {
#ifdef AGE_COMPILE_LOGGER
            m_stream << "0x"
                     << std::hex << std::uppercase << std::setw(chars) << std::setfill('0')
                     << value
                     << std::setfill(' ') << std::setw(0) << std::nouppercase << std::dec
                     << " (" << value << ")";
#else
            AGE_UNUSED(value);
            AGE_UNUSED(chars);
#endif
        }

    public:
        template<typename T>
        gb_log_message_stream& operator<<(const T& value)
        {
#ifdef AGE_COMPILE_LOGGER
            m_stream << value;
#else
            AGE_UNUSED(value);
#endif
            return *this;
        }

        gb_log_message_stream& operator<<(const log_hex8& value)
        {
            stream_hex(value.m_value, 2);
            return *this;
        }

        gb_log_message_stream& operator<<(const log_hex16& value)
        {
            stream_hex(value.m_value, 4);
            return *this;
        }

        gb_log_message_stream& operator<<(const log_detail& value)
        {
            AGE_UNUSED(value);
#ifdef AGE_COMPILE_LOGGER
            m_stream << "\n    * ";
#endif
            return *this;
        }

        std::string operator()() const
        {
#ifdef AGE_COMPILE_LOGGER
            return m_stream.str();
#else
            return "";
#endif
        }
    };



    class gb_logger
    {
    public:
        [[nodiscard]] gb_log_message_stream& log(gb_log_type type, int clock, int div_offset)
        {
#ifdef AGE_COMPILE_LOGGER
            finish_pending_entry();
            m_pending_entry  = {type, clock, div_offset, ""};
            m_pending_stream = {};
#else
            AGE_UNUSED(type);
            AGE_UNUSED(clock);
            AGE_UNUSED(div_offset);
#endif
            return m_pending_stream;
        }

        [[nodiscard]] const std::vector<gb_log_entry>& get_log_entries()
        {
            finish_pending_entry();
            return m_log_entries;
        }

    private:
        void finish_pending_entry()
        {
#ifdef AGE_COMPILE_LOGGER
            std::string str = m_pending_stream();
            if (!str.empty())
            {
                m_pending_entry.m_message = str;
                m_log_entries.emplace_back(m_pending_entry);
            }
#endif
        }

        std::vector<gb_log_entry> m_log_entries;
        gb_log_entry              m_pending_entry{gb_log_type::LT_CLOCK, 0, 0, ""};
        gb_log_message_stream     m_pending_stream;
    };

} // namespace age



#endif // AGE_GB_LOGGER_HPP
