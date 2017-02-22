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

#ifndef AGE_GB_EMULATOR_HPP
#define AGE_GB_EMULATOR_HPP

//!
//! \file
//!

#include "age_gb_cpu.hpp"



namespace age
{

class gb_emulator : public emulator
{
public:

    static std::string extract_rom_name(const uint8_vector &rom);

    gb_emulator(const uint8_vector &rom, bool force_dmg);

    bool is_cgb() const;
    gb_test_info get_test_info() const;

    uint8_vector get_persistent_ram() const override;
    void set_persistent_ram(const uint8_vector &source) override;

    void set_buttons_down(uint buttons) override;
    void set_buttons_up(uint buttons) override;

protected:

    uint64 inner_emulate(uint64 min_ticks_to_emulate) override;

    std::string inner_get_emulator_title() const override;

private:

    gb_memory m_memory;
    gb_core m_core;
    gb_sound m_sound;
    gb_lcd m_lcd;
    gb_timer m_timer;
    gb_joypad m_joypad;
    gb_serial m_serial;
    gb_bus m_bus;
    gb_cpu m_cpu;
};

} // namespace age



#endif // AGE_GB_EMULATOR_HPP
