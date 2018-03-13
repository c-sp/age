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

#include "age_emulator.hpp"



age::emulator::emulator(uint screen_width, uint screen_height, uint sampling_rate, uint cycles_per_second)
    : m_sampling_rate(sampling_rate),
      m_cycles_per_second(cycles_per_second),
      m_video_buffer_handler(screen_width, screen_height)
{
    AGE_ASSERT(m_sampling_rate > 0);
    AGE_ASSERT(m_cycles_per_second > 0);
}



std::string age::emulator::get_emulator_title() const
{
    constexpr char ascii_white_space = 0x20;
    constexpr char ascii_underscore = 0x5F;
    constexpr char ascii_0 = 0x30;
    constexpr char ascii_9 = 0x39;
    constexpr char ascii_a = 0x61;
    constexpr char ascii_z = 0x7A;
    constexpr char ascii_A = 0x41;
    constexpr char ascii_Z = 0x5A;

    std::string inner_title = inner_get_emulator_title();
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



age::uint age::emulator::get_screen_width() const
{
    return m_video_buffer_handler.get_screen_width();
}

age::uint age::emulator::get_screen_height() const
{
    return m_video_buffer_handler.get_screen_height();
}

const age::pixel_vector& age::emulator::get_video_front_buffer() const
{
    return m_video_buffer_handler.get_front_buffer();
}

const age::pcm_vector& age::emulator::get_audio_buffer() const
{
    return m_audio_buffer;
}

age::uint age::emulator::get_pcm_sampling_rate() const
{
    return m_sampling_rate;
}

age::uint age::emulator::get_cycles_per_second() const
{
    return m_cycles_per_second;
}

age::uint64 age::emulator::get_emulated_cycles() const
{
    return m_emulated_cycles;
}

bool age::emulator::emulate(uint64 min_cycles_to_emulate)
{
    uint current_front_buffer = m_video_buffer_handler.get_front_buffer_index();
    m_audio_buffer.clear();

    uint64 emulated_cycles = inner_emulate(min_cycles_to_emulate);
    AGE_ASSERT(emulated_cycles >= min_cycles_to_emulate);
    m_emulated_cycles += emulated_cycles;

    bool new_frame = m_video_buffer_handler.get_front_buffer_index() != current_front_buffer;
    return new_frame;
}



age::video_buffer_handler& age::emulator::get_video_buffer_handler()
{
    return m_video_buffer_handler;
}

age::pcm_vector& age::emulator::get_pcm_vector()
{
    return m_audio_buffer;
}
