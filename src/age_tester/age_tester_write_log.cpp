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

#include "age_tester_write_log.hpp"

#include <algorithm>
#include <bitset>
#include <fstream>
#include <sstream>



namespace
{
    std::string format_log_entry(const age::gb_log_entry& entry)
    {
        unsigned div = entry.m_clock + entry.m_div_offset;

        std::bitset<4> div4{div >> 12};
        std::bitset<4> div3{div >> 8};
        std::bitset<4> div2{div >> 4};
        std::bitset<4> div1{div};

        std::stringstream prefix_str;
        prefix_str << std::setw(8) << entry.m_clock << std::setw(0)
                   << "  " << div4 << "'" << div3 << "'" << div2 << "'" << div1
                   << " : ";

        auto prefix = prefix_str.str();

        std::stringstream result;

        auto start = 0U;
        auto end = entry.m_message.find('\n');

        while (end != std::string::npos)
        {
            auto line = entry.m_message.substr(start, end - start);
            result << prefix << line << std::endl;

            start = end + 1;
            end = entry.m_message.find('\n', start);
        }

        result << prefix << entry.m_message.substr(start, end) << std::endl;

        return result.str();
    }

} // namespace



void age::tester::write_log(const std::filesystem::path& log_path, const std::vector<gb_log_entry>& log_entries)
{
    std::ofstream file(log_path);

    std::for_each(begin(log_entries),
                  end(log_entries),
                  [&](const auto& entry) {
                      file << format_log_entry(entry);
                  });
}
