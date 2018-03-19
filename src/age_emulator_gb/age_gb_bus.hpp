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

#ifndef AGE_GB_BUS_HPP
#define AGE_GB_BUS_HPP

//!
//! \file
//!

#include <vector>

#include <age_non_copyable.hpp>
#include <age_types.hpp>

#include "age_gb_memory.hpp"
#include "age_gb_sound.hpp"
#include "age_gb_lcd.hpp"
#include "age_gb_timer.hpp"
#include "age_gb_joypad.hpp"
#include "age_gb_serial.hpp"



namespace age
{

enum class gb_io_port : uint16
{
    p1 = 0xFF00,

    sb = 0xFF01,
    sc = 0xFF02,

    div = 0xFF04,
    tima = 0xFF05,
    tma = 0xFF06,
    tac = 0xFF07,

    if_ = 0xFF0F,

    nr10 = 0xFF10,
    nr11 = 0xFF11,
    nr12 = 0xFF12,
    nr13 = 0xFF13,
    nr14 = 0xFF14,

    nr20 = 0xFF15,
    nr21 = 0xFF16,
    nr22 = 0xFF17,
    nr23 = 0xFF18,
    nr24 = 0xFF19,

    nr30 = 0xFF1A,
    nr31 = 0xFF1B,
    nr32 = 0xFF1C,
    nr33 = 0xFF1D,
    nr34 = 0xFF1E,

    nr40 = 0xFF1F,
    nr41 = 0xFF20,
    nr42 = 0xFF21,
    nr43 = 0xFF22,
    nr44 = 0xFF23,

    nr50 = 0xFF24,
    nr51 = 0xFF25,
    nr52 = 0xFF26,

    lcdc = 0xFF40,
    stat = 0xFF41,
    scy = 0xFF42,
    scx = 0xFF43,
    ly = 0xFF44,
    lyc = 0xFF45,
    dma = 0xFF46,
    bgp = 0xFF47,
    obp0 = 0xFF48,
    obp1 = 0xFF49,
    wy = 0xFF4A,
    wx = 0xFF4B,

    key1 = 0xFF4D,

    vbk = 0xFF4F,

    hdma1 = 0xFF51,
    hdma2 = 0xFF52,
    hdma3 = 0xFF53,
    hdma4 = 0xFF54,
    hdma5 = 0xFF55,
    rp = 0xFF56,

    bcps = 0xFF68,
    bcpd = 0xFF69,
    ocps = 0xFF6A,
    ocpd = 0xFF6B,
    un6c = 0xFF6C,

    svbk = 0xFF70,

    un72 = 0xFF72,
    un73 = 0xFF73,
    un74 = 0xFF74,
    un75 = 0xFF75,
    un76 = 0xFF76,
    un77 = 0xFF77,

    ie = 0xFFFF
};



class gb_bus : public non_copyable
{

public:

    gb_bus(gb_core &core, gb_memory &memory, gb_sound &sound, gb_lcd &lcd, gb_timer &timer, gb_joypad &joypad, gb_serial &serial);

    uint8 read_byte(uint16 address);
    void write_byte(uint16 address, uint8 byte);

    void handle_events();
    void handle_dma();

private:

    void write_dma(uint8 value);
    void write_hdma5(uint8 value);

    void handle_oam_dma();

    gb_core &m_core;
    gb_memory &m_memory;
    gb_sound &m_sound;
    gb_lcd &m_lcd;
    gb_timer &m_timer;
    gb_joypad &m_joypad;
    gb_serial &m_serial;

    uint8_array<0x200> m_high_ram;  // 0xFE00 - 0xFFFF (including OAM ram and i/o ports for easier handling)
    uint8 m_rp = 0x3E;
    uint8 m_un6c = 0xFE;
    uint8 m_un72 = 0;
    uint8 m_un73 = 0;
    uint8 m_un74 = 0;
    uint8 m_un75 = 0x8F;
    uint8 m_un76 = 0;
    uint8 m_un77 = 0;

    std::vector<gb_event> m_events;

    uint8 m_oam_dma_byte = 0xFF;
    bool m_oam_dma_active = false;
    uint m_oam_dma_address = 0;
    uint m_oam_dma_offset = 0;
    uint m_oam_dma_last_cycle = 0;

    uint8 m_hdma5 = 0xFF;
    uint m_dma_source = 0;
    uint m_dma_destination = 0;
};

} // namespace age



#endif // AGE_GB_BUS_HPP
