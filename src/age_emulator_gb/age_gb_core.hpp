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

#ifndef AGE_GB_CORE_HPP
#define AGE_GB_CORE_HPP

//!
//! \file
//!

#include "age_gb.hpp"



namespace age
{

enum class gb_interrupt : uint8
{
    joypad = 0x10,
    serial = 0x08,
    timer = 0x04,
    lcd = 0x02,
    vblank = 0x01
};

enum class gb_mode : uint
{
    halted = 0,
    cpu_active = 1,
    dma = 2
};

enum class gb_event : uint
{
    switch_double_speed = 0,
    timer_overflow = 1,
    sound_frame_sequencer = 2,
    lcd_lyc_check = 3,
    lcd_late_lyc_interrupt = 4,
    start_hdma = 5,
    start_oam_dma = 6,

    none = 7 // must always be the last value
};



class gb_core : public non_copyable
{
public:

    uint get_oscillation_cycle() const;
    uint get_machine_cycles_per_cpu_cycle() const;
    bool is_double_speed() const;
    bool is_cgb() const;
    gb_mode get_mode() const;

    void oscillate_cpu_cycle();
    void oscillate_2_cycles();
    void insert_event(uint oscillation_cycle_offset, gb_event event);
    void remove_event(gb_event event);
    gb_event poll_event();
    uint get_event_cycle(gb_event event) const;

    void start_dma();
    void finish_dma();

    void request_interrupt(gb_interrupt interrupt);
    void ei();
    void di();
    bool halt();
    void stop();
    uint16 get_interrupt_to_service();

    uint8 read_key1() const;
    uint8 read_ie() const;
    uint8 read_if() const;

    void write_key1(uint8 value);
    void write_ie(uint8 value);
    void write_if(uint8 value);

    gb_core(bool cgb);



private:

    class gb_events
    {
    public:

        gb_events();

        uint get_event_cycle(gb_event event) const;

        gb_event poll_event(uint current_cycle);
        void insert_event(uint for_cycle, gb_event event);

    private:

        // the following two members should be replaced by some better suited data structure(s)
        std::multimap<uint, gb_event> m_events;
        std::array<uint, to_integral(gb_event::none)> m_event_cycle;
    };



    void check_halt_mode();

    const bool m_cgb;
    gb_mode m_mode = gb_mode::cpu_active;

    uint m_oscillation_cycle = 0;
    uint m_machine_cycles_per_cpu_cycle = 4;

    bool m_halt = false;
    bool m_ei = false;
    bool m_ime = false;
    bool m_double_speed = false;

    uint8 m_key1 = 0x7E;
    uint8 m_if = 0xE0;
    uint8 m_ie = 0;

    gb_events m_events;
};

} // namespace age



#endif // AGE_GB_CORE_HPP
