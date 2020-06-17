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

#include "common/age_gb_device.hpp"
#include "common/age_gb_clock.hpp"



namespace age
{

enum class gb_interrupt : uint8_t
{
    joypad = 0x10,
    serial = 0x08,
    timer = 0x04,
    lcd = 0x02,
    vblank = 0x01
};

enum class gb_state
{
    halted = 0,
    cpu_active = 1,
    dma = 2
};

enum class gb_event : uint8_t
{
    switch_double_speed = 0,
    timer_overflow = 1,
    lcd_lyc_check = 2,
    lcd_late_lyc_interrupt = 3,
    start_hdma = 4,
    start_oam_dma = 5,
    serial_transfer_finished = 6,

    none = 7 // must always be the last value
};



class gb_core
{
    AGE_DISABLE_COPY(gb_core);

public:

    gb_core(const gb_device &device, gb_clock &clock);

    gb_state get_state() const;

    void insert_event(int clock_cycle_offset, gb_event event);
    void remove_event(gb_event event);
    gb_event poll_event();
    int get_event_cycle(gb_event event) const;
    void set_back_clock(int clock_cycle_offset);

    void start_dma();
    void finish_dma();

    void request_interrupt(gb_interrupt interrupt);
    void ei_delayed();
    void ei_now();
    void di();
    bool halt();
    void stop();
    bool must_service_interrupt();
    uint8_t get_interrupt_to_service();

    uint8_t read_key1() const;
    uint8_t read_ie() const;
    uint8_t read_if() const;

    void write_key1(uint8_t value);
    void write_ie(uint8_t value);
    void write_if(uint8_t value);



private:

    class gb_events
    {
    public:

        gb_events();

        int get_event_cycle(gb_event event) const;

        gb_event poll_event(int current_cycle);
        void insert_event(int for_cycle, gb_event event);

        void set_back_clock(int offset);

    private:

        // the following two members should be replaced by some better suited data structure(s)
        std::multimap<int, gb_event> m_events;
        std::array<int, to_integral(gb_event::none)> m_event_cycle;
    };



    void check_halt_mode();

    const gb_device &m_device;
    gb_clock &m_clock;

    gb_events m_events;
    gb_state m_state = gb_state::cpu_active;

    bool m_halt = false;
    bool m_ei = false;
    bool m_ime = false;

    uint8_t m_key1 = 0x7E;
    uint8_t m_if = 0xE0;
    uint8_t m_ie = 0;
};

} // namespace age



#endif // AGE_GB_CORE_HPP
