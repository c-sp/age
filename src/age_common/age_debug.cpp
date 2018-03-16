//
// Copyright (c) 2010-2017 Christoph Sprenger
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

#include <chrono>
#include <iostream> // std::cout

#include <age_debug.hpp>



#ifdef AGE_DEBUG

std::mutex age::concurrent_cout::M_mutex;



void age::concurrent_cout::log_line()
{
    std::lock_guard<std::mutex> guard(M_mutex);

    std::string str = m_ss.str();
    std::cout << str << std::endl;
    // fush immediately to make sure we immediately see all logging in case the program crashes
    std::cout.flush();
}



age::age_log_time::age_log_time()
    : std::string(get_timestamp())
{
}

std::string age::age_log_time::get_timestamp()
{
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    time_t tt = std::chrono::system_clock::to_time_t(tp);
    tm *time_info = localtime(&tt);

    std::chrono::system_clock::duration dtn = tp.time_since_epoch();
    size_t milliseconds = dtn.count() * std::chrono::system_clock::period::num * 1000
                                      / std::chrono::system_clock::period::den;

    char tmp [80];
    strftime(tmp, 80, "%H:%M:%S", time_info);

    char tmp_millis[84] = "";
    sprintf(tmp_millis, "%s.%03d", tmp, static_cast<int>(milliseconds % 1000));

    return std::string(tmp_millis);
}

#endif // AGE_DEBUG
