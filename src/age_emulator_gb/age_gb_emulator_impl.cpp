//
// © 2017 Christoph Sprenger <https://github.com/c-sp>
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

#include "age_gb_emulator_impl.hpp"

#include <cassert>



std::string age::gb_emulator_impl::get_emulator_title() const
{
    constexpr char ascii_white_space = 0x20;
    constexpr char ascii_underscore  = 0x5F;
    constexpr char ascii_0           = 0x30;
    constexpr char ascii_9           = 0x39;
    constexpr char ascii_a           = 0x61;
    constexpr char ascii_z           = 0x7A;
    constexpr char ascii_A           = 0x41;
    constexpr char ascii_Z           = 0x5A;

    auto        cart_title = m_memory.get_cartridge_title();
    std::string result;

    for (char c : cart_title)
    {
        // translate white spaces to underscores
        if (c == ascii_white_space)
        {
            c = ascii_underscore;
        }

        // stop on the first invalid character
        if ((c != ascii_underscore)
            && ((c < ascii_0) || (c > ascii_9))
            && ((c < ascii_a) || (c > ascii_z))
            && ((c < ascii_A) || (c > ascii_Z)))
        {
            break;
        }

        // add character to result
        result.append(1, c);

        // stop if we hit the length limit
        if (result.length() >= 32)
        {
            break;
        }
    }

    return result;
}

age::int16_t age::gb_emulator_impl::get_screen_width() const
{
    return m_screen_buffer.get_screen_width();
}

age::int16_t age::gb_emulator_impl::get_screen_height() const
{
    return m_screen_buffer.get_screen_height();
}

const age::pixel_vector& age::gb_emulator_impl::get_screen_front_buffer() const
{
    return m_screen_buffer.get_front_buffer();
}

const age::pcm_vector& age::gb_emulator_impl::get_audio_buffer() const
{
    return m_audio_buffer;
}

int age::gb_emulator_impl::get_pcm_sampling_rate() const
{
    return gb_clock_cycles_per_second / 2;
}

int age::gb_emulator_impl::get_cycles_per_second() const
{
    return gb_clock_cycles_per_second;
}

age::int64_t age::gb_emulator_impl::get_emulated_cycles() const
{
    return m_emulated_cycles;
}

age::uint8_vector age::gb_emulator_impl::get_persistent_ram() const
{
    return m_memory.get_persistent_ram();
}

void age::gb_emulator_impl::set_persistent_ram(const uint8_vector& source)
{
    m_memory.set_persistent_ram(source);
}

void age::gb_emulator_impl::set_buttons_down(int buttons)
{
    m_joypad.set_buttons_down(buttons);
}

void age::gb_emulator_impl::set_buttons_up(int buttons)
{
    m_joypad.set_buttons_up(buttons);
}

bool age::gb_emulator_impl::emulate(int cycles_to_emulate)
{
    if (cycles_to_emulate <= 0)
    {
        return false;
    }

    auto frame_id = m_screen_buffer.get_current_frame_id();
    m_audio_buffer.clear();

    int emulated_cycles = emulate_cycles(cycles_to_emulate);
    assert(emulated_cycles > 0);
    m_emulated_cycles += emulated_cycles;

    return m_screen_buffer.get_current_frame_id() != frame_id;
}

age::gb_test_info age::gb_emulator_impl::get_test_info() const
{
    return m_cpu.get_test_info();
}

std::vector<age::gb_log_entry> age::gb_emulator_impl::get_and_clear_log_entries()
{
    return m_logger.get_and_clear_log_entries();
}



int age::gb_emulator_impl::emulate_cycles(int cycles_to_emulate)
{
    assert(cycles_to_emulate > 0);

    // make sure we have some headroom since we usually emulate
    // a few more cycles than requested
    constexpr int cycle_setback_limit = 2 * gb_clock_cycles_per_second;
    constexpr int cycle_limit         = int_max - cycle_setback_limit - gb_clock_cycles_per_second;

    // calculate the number of cycles to emulate based on the current
    // cycle and the cycle limit
    int starting_cycle = m_clock.get_clock_cycle();
    assert(starting_cycle < cycle_setback_limit);

    int cycle_to_reach = starting_cycle + std::min(cycles_to_emulate, cycle_limit - starting_cycle);

    // emulate until we reach the calculated cycle
    // (depending on CPU instruction length or running DMA
    // we usually emulate a bit past that cycle)
    while (m_clock.get_clock_cycle() < cycle_to_reach)
    {
        if (m_bus.handle_gp_dma())
        {
            assert(m_device.cgb_mode());
        }
        else if (m_interrupts.halted() || m_cpu.is_frozen())
        {
            int fast_forward_cycles = get_fast_forward_halt_cycles(cycle_to_reach);
            assert(fast_forward_cycles >= 0);
            m_interrupts.log() << "CPU halted ("
                               << (m_clock.is_double_speed() ? "double" : "normal")
                               << " speed), fast forwarding to clock cycle "
                               << (m_clock.get_clock_cycle() + fast_forward_cycles)
                               << " (skipping " << fast_forward_cycles << " clock cycles)";
            m_clock.tick_clock_cycles(fast_forward_cycles);
            // handle events:
            //  - HALT: may be terminated by interrupt
            //  - frozen CPU: keep overall state consistent to prevent set_back_clock() errors
            m_bus.handle_events();
        }
        else
        {
            m_cpu.emulate();
        }
    }

    // generate remaining sound samples
    m_sound.update_state();

    // finish the next LCD frame, if possible
    // (not using m_lcd.update_state() reduces fifo-rendering for some roms)
    m_lcd.check_for_finished_frame();

    // calculate the cycles actually emulated
    int current_cycle   = m_clock.get_clock_cycle();
    int cycles_emulated = current_cycle - starting_cycle;
    assert(cycles_emulated >= 0);

    // if the cycle counter reaches a certain threshold,
    // set back all stored cycle values to keep the
    // cycle counter from overflowing
    if (current_cycle >= cycle_setback_limit)
    {
        m_timer.update_state();
        m_memory.update_state();

        // keep a minimum of cycles to prevent negative cycle values
        // (which should still work but is kind of unintuitive)
        int cycles_to_keep = gb_clock_cycles_per_second
                             + (current_cycle % gb_clock_cycles_per_second);

        int clock_cycle_offset = current_cycle - cycles_to_keep;
        assert(clock_cycle_offset > 0);
        assert(clock_cycle_offset < current_cycle);

        m_clock.set_back_clock(clock_cycle_offset);
        m_logger.set_back_clock(clock_cycle_offset); // this goes second to not mess up the logs
        m_memory.set_back_clock(clock_cycle_offset);
        m_events.set_back_clock(clock_cycle_offset);
        m_sound.set_back_clock(clock_cycle_offset);
        m_lcd.set_back_clock(clock_cycle_offset);
        m_timer.set_back_clock(clock_cycle_offset);
        m_serial.set_back_clock(clock_cycle_offset);
        m_bus.set_back_clock(clock_cycle_offset);
    }

    return cycles_emulated;
}

int age::gb_emulator_impl::get_fast_forward_halt_cycles(int cycle_to_reach) const
{
    int current_clk = m_clock.get_clock_cycle();
    assert(current_clk < cycle_to_reach);

    // get the maximal cycle we can fast-forward to
    int next_event_cycle   = m_events.get_next_event_cycle();
    int fast_forward_cycle = (next_event_cycle == gb_no_clock_cycle)
                                 ? cycle_to_reach
                                 : std::min(cycle_to_reach, next_event_cycle);

    // make sure we fast-forward in complete m-cycles
    // (don't align absolute clock cycles to not mess up the alignment,
    // e.g. after multiple speed switches)
    int fast_forward_clk_diff = fast_forward_cycle - current_clk;
    int t4_cycles             = m_clock.get_machine_cycle_clocks();
    int fraction              = fast_forward_clk_diff & (t4_cycles - 1);
    if (fraction)
    {
        fast_forward_clk_diff += t4_cycles - fraction;
    }

    return fast_forward_clk_diff;
}



//---------------------------------------------------------
//
//   object creation & destruction
//
//---------------------------------------------------------

age::gb_emulator_impl::gb_emulator_impl(const uint8_vector& rom,
                                        gb_device_type      device_type,
                                        gb_colors_hint      colors_hint,
                                        gb_log_categories   log_categories)

    : m_screen_buffer(gb_screen_width, gb_screen_height),
      m_logger(std::move(log_categories)),
      m_device(rom, device_type),
      m_clock(m_logger, m_device),
      m_memory(rom, m_clock, m_device.is_cgb_device()),
      m_interrupts(m_device, m_clock),
      m_events(m_clock),
      m_sound(m_device, m_clock, m_audio_buffer),
      m_lcd(m_device, m_clock, m_memory.get_video_ram(), m_memory.get_rom_header(), m_events, m_interrupts, m_screen_buffer, colors_hint),
      m_timer(m_device, m_clock, m_interrupts, m_events),
      m_joypad(m_device, m_interrupts),
      m_serial(m_device, m_clock, m_interrupts, m_events),
      m_bus(m_device, m_clock, m_interrupts, m_events, m_memory, m_sound, m_lcd, m_timer, m_joypad, m_serial),
      m_cpu(m_device, m_clock, m_events, m_interrupts, m_bus)
{
}
