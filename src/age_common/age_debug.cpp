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

#include <age_debug.hpp>



#ifdef AGE_DEBUG

#include <array>
#include <chrono>
#include <iostream> // std::cout

std::mutex age::concurrent_cout::M_mutex;



void age::concurrent_cout::log_line()
{
    std::lock_guard<std::mutex> guard(M_mutex);

    std::string str = m_ss.str();
    std::cout << str << std::endl;
    // flush immediately to make sure we immediately see all logging in case the program crashes
    std::cout.flush();
}



age::age_log_time::age_log_time()
    : std::string(get_timestamp())
{
}

std::string age::age_log_time::get_timestamp()
{
    std::chrono::system_clock::time_point tp        = std::chrono::system_clock::now();
    std::chrono::system_clock::duration   dtn       = tp.time_since_epoch();
    time_t                                tt        = std::chrono::system_clock::to_time_t(tp);
    tm*                                   time_info = localtime(&tt);

    int64_t milliseconds = dtn.count()
                           * std::chrono::system_clock::period::num * 1000
                           / std::chrono::system_clock::period::den;
    std::array<char, 80> tmp{};
    strftime(tmp.data(), 80, "%H:%M:%S", time_info);

    std::array<char, 84> tmp_millis{};
    sprintf(tmp_millis.data(), "%s.%03d", tmp.data(), static_cast<int>(milliseconds % 1000));

    return {tmp_millis.data()};
}

#endif // AGE_DEBUG
