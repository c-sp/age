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
age::uint8_vector gb_rom;
age::uint8_vector gb_persistent_ram;
bool gb_persistent_ram_dirty = false; // true => emulator_exists() == true

void free_memory(age::uint8_vector &vec)
{
    LOG("freeing memory buffer of " << vec.size() << " bytes at " << (void*)vec.data());
    age::uint8_vector().swap(vec);
}

bool emulator_exists()
{
    return gb_emu != nullptr;
}

void delete_current_emulator()
{
    if (emulator_exists())
    {
        LOG("deleting gb_emulator instance");
        delete gb_emu;
        gb_emu = nullptr;

        free_memory(gb_rom);
        free_memory(gb_persistent_ram);

        gb_persistent_ram_dirty = false;
    }
}



extern "C" { // prevent function name mangling



EMSCRIPTEN_KEEPALIVE
age::uint8* gb_allocate_rom_buffer(age::uint rom_size)
{
    free_memory(gb_rom);
    gb_rom.resize(rom_size);
    return gb_rom.data();
}

EMSCRIPTEN_KEEPALIVE
void gb_new_emulator()
{
    delete_current_emulator();

    LOG("creating emulator from rom vector of " << gb_rom.size() << " bytes at " << (void*)gb_rom.data());
    gb_emu = new age::gb_emulator(gb_rom);
    gb_persistent_ram_dirty = true;

    free_memory(gb_rom);
}



EMSCRIPTEN_KEEPALIVE
age::uint8* gb_get_persistent_ram()
{
    if (gb_persistent_ram_dirty)
    {
        gb_persistent_ram = gb_emu->get_persistent_ram();
    }
    return gb_persistent_ram.data();
}

EMSCRIPTEN_KEEPALIVE
age::uint gb_get_persistent_ram_size()
{
    if (gb_persistent_ram_dirty)
    {
        gb_persistent_ram = gb_emu->get_persistent_ram();
    }
    return gb_persistent_ram.size();
}

EMSCRIPTEN_KEEPALIVE
void gb_set_persistent_ram()
{
    if (emulator_exists())
    {
        gb_emu->set_persistent_ram(gb_persistent_ram);
    }
}



EMSCRIPTEN_KEEPALIVE
bool gb_emulate(age::uint64 min_cycles_to_emulate)
{
    bool result = false;

    if (emulator_exists())
    {
        gb_persistent_ram_dirty = true;
        result = gb_emu->emulate(min_cycles_to_emulate);
    }

    return result;
}

EMSCRIPTEN_KEEPALIVE
age::uint gb_get_cycles_per_second()
{
    return emulator_exists() ? gb_emu->get_cycles_per_second() : 0;
}

EMSCRIPTEN_KEEPALIVE
age::uint64 gb_get_emulated_cycles()
{
    return emulator_exists() ? gb_emu->get_emulated_cycles() : 0;
}



EMSCRIPTEN_KEEPALIVE
void gb_set_buttons_down(age::uint buttons)
{
    if (emulator_exists())
    {
        gb_emu->set_buttons_down(buttons);
    }
}

EMSCRIPTEN_KEEPALIVE
void gb_set_buttons_up(age::uint buttons)
{
    if (emulator_exists())
    {
        gb_emu->set_buttons_up(buttons);
    }
}



EMSCRIPTEN_KEEPALIVE
age::uint gb_get_screen_width()
{
    return emulator_exists() ? gb_emu->get_screen_width() : 0;
}

EMSCRIPTEN_KEEPALIVE
age::uint gb_get_screen_height()
{
    return emulator_exists() ? gb_emu->get_screen_height() : 0;
}

EMSCRIPTEN_KEEPALIVE
const age::pixel* gb_get_screen_front_buffer()
{
    return emulator_exists() ? gb_emu->get_screen_front_buffer().data() : nullptr;
}



EMSCRIPTEN_KEEPALIVE
const age::pcm_sample* gb_get_audio_buffer()
{
    return emulator_exists() ? gb_emu->get_audio_buffer().data() : nullptr;
}

EMSCRIPTEN_KEEPALIVE
age::uint gb_get_pcm_sampling_rate()
{
    return emulator_exists() ? gb_emu->get_pcm_sampling_rate() : 0;
}

} // extern "C"
