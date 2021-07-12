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
#include <memory>
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
    template<typename Cycles, typename Clock>
    struct log_in_clks
    {
        explicit log_in_clks(Cycles clock_cycles, Clock current_clock_cycle)
            : m_clock_cycles(clock_cycles),
              m_current_clock_cycle(current_clock_cycle)
        {}
        Cycles m_clock_cycles;
        Clock  m_current_clock_cycle;
    };

    template<int Bits, typename Value>
    struct log_hex_v
    {
        static_assert(Bits % 4 == 0);
        explicit log_hex_v(Value value) : m_value(value) {}
        Value m_value;
    };

    template<typename Value>
    log_hex_v<0, Value> log_hex(Value value)
    {
        return log_hex_v<0, Value>(value);
    }

    template<typename Value>
    log_hex_v<8, Value> log_hex8(Value value)
    {
        return log_hex_v<8, Value>(value);
    }

    template<typename Value>
    log_hex_v<16, Value> log_hex16(Value value)
    {
        return log_hex_v<16, Value>(value);
    }

    template<typename Value>
    struct log_dec
    {
        explicit log_dec(Value value) : m_value(value) {}
        Value m_value;
    };

    class gb_log_message_stream
    {
#ifdef AGE_COMPILE_LOGGER
        std::unique_ptr<std::ostringstream> m_stream;
        gb_log_entry*                       m_entry;

    public:
        explicit gb_log_message_stream(gb_log_entry* entry) : m_stream(new std::ostringstream),
                                                              m_entry(entry) {}

        ~gb_log_message_stream()
        {
            if (m_entry)
            {
                m_entry->m_message = m_stream->str();
            }
            // discard stream to explicitly fail in any use-after-free situation
            // (e.g. auto& msg = log() << "foo"; msg << "bar";)
            m_stream = nullptr;
        }

        gb_log_message_stream(const gb_log_message_stream&) = delete;
        gb_log_message_stream(gb_log_message_stream&&)      = default;

        gb_log_message_stream& operator=(const gb_log_message_stream&) = delete;
        gb_log_message_stream& operator=(gb_log_message_stream&&) = default;

        template<typename Cycles, typename Clock>
        gb_log_message_stream& operator<<(const log_in_clks<Cycles, Clock>& value)
        {
            *m_stream << static_cast<int64_t>(value.m_clock_cycles) << " clock cycles"
                      << " (on clock cycle " << static_cast<int64_t>(value.m_current_clock_cycle + value.m_clock_cycles) << ")";
            return *this;
        }

        template<typename Value>
        gb_log_message_stream& operator<<(const log_dec<Value>& value)
        {
            *m_stream << static_cast<int64_t>(value.m_value);
            return *this;
        };


        template<int Bits, typename Value>
        gb_log_message_stream& operator<<(const log_hex_v<Bits, Value>& value)
        {
            auto chars = Bits / 4;
            auto v     = static_cast<uint64_t>(value.m_value);

            *m_stream << "0x"
                      << std::hex << std::uppercase << std::setw(chars) << std::setfill('0')
                      << v
                      << std::setfill(' ') << std::setw(0) << std::nouppercase << std::dec
                      << " (" << v << ")";

            return *this;
        }

        template<typename Value>
        gb_log_message_stream& operator<<(const Value& value)
        {
            *m_stream << value;
            return *this;
        }
#else
    public:
        template<typename Value>
        gb_log_message_stream& operator<<(const Value& value)
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
        explicit gb_logger(gb_log_categories log_categories = {}) : m_log_categories(std::move(log_categories)) {}

        [[nodiscard]] gb_log_message_stream log(gb_log_category category, int clock, int div_clock)
        {
            if (m_log_categories.find(category) == end(m_log_categories))
            {
                return gb_log_message_stream(nullptr);
            }
            m_messages.emplace_back(gb_log_entry{category, clock, div_clock, ""});
            return gb_log_message_stream(&m_messages[m_messages.size() - 1]);
        }

        [[nodiscard]] const std::vector<gb_log_entry>& get_log_entries() const
        {
            return m_messages;
        }

    private:
        gb_log_categories         m_log_categories;
        std::vector<gb_log_entry> m_messages;

#else
    public:
        explicit gb_logger(gb_log_categories log_categories = {}) { AGE_UNUSED(log_categories); }

        [[nodiscard]] gb_log_message_stream log(gb_log_category category, int clock, int div_offset)
        {
            AGE_UNUSED(category);
            AGE_UNUSED(clock);
            AGE_UNUSED(div_offset);
            return gb_log_message_stream();
        }

        [[nodiscard]] const std::vector<gb_log_entry>& get_log_entries() const
        {
            return m_no_entries;
        }

    private:
        std::vector<gb_log_entry> m_no_entries;
#endif
    };

} // namespace age



#endif // AGE_GB_LOGGER_HPP
