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



void age::gb_interrupt_trigger::trigger_interrupt(gb_interrupt interrupt,
                                                  int irq_clock_cycle)
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
        AGE_GB_CLOG_IRQS("denying interrupt request " << AGE_LOG_HEX8(intr_bit)
                         << " on clock cycle " << irq_clock_cycle
                         << " (currently being dispatched)");
        return;
    }

    AGE_GB_CLOG_IRQS("interrupt " << AGE_LOG_HEX8(intr_bit)
                     <<" requested on clock cycle " << irq_clock_cycle);
    m_if |= intr_bit;

    if (!m_halted)
    {
        return;
    }

    // terminate HALT mode
    m_halted = false;

    // We know from Gambatte timer tests that leaving HALT mode
    // takes one additional machine cycle (DMG and CGB) for a timer
    // interrupt.
    //
    // Doing this for all interrupts breaks the following
    // Mooneye GB tests though:
    //      acceptance/halt_ime0_nointr_timing (v-blank interrupt)
    //      acceptance/halt_ime1_timing2-GS    (v-blank interrupt)
    //      acceptance/ppu/intr_2_mode0_timing (lcd mode-2 interrupt)
    //      <... there may be more ...>
    //

    //! \todo Gambatte: always delay for CGB (analyse test roms)
    if (m_device.is_cgb())
    {
        m_clock.tick_machine_cycle();
        AGE_GB_CLOG_IRQS("    * HALT termination M-cycle (CGB)");
        return;
    }

    //! \todo Gambatte: delay depends on sub-m-cycle timing (analyse test roms)
    int clk_current = m_clock.get_clock_cycle();
    AGE_ASSERT(clk_current >= irq_clock_cycle);
    int clks_diff = clk_current - irq_clock_cycle;
    int half_mcycle = m_clock.get_machine_cycle_clocks() >> 1;

    if (clks_diff < half_mcycle)
    {
        m_clock.tick_machine_cycle();
        AGE_GB_CLOG_IRQS("    * HALT termination M-cycle (irq "
                         << clks_diff << " T4-cycles ago)");
    }

    AGE_GB_CLOG_IRQS("    * no HALT termination M-cycle");
}





//---------------------------------------------------------
//
//   interrupt i/o ports
//
//---------------------------------------------------------

age::uint8_t age::gb_interrupt_ports::read_if() const
{
    AGE_GB_CLOG_IRQS("read IF " << AGE_LOG_HEX8(m_if));
    return m_if;
}

age::uint8_t age::gb_interrupt_ports::read_ie() const
{
    AGE_GB_CLOG_IRQS("read IE " << AGE_LOG_HEX8(m_if));
    return m_ie;
}



void age::gb_interrupt_ports::write_if(uint8_t value)
{
    AGE_ASSERT(!m_halted);
    AGE_GB_CLOG_IRQS("write IF " << AGE_LOG_HEX8(value));
    m_if = value | 0xE0;
}

void age::gb_interrupt_ports::write_ie(uint8_t value)
{
    AGE_ASSERT(!m_halted);
    AGE_GB_CLOG_IRQS("write IE " << AGE_LOG_HEX8(value));
    m_ie = value;
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
    if (ime == m_ime)
    {
        AGE_GB_CLOG_IRQS("(interrupt dispatching already "
                         << (ime ? "enabled)" : "disabled)"));
        return;
    }
    AGE_GB_CLOG_IRQS("interrupt dispatching " << (ime ? "enabled" : "disabled"));
    m_ime = ime;
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
    AGE_GB_CLOG_IRQS("clear IF flag " << AGE_LOG_HEX8(interrupt_bit));
}

void age::gb_interrupt_dispatcher::finish_dispatch()
{
    AGE_GB_CLOG_IRQS("interrupt dispatching disabled");
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

    if (m_if & m_ie & 0x1F)
    {
        AGE_GB_CLOG_IRQS("HALT immediately terminated by pending interrupts");
        return;
    }

    m_halted = true;
    AGE_GB_CLOG_IRQS("HALTed");
}
