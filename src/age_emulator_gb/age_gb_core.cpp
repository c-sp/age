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

#include <algorithm>

#include <age_debug.hpp>

#include "age_gb_core.hpp"

#if 0
#define LOG(x) AGE_LOG("cycle " << m_oscillation_cycle << ": " << x)
#else
#define LOG(x)
#endif

constexpr const age::uint8_array<17> gb_interrupt_pc_lookup =
{{
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


age::gb_core::gb_core(gb_mode mode)
    : m_mode(mode)
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
    if (is_cgb())
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



int age::gb_core::get_oscillation_cycle() const
{
    AGE_ASSERT(m_oscillation_cycle != gb_no_cycle);
    return m_oscillation_cycle;
}

age::int8_t age::gb_core::get_machine_cycles_per_cpu_cycle() const
{
    return m_machine_cycles_per_cpu_cycle;
}

bool age::gb_core::is_double_speed() const
{
    return m_double_speed;
}

bool age::gb_core::is_cgb() const
{
    return m_mode == gb_mode::cgb;
}

bool age::gb_core::is_cgb_hardware() const
{
    return m_mode != gb_mode::dmg;
}

age::gb_mode age::gb_core::get_mode() const
{
    return m_mode;
}

age::gb_state age::gb_core::get_state() const
{
    return m_state;
}



void age::gb_core::oscillate_cpu_cycle()
{
    m_oscillation_cycle += m_machine_cycles_per_cpu_cycle;
}

void age::gb_core::oscillate_2_cycles()
{
    m_oscillation_cycle += 2;
}

void age::gb_core::insert_event(int cycle_offset, gb_event event)
{
    AGE_ASSERT(cycle_offset >= 0);
    AGE_ASSERT(int_max - cycle_offset >= m_oscillation_cycle);
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

int age::gb_core::get_event_cycle(gb_event event) const
{
    return m_events.get_event_cycle(event);
}

void age::gb_core::set_back_cycles(int offset)
{
    AGE_GB_SET_BACK_CYCLES(m_oscillation_cycle, offset);
    m_events.set_back_cycles(offset);
}



void age::gb_core::start_dma()
{
    m_state = gb_state::dma;
}

void age::gb_core::finish_dma()
{
    m_state = m_halt ? gb_state::halted : gb_state::cpu_active;
}



void age::gb_core::request_interrupt(gb_interrupt interrupt)
{
    m_if |= to_integral(interrupt);
    check_halt_mode();
    LOG("interrupt requested: " << AGE_LOG_DEC(to_integral(interrupt)));
}

void age::gb_core::ei_delayed()
{
    AGE_ASSERT(m_state == gb_state::cpu_active);
    m_ei = !m_ime;
    LOG("ei " << m_ei << ", ime " << m_ime);
}

void age::gb_core::ei_now()
{
    AGE_ASSERT(m_state == gb_state::cpu_active);
    m_ei = false;
    m_ime = true;
    LOG("ei " << m_ei << ", ime " << m_ime);
}

void age::gb_core::di()
{
    AGE_ASSERT(m_state == gb_state::cpu_active);
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
    AGE_ASSERT(m_state == gb_state::cpu_active);
    LOG("halted");
    m_halt = true;
    m_state = gb_state::halted;
    check_halt_mode();
    return !m_ime && !m_halt;
}

void age::gb_core::stop()
{
    AGE_ASSERT(m_state == gb_state::cpu_active);
    if (is_cgb() && ((m_key1 & 0x01) > 0))
    {
        m_key1 ^= 0x81;
    }
    m_double_speed = (m_key1 & 0x80) > 0;
    m_machine_cycles_per_cpu_cycle = m_double_speed ? 2 : 4;
    LOG("double speed " << m_double_speed << ", machine cycles per cpu cycle " << m_machine_cycles_per_cpu_cycle);
    insert_event(0, gb_event::switch_double_speed);
}

bool age::gb_core::must_service_interrupt()
{
    AGE_ASSERT(m_state == gb_state::cpu_active);
    bool result = false;

    // we assume that m_ime is false, if m_ei is true
    if (m_ei)
    {
        m_ime = true;
        m_ei = false;
        LOG("ei " << m_ei << ", ime " << m_ime);
    }
    else if (m_ime)
    {
        int interrupt = m_ie & m_if & 0x1F;
        result = (interrupt > 0);

        if (result)
        {
            LOG("noticed interrupt " << AGE_LOG_HEX(interrupt));
            AGE_ASSERT(!m_halt); // should have been terminated by write_iX() call already
            m_ime = false; // disable interrupts
        }
    }

    return result;
}

age::uint8_t age::gb_core::get_interrupt_to_service()
{
    AGE_ASSERT(m_state == gb_state::cpu_active);

    uint8_t interrupt = m_ie & m_if & 0x1F;
    if (interrupt > 0)
    {
        // lower interrupt bits have higher priority
        interrupt &= ~(interrupt - 1); // get lowest interrupt bit that is set
        m_if &= ~interrupt; // clear interrupt bit

        AGE_ASSERT(   (interrupt == 0x01)
                   || (interrupt == 0x02)
                   || (interrupt == 0x04)
                   || (interrupt == 0x08)
                   || (interrupt == 0x10));

        interrupt = gb_interrupt_pc_lookup[interrupt];

        AGE_ASSERT(interrupt > 0);
    }

    return interrupt;
}



age::uint8_t age::gb_core::read_key1() const
{
    return m_key1;
}

age::uint8_t age::gb_core::read_if() const
{
    LOG(AGE_LOG_HEX(m_if));
    return m_if;
}

age::uint8_t age::gb_core::read_ie() const
{
    return m_ie;
}



void age::gb_core::write_key1(uint8_t value)
{
    LOG(AGE_LOG_HEX(value));
    m_key1 = (m_key1 & 0xFE) | (value & 0x01);
}

void age::gb_core::write_if(uint8_t value)
{
    LOG(AGE_LOG_HEX(value));
    m_if = value | 0xE0;
    check_halt_mode();
}

void age::gb_core::write_ie(uint8_t value)
{
    LOG(AGE_LOG_HEX(value));
    m_ie = value;
    check_halt_mode();
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

void age::gb_core::check_halt_mode()
{
    // terminate halt state once ie & if > 0, even if ime is cleared
    if (m_halt && ((m_if & m_ie & 0x1F) > 0))
    {
        m_halt = false;
        if (m_state == gb_state::halted)
        {
            m_state = gb_state::cpu_active;
        }

        // seen in gambatte source code, no other source found on this yet:
        // for CGB, unhalting on interrupt takes an additional 4 cycles
        // (however, some gambatte tests check this indirectly by relying
        // on interrupts during a halt)
        if (is_cgb())
        {
            oscillate_cpu_cycle();
        }

        LOG("halt state terminated");
    }
}



//---------------------------------------------------------
//
//   gb_events class
//
//---------------------------------------------------------

age::gb_core::gb_events::gb_events()
{
    std::for_each(begin(m_event_cycle), end(m_event_cycle), [&](auto &elem)
    {
        elem = gb_no_cycle;
    });
//    for (size_t i = 0; i < m_event_cycle.size(); ++i)
//    {
//        m_event_cycle[i] = gb_no_cycle;
//    }
}

int age::gb_core::gb_events::get_event_cycle(gb_event event) const
{
    return m_event_cycle[to_integral(event)];
}

age::gb_event age::gb_core::gb_events::poll_event(int current_cycle)
{
    auto result = gb_event::none;

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

void age::gb_core::gb_events::insert_event(int for_cycle, gb_event event)
{
    auto event_id = to_integral(event);

    auto old_cycle = m_event_cycle[event_id];
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
        m_events.insert(std::pair<int, gb_event>(for_cycle, event));
    }
    m_event_cycle[event_id] = for_cycle;
}

void age::gb_core::gb_events::set_back_cycles(int offset)
{
    // find all scheduled events
    std::vector<gb_event> events_to_adjust;
    std::for_each(begin(m_events), end(m_events), [&](const auto &pair)
    {
        events_to_adjust.push_back(pair.second);
    });

    // re-schedule all found events
    std::for_each(begin(events_to_adjust), end(events_to_adjust), [&](const gb_event &event)
    {
        auto idx = to_integral(event);
        auto event_cycle = m_event_cycle[idx];
        AGE_GB_SET_BACK_CYCLES(event_cycle, offset);
        insert_event(event_cycle, event);
    });
}
