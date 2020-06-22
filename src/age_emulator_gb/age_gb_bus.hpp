//
// Copyright 2019 Christoph Sprenger
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

#ifndef AGE_GB_BUS_HPP
#define AGE_GB_BUS_HPP

//!
//! \file
//!

#include <age_types.hpp>

#include "common/age_gb_clock.hpp"
#include "common/age_gb_device.hpp"
#include "common/age_gb_events.hpp"
#include "common/age_gb_interrupts.hpp"

#include "age_gb_div.hpp"
#include "age_gb_joypad.hpp"
#include "age_gb_lcd.hpp"
#include "age_gb_memory.hpp"
#include "age_gb_serial.hpp"
#include "age_gb_sound.hpp"
#include "age_gb_timer.hpp"



namespace age
{

enum class gb_io_port : uint16_t
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



class gb_bus
{
    AGE_DISABLE_COPY(gb_bus);

public:

    gb_bus(const gb_device &device, gb_clock &clock, gb_interrupt_ports &interrupts, gb_events &events, gb_memory &memory, gb_div &div, gb_sound &sound, gb_lcd &lcd, gb_timer &timer, gb_joypad &joypad, gb_serial &serial);

    uint8_t read_byte(uint16_t address);
    void write_byte(uint16_t address, uint8_t byte);

    void handle_events();
    void handle_dma();
    bool during_dma() const;

    void set_back_clock(int clock_cycle_offset);

private:

    void write_dma(uint8_t value);
    void write_hdma5(uint8_t value);

    void handle_oam_dma();

    const gb_device &m_device;
    gb_clock &m_clock;
    gb_interrupt_ports &m_interrupts;
    gb_events &m_events;
    gb_memory &m_memory;
    gb_div &m_div;
    gb_sound &m_sound;
    gb_lcd &m_lcd;
    gb_timer &m_timer;
    gb_joypad &m_joypad;
    gb_serial &m_serial;

    uint8_array<0x200> m_high_ram;  // 0xFE00 - 0xFFFF (including OAM ram and i/o ports for easier handling)
    uint8_t m_rp = 0x3E;
    uint8_t m_un6c = 0xFE;
    uint8_t m_un72 = 0;
    uint8_t m_un73 = 0;
    uint8_t m_un75 = 0x8F;

    uint8_t m_oam_dma_byte;
    bool m_oam_dma_active = false;
    int m_oam_dma_address = 0;
    int m_oam_dma_offset = 0;
    int m_oam_dma_last_cycle = gb_no_clock_cycle;

    uint8_t m_hdma5 = 0xFF;
    int m_dma_source = 0;
    int m_dma_destination = 0;
    bool m_during_dma = false;
};

} // namespace age



#endif // AGE_GB_BUS_HPP
