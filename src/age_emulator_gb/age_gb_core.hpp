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

#ifndef AGE_GB_CORE_HPP
#define AGE_GB_CORE_HPP

//!
//! \file
//!

#include <array>
#include <map>

#include <age_debug.hpp>
#include <age_types.hpp>
#include <emulator/age_gb_types.hpp>

#ifdef AGE_DEBUG
#define AGE_GB_CYCLE_LOG(x) AGE_LOG("cycle " << m_core.get_oscillation_cycle() << ": " << x)
#else
#define AGE_GB_CYCLE_LOG(x)
#endif



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

enum class gb_state : uint
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
    serial_transfer_finished = 7,

    none = 8 // must always be the last value
};

constexpr uint gb_no_cycle = uint_max;
constexpr uint gb_machine_cycles_per_second = 4194304;



class gb_core
{
public:

    uint get_oscillation_cycle() const;
    uint get_machine_cycles_per_cpu_cycle() const;
    bool is_double_speed() const;
    bool is_cgb() const; //!< get_mode() == gb_mode::cgb
    bool is_cgb_hardware() const; //!< get_mode() != gb_mode::dmg
    gb_mode get_mode() const;
    gb_state get_state() const;

    void oscillate_cpu_cycle();
    void oscillate_2_cycles();
    void insert_event(uint oscillation_cycle_offset, gb_event event);
    void remove_event(gb_event event);
    gb_event poll_event();
    uint get_event_cycle(gb_event event) const;

    void start_dma();
    void finish_dma();

    void request_interrupt(gb_interrupt interrupt);
    void ei_delayed();
    void ei_now();
    void di();
    bool halt();
    void stop();
    bool must_service_interrupt();
    uint8 get_interrupt_to_service();

    uint8 read_key1() const;
    uint8 read_ie() const;
    uint8 read_if() const;

    void write_key1(uint8 value);
    void write_ie(uint8 value);
    void write_if(uint8 value);

    gb_core(gb_mode mode);



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

    const gb_mode m_mode;
    gb_state m_state = gb_state::cpu_active;

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
