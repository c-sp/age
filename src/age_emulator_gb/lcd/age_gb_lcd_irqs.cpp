//
// © 2020 Christoph Sprenger <https://github.com/c-sp>
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

#include "age_gb_lcd.hpp"

#include <cassert>



namespace
{
    int add_total_frames(int clk_last, int clk_current)
    {
        assert(clk_last <= clk_current);

        int clk_diff = clk_current - clk_last;
        int frames   = 1 + clk_diff / age::gb_clock_cycles_per_lcd_frame;

        return frames * age::gb_clock_cycles_per_lcd_frame;
    }

} // namespace



age::gb_lcd_irqs::gb_lcd_irqs(const gb_device&      device,
                              const gb_clock&       clock,
                              const gb_lcd_line&    line,
                              gb_events&            events,
                              gb_interrupt_trigger& interrupts)
    : m_device(device),
      m_clock(clock),
      m_line(line),
      m_events(events),
      m_interrupts(interrupts)
{
    int clk_current     = m_clock.get_clock_cycle();
    int clk_frame_start = m_line.clk_frame_start();
    assert(m_line.lcd_is_on());
    assert(clk_frame_start <= clk_current);

    m_clk_next_irq_vblank = clk_frame_start
                            + gb_screen_height * gb_clock_cycles_per_lcd_line;
    if (m_clk_next_irq_vblank < clk_current)
    {
        m_clk_next_irq_vblank += add_total_frames(m_clk_next_irq_vblank, clk_current);
    }
    assert(m_clk_next_irq_vblank > clk_current);

    m_events.schedule_event(gb_event::lcd_interrupt_vblank, m_clk_next_irq_vblank - clk_current);
}



age::uint8_t age::gb_lcd_irqs::read_stat() const
{
    return m_stat;
}

void age::gb_lcd_irqs::write_stat(uint8_t value, int scx)
{
    m_stat = 0x80 | (value & ~(gb_stat_modes | gb_stat_ly_match));

    if (m_line.lcd_is_on())
    {
        schedule_irq_lyc();
        schedule_irq_mode2();
        schedule_irq_mode0(scx);
    }
}



void age::gb_lcd_irqs::lcd_on(int scx)
{
    schedule_irq_vblank();
    schedule_irq_lyc();
    schedule_irq_mode2();
    schedule_irq_mode0(scx);
}

void age::gb_lcd_irqs::lcd_off()
{
    m_clk_next_irq_vblank = gb_no_clock_cycle;
    m_clk_next_irq_lyc    = gb_no_clock_cycle;
    m_clk_next_irq_mode2  = gb_no_clock_cycle;
    m_clk_next_irq_mode0  = gb_no_clock_cycle;

    m_events.remove_event(gb_event::lcd_interrupt_vblank);
    m_events.remove_event(gb_event::lcd_interrupt_lyc);
    m_events.remove_event(gb_event::lcd_interrupt_mode2);
    m_events.remove_event(gb_event::lcd_interrupt_mode0);
}

void age::gb_lcd_irqs::align_after_speed_change(int clock_cycle_offset)
{
    assert(m_line.lcd_is_on());
    assert(m_clock.is_double_speed());
    int clk_current = m_clock.get_clock_cycle();

    if (m_clk_next_irq_vblank != gb_no_clock_cycle)
    {
        m_clk_next_irq_vblank += clock_cycle_offset;
        m_events.schedule_event(gb_event::lcd_interrupt_vblank, m_clk_next_irq_vblank - clk_current);

        m_line.log() << "aligning v-blank IRQ (+" << clock_cycle_offset << "), next v-blank IRQ in "
                     << log_in_clks(m_clk_next_irq_vblank - clk_current, clk_current);
    }
    if (m_clk_next_irq_lyc != gb_no_clock_cycle)
    {
        m_clk_next_irq_lyc += clock_cycle_offset;
        m_events.schedule_event(gb_event::lcd_interrupt_lyc, m_clk_next_irq_lyc - clk_current);

        m_line.log() << "aligning LYC IRQ (+" << clock_cycle_offset << "), next LYC IRQ in "
                     << log_in_clks(m_clk_next_irq_lyc - clk_current, clk_current);
    }
    if (m_clk_next_irq_mode2 != gb_no_clock_cycle)
    {
        m_clk_next_irq_mode2 += clock_cycle_offset;
        m_events.schedule_event(gb_event::lcd_interrupt_mode2, m_clk_next_irq_mode2 - clk_current);

        m_line.log() << "aligning mode 2 IRQ (+" << clock_cycle_offset << "), next mode 2 IRQ in "
                     << log_in_clks(m_clk_next_irq_mode2 - clk_current, clk_current);
    }
    if (m_clk_next_irq_mode0 != gb_no_clock_cycle)
    {
        m_clk_next_irq_mode0 += clock_cycle_offset;
        m_events.schedule_event(gb_event::lcd_interrupt_mode0, m_clk_next_irq_mode0 - clk_current);

        m_line.log() << "aligning mode 0 IRQ (+" << clock_cycle_offset << "), next mode 0 IRQ in "
                     << log_in_clks(m_clk_next_irq_mode0 - clk_current, clk_current);
    }
}

void age::gb_lcd_irqs::set_back_clock(int clock_cycle_offset)
{
    gb_set_back_clock_cycle(m_clk_next_irq_vblank, clock_cycle_offset);
    gb_set_back_clock_cycle(m_clk_next_irq_lyc, clock_cycle_offset);
    gb_set_back_clock_cycle(m_clk_next_irq_mode2, clock_cycle_offset);
    gb_set_back_clock_cycle(m_clk_next_irq_mode0, clock_cycle_offset);
}



//---------------------------------------------------------
//
//   v-blank irq
//
//---------------------------------------------------------

void age::gb_lcd_irqs::trigger_irq_vblank()
{
    assert(m_clk_next_irq_vblank != gb_no_clock_cycle);
    assert(m_clk_next_irq_vblank <= m_clock.get_clock_cycle());

    m_interrupts.trigger_interrupt(gb_interrupt::vblank, m_clk_next_irq_vblank);
    m_interrupts.log() << "v-blank interrupt occurred" << log_line_clks(m_line, m_clk_next_irq_vblank);
    if (m_stat & gb_stat_irq_mode1)
    {
        m_interrupts.trigger_interrupt(gb_interrupt::lcd, m_clk_next_irq_vblank);
    }

    int clk_current = m_clock.get_clock_cycle();
    m_clk_next_irq_vblank += add_total_frames(m_clk_next_irq_vblank, clk_current);
    int clk_diff = m_clk_next_irq_vblank - clk_current;
    assert(clk_diff > 0);

    m_events.schedule_event(gb_event::lcd_interrupt_vblank, clk_diff);
    m_interrupts.log() << "next v-blank IRQ in " << log_in_clks(clk_diff, clk_current);
}



void age::gb_lcd_irqs::schedule_irq_vblank()
{
    int clk_current     = m_clock.get_clock_cycle();
    int clk_frame_start = m_line.clk_frame_start();
    assert(m_line.lcd_is_on());
    assert(clk_frame_start <= clk_current);

    m_clk_next_irq_vblank = clk_frame_start
                            + gb_screen_height * gb_clock_cycles_per_lcd_line;
    int clk_diff = m_clk_next_irq_vblank - clk_current;
    assert(m_clk_next_irq_vblank > clk_current);

    m_events.schedule_event(gb_event::lcd_interrupt_vblank, clk_diff);
    m_interrupts.log() << "next v-blank IRQ in " << log_in_clks(clk_diff, clk_current);
}



//---------------------------------------------------------
//
//   lyc irq
//
//---------------------------------------------------------

void age::gb_lcd_irqs::on_lyc_change()
{
    if (m_line.lcd_is_on())
    {
        schedule_irq_lyc();
    }
}



void age::gb_lcd_irqs::trigger_irq_lyc()
{
    assert(m_clk_next_irq_lyc != gb_no_clock_cycle);
    assert(m_clk_next_irq_lyc <= m_clock.get_clock_cycle());

    m_interrupts.trigger_interrupt(gb_interrupt::lcd, m_clk_next_irq_lyc);
    m_interrupts.log() << "LYC interrupt for LYC " << log_hex8(m_line.m_lyc)
                       << " occurred" << log_line_clks(m_line, m_clk_next_irq_lyc);

    int clk_current = m_clock.get_clock_cycle();
    m_clk_next_irq_lyc += add_total_frames(m_clk_next_irq_lyc, clk_current);
    int clk_diff = m_clk_next_irq_lyc - clk_current;
    assert(clk_diff > 0);

    m_events.schedule_event(gb_event::lcd_interrupt_lyc, clk_diff);

    m_interrupts.log() << "next LYC IRQ for LYC " << log_hex8(m_line.m_lyc)
                       << " in " << log_in_clks(clk_diff, clk_current);
}



void age::gb_lcd_irqs::schedule_irq_lyc()
{
    if (!(m_stat & gb_stat_irq_ly_match) || (m_line.m_lyc >= gb_lcd_line_count))
    {
        m_clk_next_irq_lyc = gb_no_clock_cycle;
        m_events.remove_event(gb_event::lcd_interrupt_lyc);
        return;
    }

    // schedule next LYC irq
    int clk_frame_start = m_line.clk_frame_start();
    assert(m_line.lcd_is_on());

    int frame_clks = (m_line.m_lyc == 0)
                             ? 153 * gb_clock_cycles_per_lcd_line + 8
                             : m_line.m_lyc * gb_clock_cycles_per_lcd_line;
    m_clk_next_irq_lyc = clk_frame_start + frame_clks;

    // schedule LYC interrupt for this frame or for the next frame?
    int clk_current = m_clock.get_clock_cycle();
    if (m_clk_next_irq_lyc <= clk_current)
    {
        m_clk_next_irq_lyc += gb_clock_cycles_per_lcd_frame;
    }
    int clk_diff = m_clk_next_irq_lyc - clk_current;
    assert(clk_diff > 0);

    m_events.schedule_event(gb_event::lcd_interrupt_lyc, clk_diff);

    m_interrupts.log() << "next LYC IRQ for LYC " << log_hex8(m_line.m_lyc)
                       << " in " << log_in_clks(clk_diff, clk_current);

    // immediate interrupt, if we're still on this line
    //! \todo what's the exact timing?
    auto line      = m_line.current_line();
    int  lyc_limit = gb_clock_cycles_per_lcd_line - (m_device.is_cgb_device() ? 2 : 0);

    if ((m_line.m_lyc == line.m_line) && (line.m_line_clks < lyc_limit))
    {
        m_interrupts.trigger_interrupt(gb_interrupt::lcd, clk_current);
    }
}



//---------------------------------------------------------
//
//   mode 2 irq
//
//---------------------------------------------------------

void age::gb_lcd_irqs::trigger_irq_mode2()
{
    assert(m_clk_next_irq_mode2 != gb_no_clock_cycle);
    assert(m_clk_next_irq_mode2 <= m_clock.get_clock_cycle());

    m_interrupts.trigger_interrupt(gb_interrupt::lcd, m_clk_next_irq_mode2);
    m_interrupts.log() << "mode 2 interrupt occurred" << log_line_clks(m_line, m_clk_next_irq_mode2);

    // handle all the details in schedule_irq_mode2()
    // (irq for specific lines at specific clock cycles)
    schedule_irq_mode2();
}



void age::gb_lcd_irqs::schedule_irq_mode2()
{
    if (!(m_stat & gb_stat_irq_mode2))
    {
        m_clk_next_irq_mode2 = gb_no_clock_cycle;
        m_events.remove_event(gb_event::lcd_interrupt_mode2);
        return;
    }

    auto current_line = m_line.current_line();
    int  line         = current_line.m_line;
    int  m2_line      = line % gb_lcd_line_count; // line during current frame

    assert(m2_line < gb_lcd_line_count);
    assert(current_line.m_line_clks < gb_clock_cycles_per_lcd_line);

    // line and m2_line are supposed to be the line DURING WHICH
    // the irq is triggered.
    // They are NOT supposed to be the line FOR WHICH the irq is triggered.
    //
    // Mode 2 irq is triggered before a line begins.
    // If we're too late, jump to the next line.
    // (not for line 0 though)
    int m2_lead = (m_clock.is_double_speed() || m_line.is_odd_alignment()) ? -2 : -1;

    if (current_line.m_line_clks >= gb_clock_cycles_per_lcd_line + m2_lead)
    {
        ++line;
        ++m2_line;
    }

    // There is a mode-2-irq triggered right before v-blank.
    // After that the next mode-2-irq is triggered right before line 0.
    if (m2_line >= gb_screen_height)
    {
        // skip v-blank and trigger at the end of line 153
        if (m2_line < gb_lcd_line_count)
        {
            int line_add = gb_lcd_line_count - 1 - m2_line;
            line += line_add;
        }
        // else trigger during line 0
    }

    // calculate next mode 2 irq clock cycle
    int clk_current     = m_clock.get_clock_cycle();
    int clk_frame_start = m_line.clk_frame_start();

    m_clk_next_irq_mode2 = clk_frame_start
                           + ((line + 1) * gb_clock_cycles_per_lcd_line)
                           // the mode 2 irq for line 0 is not triggered early
                           + ((line == 153) ? 0 : m2_lead);
    int clk_diff = m_clk_next_irq_mode2 - clk_current;

    assert(m_clk_next_irq_mode2 > clk_current);

    m_events.schedule_event(gb_event::lcd_interrupt_mode2, clk_diff);
    m_interrupts.log() << "next mode 2 IRQ in " << log_in_clks(clk_diff, clk_current);
}



//---------------------------------------------------------
//
//   mode 0 irq
//
//---------------------------------------------------------

void age::gb_lcd_irqs::trigger_irq_mode0(int scx)
{
    assert(m_clk_next_irq_mode0 != gb_no_clock_cycle);
    assert(m_clk_next_irq_mode0 <= m_clock.get_clock_cycle());

    m_interrupts.trigger_interrupt(gb_interrupt::lcd, m_clk_next_irq_mode0);
    m_interrupts.log() << "mode 0 interrupt occurred" << log_line_clks(m_line, m_clk_next_irq_mode0);

    // handle all the details in schedule_irq_mode0()
    // (irq for specific lines at specific clock cycles)
    schedule_irq_mode0(scx);
}



void age::gb_lcd_irqs::schedule_irq_mode0(int scx)
{
    if (!(m_stat & gb_stat_irq_mode0))
    {
        m_clk_next_irq_mode0 = gb_no_clock_cycle;
        m_events.remove_event(gb_event::lcd_interrupt_mode0);
        return;
    }

    auto current_line = m_line.current_line();
    int  line         = current_line.m_line;
    int  m0_line      = line % gb_lcd_line_count; // line during current frame

    assert(m0_line < gb_lcd_line_count);
    assert(current_line.m_line_clks < gb_clock_cycles_per_lcd_line);

    //! \todo Gambatte test rom analysis: enable_display/frame0_m0irq_count_scx{2|3}
    //! \todo write a test rom for this
    int m3_end = 80 + 172 + (scx & 7) + (m_line.is_odd_alignment() && !m_clock.is_double_speed() ? 1 : 0);

    // if we're past the mode 0 irq for this line,
    // continue with the next line
    //! \todo too simple: mode 3 timing also depends on sprites & window
    if (current_line.m_line_clks >= m3_end)
    {
        ++line;
        ++m0_line;
    }

    // skip v-blank
    if (m0_line >= gb_screen_height)
    {
        int line_add = gb_lcd_line_count - m0_line;
        line += line_add;
    }

    // calculate next mode 0 irq clock cycle
    int clk_current     = m_clock.get_clock_cycle();
    int clk_frame_start = m_line.clk_frame_start();

    m_clk_next_irq_mode0 = clk_frame_start
                           + (line * gb_clock_cycles_per_lcd_line)
                           + m3_end;
    int clk_diff = m_clk_next_irq_mode0 - clk_current;

    assert(m_clk_next_irq_mode0 > clk_current);

    m_events.schedule_event(gb_event::lcd_interrupt_mode0, clk_diff);
    m_interrupts.log() << "next mode 0 IRQ in " << log_in_clks(clk_diff, clk_current);
}
