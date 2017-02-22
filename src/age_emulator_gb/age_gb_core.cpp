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

#include "age_gb_core.hpp"

#if 0
#define LOG(x) if (m_oscillation_cycle < 15000) { AGE_LOG("cycle " << m_oscillation_cycle << ": " << x); }
#else
#define LOG(x)
#endif



age::gb_core::gb_core(bool cgb)
    : m_cgb(cgb)
{
    //
    // verified by gambatte tests
    //
    // adjust the initial oscillation cycle
    // (due to the Gameboy playing the nintendo intro, thus we're not
    // starting with cycle 0 at PC 0x0100)
    //
    //      div/start_inc_1_cgb_out1E
    //      div/start_inc_2_cgb_out1F
    //      div/start_inc_1_dmg08_outAB
    //      div/start_inc_2_dmg08_outAC
    //
    //      tima/tc00_start_1_outF0
    //      tima/tc00_start_2_outF1
    //
    if (m_cgb)
    {
        m_oscillation_cycle = 0x1F * 0x100;
        m_oscillation_cycle -= 96;
    }
    else
    {
        m_oscillation_cycle = 0xAC * 0x100;
        m_oscillation_cycle -= 52;
    }
}



age::uint age::gb_core::get_oscillation_cycle() const
{
    AGE_ASSERT(m_oscillation_cycle != gb_no_cycle);
    return m_oscillation_cycle;
}

age::uint age::gb_core::get_cycles_per_cpu_tick() const
{
    return m_cycles_per_cpu_tick;
}

bool age::gb_core::is_double_speed() const
{
    return m_double_speed;
}

bool age::gb_core::is_cgb() const
{
    return m_cgb;
}

age::gb_mode age::gb_core::get_mode() const
{
    return m_mode;
}



void age::gb_core::oscillate_cpu_tick()
{
    m_oscillation_cycle += m_cycles_per_cpu_tick;
}

void age::gb_core::oscillate_2_cycles()
{
    m_oscillation_cycle += 2;
}

void age::gb_core::insert_event(uint cycle_offset, gb_event event)
{
    m_events.insert_event(m_oscillation_cycle + cycle_offset, event);
}

void age::gb_core::remove_event(gb_event event)
{
    m_events.insert_event(gb_no_cycle, event);
}

age::gb_event age::gb_core::poll_event()
{
    return m_events.poll_event(m_oscillation_cycle);
}

age::uint age::gb_core::get_event_cycle(gb_event event) const
{
    return m_events.get_event_cycle(event);
}



void age::gb_core::start_dma()
{
    m_mode = gb_mode::dma;
}

void age::gb_core::finish_dma()
{
    m_mode = m_halt ? gb_mode::halted : gb_mode::cpu_active;
}



void age::gb_core::request_interrupt(gb_interrupt interrupt)
{
    m_if |= to_integral(interrupt);
    check_halt_mode();
    LOG("interrupt requested: " << (uint)to_integral(interrupt));
}

void age::gb_core::ei()
{
    AGE_ASSERT(m_mode == gb_mode::cpu_active);
    m_ei = !m_ime;
    LOG("ei " << m_ei << ", ime " << m_ime);
}

void age::gb_core::di()
{
    AGE_ASSERT(m_mode == gb_mode::cpu_active);
    m_ei = false;
    m_ime = false;
    LOG("ei " << m_ei << ", ime " << m_ime);
}

bool age::gb_core::halt()
{
    // demotronic demo:
    //   - ie > 0
    //   - if = ime = 0
    //   - halt pauses, until interrupt would occur (if & ie > 0)
    //   - however, the interrupt is not serviced because ime = 0
    //  => special halt treatment only for m_ei = 0?
    AGE_ASSERT(m_mode == gb_mode::cpu_active);
    LOG("halted");
    m_halt = true;
    m_mode = gb_mode::halted;
    check_halt_mode();
    return !m_ime && !m_halt;
}

void age::gb_core::stop()
{
    AGE_ASSERT(m_mode == gb_mode::cpu_active);
    if (m_cgb && ((m_key1 & 0x01) > 0))
    {
        m_key1 ^= 0x81;
    }
    m_double_speed = (m_key1 & 0x80) > 0;
    m_cycles_per_cpu_tick = m_double_speed ? 2 : 4;
    LOG("double speed " << m_double_speed << ", cycles per cpu tick " << m_cycles_per_cpu_tick);
    insert_event(0, gb_event::switch_double_speed);
}

age::uint16 age::gb_core::get_interrupt_to_service()
{
    AGE_ASSERT(m_mode == gb_mode::cpu_active);
    uint16 result = 0;

    // we assume that m_ime is false, if m_ei is true
    if (m_ei)
    {
        m_ime = true;
        m_ei = false;
        LOG("ei " << m_ei << ", ime " << m_ime);
    }
    else if (m_ime)
    {
        uint8 interrupts = m_ie & m_if;
        if (interrupts > 0)
        {
            AGE_ASSERT(!m_halt); // should have been terminated by write_iX() call already

            // lower interrupt bits have higher priority
            interrupts &= ~(interrupts - 1); // get lowest interrupt bit that is set
            AGE_ASSERT(interrupts <= 0x60);

            m_if &= ~interrupts; // clear interrupt bit
            m_ime = false; // disable interrupts

            result = (interrupts < 8) ? gb_interrupt_pc_lookup[interrupts] : interrupts + 0x50;
            LOG("noticed interrupt 0x" << std::hex << result << std::dec);
        }
    }

    return result;
}



age::uint8 age::gb_core::read_key1() const
{
    return m_key1;
}

age::uint8 age::gb_core::read_if() const
{
    LOG((uint)(m_if & 0x1F));
    return m_if;
}

age::uint8 age::gb_core::read_ie() const
{
    return m_ie;
}



void age::gb_core::write_key1(uint8 value)
{
    LOG((uint)value);
    m_key1 = (m_key1 & 0xFE) | (value & 0x01);
}

void age::gb_core::write_if(uint8 value)
{
    LOG((uint)value);
    m_if = value | 0xE0;
    check_halt_mode();
}

void age::gb_core::write_ie(uint8 value)
{
    LOG((uint)value);
    m_ie = value & 0x1F;
    check_halt_mode();
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

void age::gb_core::check_halt_mode()
{
    // terminate halt mode once ie & if > 0, even if ime is cleared
    if (m_halt && ((m_if & m_ie) > 0))
    {
        m_halt = false;
        if (m_mode == gb_mode::halted)
        {
            m_mode = gb_mode::cpu_active;
        }

        // seen in gambatte source code, no other source found on this yet:
        // for CGB, unhalting on interrupt takes an additional 4 cycles
        // (however, some gambatte tests check this indirectly by relying
        // on interrupts during a halt)
        if (m_cgb)
        {
            oscillate_cpu_tick();
        }

        LOG("halt mode terminated");
    }
}



//---------------------------------------------------------
//
//   gb_events class
//
//---------------------------------------------------------

age::gb_core::gb_events::gb_events()
{
    for (uint i = 0; i < m_event_cycle.size(); ++i)
    {
        m_event_cycle[i] = gb_no_cycle;
    }
}

age::uint age::gb_core::gb_events::get_event_cycle(gb_event event) const
{
    return m_event_cycle[to_integral(event)];
}

age::gb_event age::gb_core::gb_events::poll_event(uint current_cycle)
{
    gb_event result = gb_event::none;

    auto itr = begin(m_events);
    if (itr != end(m_events))
    {
        if (itr->first <= current_cycle)
        {
            result = itr->second;
            m_events.erase(itr);
            m_event_cycle[to_integral(result)] = gb_no_cycle;
        }
    }

    return result;
}

void age::gb_core::gb_events::insert_event(uint for_cycle, gb_event event)
{
    uint event_id = to_integral(event);

    uint old_cycle = m_event_cycle[event_id];
    if (old_cycle != gb_no_cycle)
    {
        auto range = m_events.equal_range(old_cycle);
        for (auto itr = range.first; itr != range.second; ++itr)
        {
            if (itr->second == event)
            {
                m_events.erase(itr);
                break;
            }
        }
    }

    if (for_cycle != gb_no_cycle)
    {
        m_events.insert(std::pair<uint, gb_event>(for_cycle, event));
    }
    m_event_cycle[event_id] = for_cycle;
}
