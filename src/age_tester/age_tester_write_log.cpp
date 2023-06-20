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

#include <git_revision.hpp>

#include "age_tester_write_log.hpp"

#include <algorithm>
#include <bitset>
#include <ctime>
#include <fstream>
#include <sstream>



namespace
{
    std::string log_header(const std::filesystem::path& rom_path, age::gb_device_type device_type)
    {
        std::time_t t = std::time(nullptr);
        // NOLINTNEXTLINE(concurrency-mt-unsafe)
        auto* tm = std::gmtime(&t);

        std::stringstream result;
        result << "--------------------------------------------------------------------------------\n"
               << "   emulation logs for: " << rom_path.filename().string() << " " << age::tester::get_device_type_string(device_type) << '\n'
               << "           created on: " << std::put_time(tm, "%Y-%m-%dT%H:%M:%SZ") << '\n'
               << "     AGE git revision: " << GIT_REV << " (" << GIT_DATE << ")\n"
               << "--------------------------------------------------------------------------------\n"
               << '\n'
               << " T4-cycle  T4-16-bit-divider    category    comments" << '\n'
               << "---------  -------------------  ----------  -------------------------------------\n";

        return result.str();
    }



    std::string category_str(age::gb_log_category category)
    {
        switch (category)
        {
            case age::gb_log_category::lc_clock: return "clock";
            case age::gb_log_category::lc_cpu: return "cpu";
            case age::gb_log_category::lc_events: return "events";
            case age::gb_log_category::lc_hdma: return "hdma";
            case age::gb_log_category::lc_interrupts: return "interrupts";
            case age::gb_log_category::lc_lcd: return "lcd";
            case age::gb_log_category::lc_lcd_oam: return "lcd-oam";
            case age::gb_log_category::lc_lcd_oam_dma: return "lcd-oam-dma";
            case age::gb_log_category::lc_lcd_registers: return "lcd-reg";
            case age::gb_log_category::lc_lcd_vram: return "lcd-vram";
            case age::gb_log_category::lc_memory: return "memory";
            case age::gb_log_category::lc_serial: return "serial";
            case age::gb_log_category::lc_sound: return "sound";
            case age::gb_log_category::lc_sound_registers: return "sound-reg";
            case age::gb_log_category::lc_timer: return "timer";
        }
        return "";
    }



    std::string format_log_entry(const age::gb_log_entry& entry)
    {
        unsigned div = entry.m_div_clock;

        std::bitset<4> div4{div >> 12};
        std::bitset<4> div3{div >> 8};
        std::bitset<4> div2{div >> 4};
        std::bitset<4> div1{div};

        std::stringstream prefix_str;
        prefix_str << std::setw(9) << entry.m_clock << std::setw(0)
                   << "  " << div4 << "'" << div3 << "'" << div2 << "'" << div1
                   << "  " << std::left << std::setw(10) << category_str(entry.m_category) << std::setw(0) << std::right
                   << "  ";

        auto prefix = prefix_str.str();

        std::stringstream result;

        auto start = 0U;
        auto end   = entry.m_message.find('\n');

        while (end != std::string::npos)
        {
            auto line = entry.m_message.substr(start, end - start);
            result << prefix << line << '\n';

            start = end + 1;
            end   = entry.m_message.find('\n', start);
        }

        result << prefix << entry.m_message.substr(start, end) << '\n';

        return result.str();
    }

} // namespace



std::string age::tester::get_device_type_string(age::gb_device_type device_type)
{
    switch (device_type)
    {
        case age::gb_device_type::auto_detect:
            return "(auto-detect)";

        case age::gb_device_type::dmg:
            return "(dmg)";

        case age::gb_device_type::cgb_abcd:
            return "(cgb-a/b/c/d)";

        case age::gb_device_type::cgb_e:
            return "(cgb-e)";
    }
    return "(unknown)";
}



void age::tester::write_log([[maybe_unused]] const std::filesystem::path& log_path,
                            std::vector<gb_log_entry>                     log_entries,
                            const std::filesystem::path&                  rom_path,
                            gb_device_type                                device_type)
{
    if (log_entries.empty())
    {
        return;
    }

    std::stringstream log;
    log << log_header(rom_path, device_type);

    std::for_each(begin(log_entries),
                  end(log_entries),
                  [&](const auto& entry) {
                      log << format_log_entry(entry);
                  });

#ifdef AGE_COMPILE_LOGGER
    std::ofstream file(log_path);
    file << log.str();
#endif
}
