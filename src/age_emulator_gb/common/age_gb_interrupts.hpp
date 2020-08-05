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

#ifndef AGE_GB_INTERRUPTS_HPP
#define AGE_GB_INTERRUPTS_HPP

//!
//! \file
//!

#include <age_types.hpp>

#include "age_gb_device.hpp"
#include "age_gb_clock.hpp"



namespace age
{

enum class gb_interrupt : uint8_t
{
    vblank = 0x01, // highest priority
    lcd = 0x02,
    timer = 0x04,
    serial = 0x08,
    joypad = 0x10, // lowest priority
};



class gb_interrupt_trigger
{
    AGE_DISABLE_COPY(gb_interrupt_trigger);
public:

    gb_interrupt_trigger(const gb_device &device, gb_clock &clock);

    void trigger_interrupt(gb_interrupt interrupt, int irq_clock_cycle);

protected:

    const gb_device &m_device;
    gb_clock &m_clock;
    uint8_t m_if = 0xE1;
    uint8_t m_ie = 0;
    uint8_t m_during_dispatch = 0;
    bool m_ime = false;
    bool m_halted = false;
};



class gb_interrupt_ports : public gb_interrupt_trigger
{
public:

    using gb_interrupt_trigger::gb_interrupt_trigger;

    uint8_t read_ie() const;
    uint8_t read_if() const;

    void write_ie(uint8_t value);
    void write_if(uint8_t value);
};



class gb_interrupt_dispatcher : public gb_interrupt_ports
{
public:

    using gb_interrupt_ports::gb_interrupt_ports;

    bool get_ime() const;
    void set_ime(bool ime);

    uint8_t next_interrupt_bit() const;
    void clear_interrupt_flag(uint8_t interrupt_bit);
    void finish_dispatch();

    bool halted() const;
    void halt();
};

} // namespace age



#endif // AGE_GB_INTERRUPTS_HPP
