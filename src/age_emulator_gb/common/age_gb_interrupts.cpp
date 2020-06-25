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

#include "age_gb_interrupts.hpp"

#define CLOG_INTERRUPTS(log) AGE_GB_CLOG(AGE_GB_CLOG_INTERRUPTS)(log)

namespace
{

constexpr age::uint8_t serial_bit = to_integral(age::gb_interrupt::serial);
constexpr age::uint8_t timer_bit = to_integral(age::gb_interrupt::timer);

constexpr uint8_t deny_retrigger = serial_bit | timer_bit;

}




//---------------------------------------------------------
//
//   interrupt trigger
//
//---------------------------------------------------------

age::gb_interrupt_trigger::gb_interrupt_trigger(const gb_device &device,
                                                gb_clock &clock)
    : m_device(device),
      m_clock(clock)
{
}



void age::gb_interrupt_trigger::trigger_interrupt(gb_interrupt interrupt)
{
    uint8_t intr_bit = to_integral(interrupt);

    // During interrupt dispatch after the respectve IF flag has
    // been cleared the CGB apparently denies that interrupt from being
    // requested.
    //
    // Gambatte tests:
    //      serial/start_wait_trigger_int8_read_if_1_dmg08_cgb04c_outE8
    //      serial/start_wait_trigger_int8_read_if_2_dmg08_outE8_cgb04c_outE0
    //      serial/start_wait_trigger_int8_read_if_ds_2_cgb04c_outE0
    //      tima/tc00_irq_late_retrigger_1_dmg08_cgb04c_outE4
    //      tima/tc00_irq_late_retrigger_2_dmg08_outE4_cgb04c_outE0
    //      tima/tc00_irq_late_retrigger_ds_2_cgb04c_outE0
    if ((intr_bit & m_during_dispatch & deny_retrigger) && m_device.is_cgb_hardware())
    {
        CLOG_INTERRUPTS("denying interrupt request: " << AGE_LOG_HEX8(intr_bit)
                        << " (currently being dispatched)");
        return;
    }

    CLOG_INTERRUPTS("interrupt requested: " << AGE_LOG_HEX8(intr_bit));
    m_if |= intr_bit;

    // might clear HALT mode
    check_halt_mode();
}



void age::gb_interrupt_trigger::check_halt_mode()
{
    // terminate HALT once ie & if != 0
    // (even if ime is cleared)
    if (m_halted && (m_if & m_ie & 0x1F))
    {
        m_halted = false;
        CLOG_INTERRUPTS("HALT terminated by pending interrupt");

        // We know from Gambatte timer tests that leaving HALT mode
        // takes one additional machine cycle (DMG and CGB) in the
        // following case:
        //      ei
        //      nop
        //      halt
        //
        // However, this breaks the Mooneye GB test:
        //      acceptance/halt_ime1_timing2-GS
        //
        //! \todo examine this further
        //
        if (m_ime_on_halt)
        {
            m_clock.tick_machine_cycle();
            CLOG_INTERRUPTS("HALT termination delay");
        }
    }
}





//---------------------------------------------------------
//
//   interrupt i/o ports
//
//---------------------------------------------------------

age::uint8_t age::gb_interrupt_ports::read_if() const
{
    CLOG_INTERRUPTS("read IF " << AGE_LOG_HEX8(m_if));
    return m_if;
}

age::uint8_t age::gb_interrupt_ports::read_ie() const
{
    CLOG_INTERRUPTS("read IE " << AGE_LOG_HEX8(m_if));
    return m_ie;
}



void age::gb_interrupt_ports::write_if(uint8_t value)
{
    CLOG_INTERRUPTS("write IF " << AGE_LOG_HEX8(value));
    m_if = value | 0xE0;
    check_halt_mode();
}

void age::gb_interrupt_ports::write_ie(uint8_t value)
{
    CLOG_INTERRUPTS("write IE " << AGE_LOG_HEX8(value));
    m_ie = value;
    check_halt_mode();
}





//---------------------------------------------------------
//
//   interrupt dispatcher
//
//---------------------------------------------------------

bool age::gb_interrupt_dispatcher::get_ime() const
{
    return m_ime;
}

void age::gb_interrupt_dispatcher::set_ime(bool ime)
{
    m_ime = ime;
    CLOG_INTERRUPTS("interrupt dispatching " << (ime ? "enabled" : "disabled"));
}



age::uint8_t age::gb_interrupt_dispatcher::next_interrupt_bit() const
{
    if (!m_ime)
    {
        return 0;
    }

    // get the lowest interrupt bit that is set
    // (lower interrupt bits have higher priority)
    int interrupts = m_ie & m_if & 0x1F;
    return interrupts & ~(interrupts - 1);
}

void age::gb_interrupt_dispatcher::clear_interrupt_flag(uint8_t interrupt_bit)
{
    m_if &= ~interrupt_bit;
    m_during_dispatch = interrupt_bit;
    CLOG_INTERRUPTS("clear IF flag " << AGE_LOG_HEX8(interrupt_bit));
}

void age::gb_interrupt_dispatcher::finish_dispatch()
{
    CLOG_INTERRUPTS("interrupt dispatching disabled");
    m_ime = false;
    m_during_dispatch = 0;
}



bool age::gb_interrupt_dispatcher::halted() const
{
    return m_halted;
}

void age::gb_interrupt_dispatcher::halt()
{
    AGE_ASSERT(!m_halted);

    m_halted = true;
    m_ime_on_halt = m_ime;
    CLOG_INTERRUPTS("halted (ime = " << m_ime_on_halt << ")");

    // a pending interrupt will terminate HALT immediately
    check_halt_mode();
}
