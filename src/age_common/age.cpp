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

#include "age.hpp"



//---------------------------------------------------------
//
//   debug build macros
//
//---------------------------------------------------------

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



age::dbg_log_time::dbg_log_time()
    : std::string(get_timestamp())
{
}

std::string age::dbg_log_time::get_timestamp()
{
    using namespace std::chrono;

    system_clock::time_point tp = system_clock::now();
    time_t tt = system_clock::to_time_t(tp);
    tm *time_info = localtime(&tt);

    system_clock::duration dtn = tp.time_since_epoch();
    size_t milliseconds = dtn.count() * system_clock::period::num * 1000 / system_clock::period::den;

    char tmp [80];
    strftime(tmp, 80, "%H:%M:%S", time_info);

    char tmp_millis[84] = "";
    sprintf(tmp_millis, "%s.%03d", tmp, static_cast<int>(milliseconds % 1000));

    return std::string(tmp_millis);
}

#endif // AGE_DEBUG



//---------------------------------------------------------
//
//   video buffer handler
//
//---------------------------------------------------------

age::video_buffer_handler::video_buffer_handler(uint screen_width, uint screen_height)
    : m_screen_width(screen_width),
      m_screen_height(screen_height)
{
    AGE_ASSERT(m_screen_width <= uint_max / m_screen_height);
    AGE_ASSERT(m_screen_height <= uint_max / m_screen_width);
    AGE_ASSERT(m_screen_width > 0);
    AGE_ASSERT(m_screen_height > 0);

    m_buffers[0] = pixel_vector(m_screen_width * m_screen_height);
    m_buffers[1] = pixel_vector(m_screen_width * m_screen_height);
}



age::uint age::video_buffer_handler::get_front_buffer_index() const
{
    return m_current_front_buffer;
}

age::uint age::video_buffer_handler::get_screen_width() const
{
    return m_screen_width;
}

age::uint age::video_buffer_handler::get_screen_height() const
{
    return m_screen_height;
}

const age::pixel_vector& age::video_buffer_handler::get_front_buffer() const
{
    return m_buffers[m_current_front_buffer];
}

const age::pixel_vector& age::video_buffer_handler::get_back_buffer() const
{
    return m_buffers[1 - m_current_front_buffer];
}



age::pixel_vector& age::video_buffer_handler::get_back_buffer()
{
    return m_buffers[1 - m_current_front_buffer];
}

age::pixel* age::video_buffer_handler::get_first_scanline_pixel(uint scanline)
{
    AGE_ASSERT(scanline < m_screen_height);
    // frames are currently stored upside-down
    pixel *result = get_back_buffer().data() + (m_screen_height - 1 - scanline) * m_screen_width;
    return result;
}

void age::video_buffer_handler::switch_buffers()
{
    m_current_front_buffer = 1 - m_current_front_buffer;
}



//---------------------------------------------------------
//
//   simulator base class
//
//---------------------------------------------------------

age::simulator::simulator(uint screen_width, uint screen_height, uint sampling_rate, uint ticks_per_second)
    : m_sampling_rate(sampling_rate),
      m_ticks_per_second(ticks_per_second),
      m_video_buffer_handler(screen_width, screen_height)
{
    AGE_ASSERT(m_sampling_rate > 0);
    AGE_ASSERT(m_ticks_per_second > 0);
}



std::string age::simulator::get_simulator_title() const
{
    constexpr char ascii_white_space = 0x20;
    constexpr char ascii_underscore = 0x5F;
    constexpr char ascii_0 = 0x30;
    constexpr char ascii_9 = 0x39;
    constexpr char ascii_a = 0x61;
    constexpr char ascii_z = 0x7A;
    constexpr char ascii_A = 0x41;
    constexpr char ascii_Z = 0x5A;

    std::string inner_title = inner_get_simulator_title();
    std::string result;
    uint chars = 0;

    for (char c : inner_title)
    {
        // translate white spaces to underscores
        if (c == ascii_white_space)
        {
            c = ascii_underscore;
        }

        // stop on the first invalid character
        if ((c != ascii_underscore)
                && !((c >= ascii_0) && (c <= ascii_9))
                && !((c >= ascii_a) && (c <= ascii_z))
                && !((c >= ascii_A) && (c <= ascii_Z))
                )
        {
            break;
        }

        // add character to result
        result.append(1, c);
        ++chars;

        // stop if we hit the length limit
        if (chars >= 32)
        {
            break;
        }
    }

    return result;
}



age::uint age::simulator::get_screen_width() const
{
    return m_video_buffer_handler.get_screen_width();
}

age::uint age::simulator::get_screen_height() const
{
    return m_video_buffer_handler.get_screen_height();
}

const age::pixel_vector& age::simulator::get_video_front_buffer() const
{
    return m_video_buffer_handler.get_front_buffer();
}

const age::pcm_vector& age::simulator::get_audio_buffer() const
{
    return m_audio_buffer;
}

age::uint age::simulator::get_pcm_sampling_rate() const
{
    return m_sampling_rate;
}

age::uint age::simulator::get_ticks_per_second() const
{
    return m_ticks_per_second;
}

age::uint64 age::simulator::get_simulated_ticks() const
{
    return m_simulated_ticks;
}

bool age::simulator::simulate(uint64 min_ticks_to_simulate)
{
    uint current_front_buffer = m_video_buffer_handler.get_front_buffer_index();
    m_audio_buffer.clear();

    uint64 simulated_ticks = inner_simulate(min_ticks_to_simulate);
    AGE_ASSERT(simulated_ticks >= min_ticks_to_simulate);
    m_simulated_ticks += simulated_ticks;

    bool new_frame = m_video_buffer_handler.get_front_buffer_index() != current_front_buffer;
    return new_frame;
}



age::video_buffer_handler& age::simulator::get_video_buffer_handler()
{
    return m_video_buffer_handler;
}

age::pcm_vector& age::simulator::get_pcm_vector()
{
    return m_audio_buffer;
}
