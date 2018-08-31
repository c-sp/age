//
// Copyright 2018 Christoph Sprenger
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

#include <emulator/age_emulator.hpp>



age::emulator::emulator(int16_t screen_width, int16_t screen_height, int sampling_rate, int cycles_per_second)
    : m_sampling_rate(sampling_rate),
      m_cycles_per_second(cycles_per_second),
      m_screen_buffer(screen_width, screen_height)
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

        // stop if we hit the length limit
        if (result.length() >= 32)
        {
            break;
        }
    }

    return result;
}



age::int16_t age::emulator::get_screen_width() const
{
    return m_screen_buffer.get_screen_width();
}

age::int16_t age::emulator::get_screen_height() const
{
    return m_screen_buffer.get_screen_height();
}

const age::pixel_vector& age::emulator::get_screen_front_buffer() const
{
    return m_screen_buffer.get_front_buffer();
}

const age::pcm_vector& age::emulator::get_audio_buffer() const
{
    return m_audio_buffer;
}

int age::emulator::get_pcm_sampling_rate() const
{
    return m_sampling_rate;
}

int age::emulator::get_cycles_per_second() const
{
    return m_cycles_per_second;
}

age::uint64 age::emulator::get_emulated_cycles() const
{
    return m_emulated_cycles;
}

bool age::emulator::emulate(uint64 min_cycles_to_emulate)
{
    auto current_front_buffer = m_screen_buffer.get_front_buffer_index();
    m_audio_buffer.clear();

    uint64 emulated_cycles = inner_emulate(min_cycles_to_emulate);
    AGE_ASSERT(emulated_cycles >= min_cycles_to_emulate);
    m_emulated_cycles += emulated_cycles;

    bool new_frame = m_screen_buffer.get_front_buffer_index() != current_front_buffer;
    return new_frame;
}



age::screen_buffer& age::emulator::get_screen_buffer()
{
    return m_screen_buffer;
}

age::pcm_vector& age::emulator::get_pcm_vector()
{
    return m_audio_buffer;
}
