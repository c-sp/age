//
// Copyright 2020 Christoph Sprenger
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

#include <age_debug.hpp>

#include "age_gb_cpu.hpp"



namespace
{
    constexpr const age::uint8_array<17> interrupt_pc_lookup
        = {{
            0,
            0x40,
            0x48,
            0,
            0x50,
            0,
            0,
            0,
            0x58,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0x60,
        }};

    constexpr const std::array interrupt_name{
        "",
        "v-blank",
        "lcd",
        "",
        "timer",
        "",
        "",
        "",
        "serial transfer",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "joypad",
    };

} // namespace



age::gb_cpu::gb_cpu(const gb_device&         device,
                    gb_clock&                clock,
                    gb_interrupt_dispatcher& interrupts,
                    gb_bus&                  bus)
    : m_device(device),
      m_clock(clock),
      m_interrupts(interrupts),
      m_bus(bus),
      m_pc(0x0100),
      m_sp(0xFFFE),
      m_prefetched_opcode(m_bus.read_byte(m_pc))
{
    if (m_device.is_dmg_device())
    {
        m_a = 0x01;
        set_flags(0xB0);
        m_b = 0x00;
        m_c = 0x13;
        m_d = 0x00;
        m_e = 0xD8;
        m_h = 0x01;
        m_l = 0x4D;
    }
    else
    {
        m_a = 0x11;
        set_flags(0x80);
        m_b = 0x00;
        m_c = 0x00;
        m_d = 0x00;
        m_e = 0x08;
        m_h = 0x00;
        m_l = 0x7C;
    }
}



bool age::gb_cpu::is_frozen() const
{
    return m_cpu_state & gb_cpu_state_frozen;
}

age::gb_test_info age::gb_cpu::get_test_info() const
{
    gb_test_info result;

    result.m_ld_b_b = m_ld_b_b;
    result.m_a      = m_a;
    result.m_b      = m_b;
    result.m_c      = m_c;
    result.m_d      = m_d;
    result.m_e      = m_e;
    result.m_h      = m_h;
    result.m_l      = m_l;

    return result;
}



void age::gb_cpu::emulate()
{
    AGE_ASSERT(!(m_cpu_state & gb_cpu_state_frozen))

    // EI - delayed interrupts enabling
    if (m_cpu_state & gb_cpu_state_ei)
    {
        AGE_ASSERT(!m_interrupts.get_ime())
        execute_prefetched();

        // enable interrupts only if this instruction was no DI
        if (m_cpu_state & gb_cpu_state_ei)
        {
            m_interrupts.set_ime(true, "EI (delayed)");
            m_cpu_state &= ~gb_cpu_state_ei;
        }
        else
        {
            m_interrupts.log() << "delayed interrupt enabling prevented by DI";
        }

        // we're done here
        return;
    }

    // look for any interrupt to dispatch
    m_bus.handle_events(); // make sure the IF register is up to date
    if (m_interrupts.next_interrupt_bit())
    {
        dispatch_interrupt();
        return;
    }

    // just execute the next instruction
    execute_prefetched();
}



void age::gb_cpu::dispatch_interrupt()
{
    m_interrupts.log() << "about to dispatch interrupt"
                       << ", current PC == " << log_hex16(m_pc);

    m_clock.tick_machine_cycle();
    m_clock.tick_machine_cycle();

    // Writing IE or IF during interrupt dispatching may influence the
    // interrupt being dispatched.
    // IE can be (indirectly) written by setting SP to 0x0000 or 0x0001
    // before the interrupt is handled.
    // For writing IF set SP to 0xFF11 or 0xFF10.

    // Writing IE here will influence interrupt dispatching.
    // Writing IF here will influence interrupt dispatching.
    tick_push_byte(m_pc >> 8);

    m_bus.handle_events(); // make sure the IF register is up to date
    uint8_t intr_bit = m_interrupts.next_interrupt_bit();
    AGE_ASSERT((intr_bit == 0x00)
               || (intr_bit == 0x01)
               || (intr_bit == 0x02)
               || (intr_bit == 0x04)
               || (intr_bit == 0x08)
               || (intr_bit == 0x10))

    // Pushing the lower PC byte happens before clearing the interrupt's
    // IF bit (checked by pushing to IF).
    tick_push_byte(m_pc);

    m_interrupts.clear_interrupt_flag(intr_bit);

    m_pc                = interrupt_pc_lookup[intr_bit];
    m_prefetched_opcode = tick_read_byte(m_pc);

    m_interrupts.finish_dispatch();

    m_interrupts.log() << "interrupt " << log_hex8(intr_bit)
                       << " (" << interrupt_name[intr_bit] << ")"
                       << " dispatched to " << log_hex16(m_pc);
}
