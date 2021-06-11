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

#include <emulator/age_gb_emulator.hpp>

#include "age_gb_emulator_impl.hpp"



age::gb_emulator::gb_emulator(const uint8_vector& rom,
                              gb_hardware hardware,
                              gb_colors_hint colors_hint,
                              gb_log_categories log_categories)

    : m_impl(new gb_emulator_impl(rom, hardware, colors_hint, log_categories))
{
}

age::gb_emulator::~gb_emulator()
{
    delete m_impl;
}



std::string age::gb_emulator::get_emulator_title() const
{
    return m_impl->get_emulator_title();
}

age::int16_t age::gb_emulator::get_screen_width() const
{
    return m_impl->get_screen_width();
}

age::int16_t age::gb_emulator::get_screen_height() const
{
    return m_impl->get_screen_height();
}

const age::pixel_vector& age::gb_emulator::get_screen_front_buffer() const
{
    return m_impl->get_screen_front_buffer();
}

const age::pcm_vector& age::gb_emulator::get_audio_buffer() const
{
    return m_impl->get_audio_buffer();
}

int age::gb_emulator::get_pcm_sampling_rate() const
{
    return m_impl->get_pcm_sampling_rate();
}

int age::gb_emulator::get_cycles_per_second() const
{
    return m_impl->get_cycles_per_second();
}

age::int64_t age::gb_emulator::get_emulated_cycles() const
{
    return m_impl->get_emulated_cycles();
}

age::uint8_vector age::gb_emulator::get_persistent_ram() const
{
    return m_impl->get_persistent_ram();
}

void age::gb_emulator::set_persistent_ram(const uint8_vector& source)
{
    m_impl->set_persistent_ram(source);
}

void age::gb_emulator::set_buttons_down(int buttons)
{
    m_impl->set_buttons_down(buttons);
}

void age::gb_emulator::set_buttons_up(int buttons)
{
    m_impl->set_buttons_up(buttons);
}

bool age::gb_emulator::emulate(int cycles_to_emulate)
{
    return m_impl->emulate(cycles_to_emulate);
}

age::gb_test_info age::gb_emulator::get_test_info() const
{
    return m_impl->get_test_info();
}

const std::vector<age::gb_log_entry>& age::gb_emulator::get_log_entries() const
{
    return m_impl->get_log_entries();
}
