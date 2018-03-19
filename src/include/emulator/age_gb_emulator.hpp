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

#ifndef AGE_GB_EMULATOR_HPP
#define AGE_GB_EMULATOR_HPP

//!
//! \file
//!

#include <string>

#include <age_types.hpp>
#include <emulator/age_emulator.hpp>



namespace age
{

constexpr uint gb_right = 0x01;
constexpr uint gb_left = 0x02;
constexpr uint gb_up = 0x04;
constexpr uint gb_down = 0x08;
constexpr uint gb_a = 0x10;
constexpr uint gb_b = 0x20;
constexpr uint gb_select = 0x40;
constexpr uint gb_start = 0x80;



//!
//! \brief Struct containing parts of the Gameboy CPU state.
//!
//! This struct exists only for evaluating Gameboy test results.
//!
struct gb_test_info
{
    bool m_is_cgb;
    bool m_found_invalid_opcode;

    uint8 m_a = 0;
    uint8 m_b = 0;
    uint8 m_c = 0;
    uint8 m_d = 0;
    uint8 m_e = 0;
    uint8 m_h = 0;
    uint8 m_l = 0;
};



class gb_emulator_impl; // forward class declaration to decouple the actual implementation

class gb_emulator : public emulator
{
public:

    gb_emulator(const uint8_vector &rom, bool force_dmg = false, bool dmg_green = true);
    virtual ~gb_emulator() override;

    uint8_vector get_persistent_ram() const override;
    void set_persistent_ram(const uint8_vector &source) override;

    void set_buttons_down(uint buttons) override;
    void set_buttons_up(uint buttons) override;

    bool is_cgb() const;
    gb_test_info get_test_info() const;

    static std::string extract_rom_name(const uint8_vector &rom);

protected:

    uint64 inner_emulate(uint64 min_cycles_to_emulate) override;

    std::string inner_get_emulator_title() const override;

private:

    // cannot use std::unique_ptr<gb_emulator_impl> since it does not work on incomplete types
    gb_emulator_impl *m_impl = nullptr;
};

} // namespace age



#endif // AGE_GB_EMULATOR_HPP
