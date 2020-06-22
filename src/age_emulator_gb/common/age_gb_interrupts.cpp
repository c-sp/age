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
    m_if |= to_integral(interrupt);

    CLOG_INTERRUPTS("interrupt requested: "
                    << AGE_LOG_HEX8(to_integral(interrupt)));

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

        // Leaving HALT mode adds some delay on CGB hardware
        // if HALT mode was started with IME=1.
        //! \todo we need some test roms for this
        //! \todo move this to gb_cpu?
        //
        // Indirectly tested:
        //
        // passing on DMG and CGB hardware:
        // acceptance\halt_ime0_nointr_timing
        //  =>  only HALTing with:
        //      <interrupts disabled>
        //      <optional ei>
        //      halt
        //      nop
        //
        // failing only on CGB hardware:
        // acceptance\halt_ime1_timing2-GS.s
        //  =>  HALTing with (additionally):
        //      <interrupts enabled>
        //      halt
        //      nop
        //
        if (m_ime_on_halt && m_device.is_cgb_hardware())
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
    CLOG_INTERRUPTS("interrupts " << (ime ? "enabled" : "disabled"));
}



int age::gb_interrupt_dispatcher::next_interrupt_bit() const
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

void age::gb_interrupt_dispatcher::interrupt_dispatched(int interrupt_bit)
{
    m_if &= ~interrupt_bit;
    m_ime = false;
    CLOG_INTERRUPTS("clearing IF interrupt bit " << AGE_LOG_HEX8(interrupt_bit)
                    << ", interrupts disabled");
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
    CLOG_INTERRUPTS("halted");

    // a pending interrupt will terminate HALT immediately
    check_halt_mode();
}
