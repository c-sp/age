//
// Copyright (c) 2010-2018 Christoph Sprenger
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


#include <cstring> // memcpy

#include <emulator/age_gb_emulator.hpp>

#ifdef EMSCRIPTEN
// If emscripten is available include it's header.
#include <emscripten.h>

#else
// If emscripten is not available define an empty
// EMSCRIPTEN_KEEPALIVE macro to allow for successful
// compilation.
#define EMSCRIPTEN_KEEPALIVE
#endif

#if 1
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif


age::gb_emulator *gb_emu = nullptr;
age::uint8_vector p_ram;



extern "C" { // prevent function name mangling

EMSCRIPTEN_KEEPALIVE
void gb_new_emulator(age::uint8 *rom_ptr, age::uint rom_size)
{
    if (gb_emu != nullptr)
    {
        LOG("deleting old gb_emulator instance");
        delete gb_emu;
        gb_emu = nullptr;
    }

    LOG("creating rom vector from " << rom_size << " bytes of rom at " << (void*)rom_ptr);
    age::uint8_vector rom;
    rom.resize(rom_size);
    memcpy(rom.data(), rom_ptr, rom_size);

    LOG("creating new gb_emulator instance");
    gb_emu = new age::gb_emulator(rom);
}

EMSCRIPTEN_KEEPALIVE
age::uint gb_get_screen_width()
{
    return gb_emu->get_screen_width();
}

EMSCRIPTEN_KEEPALIVE
age::uint gb_get_screen_height()
{
    return gb_emu->get_screen_height();
}

EMSCRIPTEN_KEEPALIVE
const age::pixel* gb_get_screen_front_buffer()
{
    const age::pixel_vector &vec = gb_emu->get_screen_front_buffer();
    return vec.data();
}

EMSCRIPTEN_KEEPALIVE
const age::pcm_sample* gb_get_audio_buffer()
{
    const age::pcm_vector &vec = gb_emu->get_audio_buffer();
    return vec.data();
}

EMSCRIPTEN_KEEPALIVE
age::uint gb_get_pcm_sampling_rate()
{
    return gb_emu->get_pcm_sampling_rate();
}

EMSCRIPTEN_KEEPALIVE
age::uint gb_get_cycles_per_second()
{
    return gb_emu->get_cycles_per_second();
}

EMSCRIPTEN_KEEPALIVE
age::uint64 gb_get_emulated_cycles()
{
    return gb_emu->get_emulated_cycles();
}

EMSCRIPTEN_KEEPALIVE
const age::uint8* gb_get_persistent_ram()
{
    p_ram = gb_emu->get_persistent_ram();
    return p_ram.data();
}

EMSCRIPTEN_KEEPALIVE
void gb_set_persistent_ram(const age::uint8_vector &source)
{
    gb_emu->set_persistent_ram(source);
}

EMSCRIPTEN_KEEPALIVE
void gb_set_buttons_down(age::uint buttons)
{
    gb_emu->set_buttons_down(buttons);
}

EMSCRIPTEN_KEEPALIVE
void gb_set_buttons_up(age::uint buttons)
{
    gb_emu->set_buttons_up(buttons);
}

EMSCRIPTEN_KEEPALIVE
bool gb_emulate(age::uint64 min_cycles_to_emulate)
{
    return gb_emu->emulate(min_cycles_to_emulate);
}

} // extern "C"
