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

#include <algorithm>
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
    template<typename T, typename U>
    struct log_in_clks
    {
        explicit log_in_clks(T clock_cycles, U current_clock_cycle)
            : m_clock_cycles(clock_cycles),
              m_current_clock_cycle(current_clock_cycle)
        {}
        T m_clock_cycles;
        U m_current_clock_cycle;
    };

    template<int BITS, typename T>
    struct log_hex_v
    {
        static_assert(BITS % 4 == 0);
        explicit log_hex_v(T value) : m_value(value) {}
        T m_value;
    };

    template<typename T>
    log_hex_v<0, T> log_hex(T value)
    {
        return log_hex_v<0, T>(value);
    }

    template<typename T>
    log_hex_v<8, T> log_hex8(T value)
    {
        return log_hex_v<8, T>(value);
    }

    template<typename T>
    log_hex_v<16, T> log_hex16(T value)
    {
        return log_hex_v<16, T>(value);
    }

    template<typename T>
    struct log_dec
    {
        explicit log_dec(T value) : m_value(value) {}
        T m_value;
    };

    class gb_log_message_stream
    {
#ifdef AGE_COMPILE_LOGGER
        std::stringstream m_stream;

        template<typename T>
        void stream_hex(T value, int chars)
        {
            auto v = static_cast<uint64_t>(value);
            m_stream << "0x"
                     << std::hex << std::uppercase << std::setw(chars) << std::setfill('0')
                     << v
                     << std::setfill(' ') << std::setw(0) << std::nouppercase << std::dec
                     << " (" << v << ")";
        }

    public:
        template<typename T, typename U>
        gb_log_message_stream& operator<<(const log_in_clks<T, U>& value)
        {
            m_stream << static_cast<int64_t>(value.m_clock_cycles) << " clock cycles"
                     << " (on clock cycle " << static_cast<int64_t>(value.m_current_clock_cycle + value.m_clock_cycles) << ")";
            return *this;
        }

        template<typename T>
        gb_log_message_stream& operator<<(const log_dec<T>& value)
        {
            m_stream << static_cast<int64_t>(value.m_value);
            return *this;
        };


        template<int BITS, typename T>
        gb_log_message_stream& operator<<(const log_hex_v<BITS, T>& value)
        {
            stream_hex(value.m_value, BITS / 4);
            return *this;
        }

        template<typename T>
        gb_log_message_stream& operator<<(const T& value)
        {
            m_stream << value;
            return *this;
        }

        std::string operator()() const
        {
            return m_stream.str();
        }
#else
    public:
        template<typename T>
        gb_log_message_stream& operator<<(const T& value)
        {
            AGE_UNUSED(value);
            return *this;
        }
#endif
    };



    class gb_logger
    {
#ifdef AGE_COMPILE_LOGGER
    public:
        explicit gb_logger(gb_log_categories log_categories) : m_log_categories(std::move(log_categories)) {}

        [[nodiscard]] gb_log_message_stream& log(gb_log_category category, int clock, int div_offset)
        {
            m_messages.emplace_back(active_stream{category, clock, div_offset});
            return (m_messages.end() - 1)->m_stream;
        }

        [[nodiscard]] std::vector<gb_log_entry> get_log_entries() const
        {
            std::vector<gb_log_entry> result;

            //! \todo filter categories
            std::transform(begin(m_messages),
                           end(m_messages),
                           std::back_inserter(result),
                           [](auto& message) {
                               return gb_log_entry(message.m_category, message.m_clock, message.m_div_offset, message.m_stream());
                           });

            return result;
        }

    private:
        struct active_stream
        {
            active_stream(gb_log_category category, int clock, int div_offset)
                : m_category(category),
                  m_clock(clock),
                  m_div_offset(div_offset)
            {}
            gb_log_category       m_category;
            int                   m_clock;
            int                   m_div_offset;
            gb_log_message_stream m_stream;
        };

        gb_log_categories          m_log_categories;
        std::vector<active_stream> m_messages;

#else
    public:
        explicit gb_logger(gb_log_categories log_categories) { AGE_UNUSED(log_categories); }

        [[nodiscard]] gb_log_message_stream& log(gb_log_category category, int clock, int div_offset)
        {
            AGE_UNUSED(category);
            AGE_UNUSED(clock);
            AGE_UNUSED(div_offset);
            return m_dummy_stream;
        }

        [[nodiscard]] const std::vector<gb_log_entry>& get_log_entries()
        {
            return m_no_entries;
        }

    private:
        std::vector<gb_log_entry> m_no_entries;
        gb_log_message_stream     m_dummy_stream;
#endif
    };

} // namespace age



#endif // AGE_GB_LOGGER_HPP
