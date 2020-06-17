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

#include <algorithm>

#include <age_debug.hpp>

#include "age_gb_lcd.hpp"

#if 0
#define LOG(x) AGE_GB_CLOCK_LOG(x)
#else
#define LOG(x)
#endif





//---------------------------------------------------------
//
//   ly_counter
//
//---------------------------------------------------------

#define GB_ASSERT_LCD_ON AGE_ASSERT(m_next_scanline_cycle != gb_no_clock_cycle)
#define GB_LY_ASSERT_CONSISTENCY AGE_ASSERT(!m_mode1_ly0 || (m_scanline == 153))



age::gb_ly_counter::gb_ly_counter(const gb_device &device,
                                  const gb_clock &clock,
                                  gb_core &core)
    : m_device(device),
      m_clock(clock),
      m_core(core)
{
    //
    // verified by gambatte tests
    //
    // the initial LY value is 0 for DMG and 144 for CGB
    //
    //      display_startstate/ly_dmg08_out00_cgb_out90
    //
    // when starting execution at PC 0x0100, the vblank
    // interrupt is flagged
    //
    //      lycint_lycirq/lycint_lycirq_1_dmg08_cgb_out1
    //      lycint_lycirq/lycint_lycirq_2_dmg08_cgb_out3
    //      display_startstate/irq_dmg08_cgb_outE1
    //
    if (m_device.is_cgb())
    {
        m_scanline = 144;
        m_next_scanline_cycle = m_clock.get_clock_cycle() + gb_cycles_per_scanline - 164;
    }
    else
    {
        m_scanline = 153;
        m_mode1_ly0 = true;
        m_next_scanline_cycle = m_clock.get_clock_cycle() + 60;
    }

    m_core.request_interrupt(gb_interrupt::vblank);
}



age::uint8_t age::gb_ly_counter::get_ly_port(bool lcd_enabled) const
{
    GB_LY_ASSERT_CONSISTENCY;
    auto result = m_scanline;

    if (lcd_enabled)
    {
        //
        // verified by gambatte tests
        //
        // the value read from 0xFF44 (LY) is 4 cycles "ahead"
        // (when running at double speed, it's only 2 cycles ahead)
        //
        //      ly0/lycint152_ly153_1_dmg08_cgb_out98
        //      ly0/lycint152_ly153_2_dmg08_cgb_out99
        //      ly0/lycint152_ly153_3_dmg08_cgb_out00
        //      ly0/lycint_ly_1_dmg08_cgb_out5
        //      ly0/lycint_ly_2_dmg08_cgb_out6
        //      ly0/lycint_ly_ds_1_out5
        //      ly0/lycint_ly_ds_2_out6
        //
        // while running at double speed, LY 153 is visible for
        // 2 more reads
        //
        //      ly0/lycint152_ly153_ds_1_out98
        //      ly0/lycint152_ly153_ds_2_out99
        //      ly0/lycint152_ly153_ds_3_out99
        //      ly0/lycint152_ly153_ds_4_out99
        //      ly0/lycint152_ly153_ds_5_out00
        //
        int current_cycle = m_clock.get_clock_cycle();
        int next_scanline_offset = get_next_scanline_cycle_offset(current_cycle);
        bool double_speed = m_clock.is_double_speed();

        if (result == 153)
        {
            if (!double_speed || (next_scanline_offset <= gb_cycles_per_scanline - 4))
            {
                result = 0;
            }
        }
        else if (next_scanline_offset <= (double_speed ? 2 : 4))
        {
            ++result;
        }
    }

    return static_cast<uint8_t>(result);
}

age::uint8_t age::gb_ly_counter::get_ly() const
{
    GB_LY_ASSERT_CONSISTENCY;
    return m_mode1_ly0 ? 0 : m_scanline;
}

age::uint8_t age::gb_ly_counter::get_scanline() const
{
    GB_LY_ASSERT_CONSISTENCY;
    return m_scanline;
}

int age::gb_ly_counter::get_next_scanline_cycle_offset(int current_cycle) const
{
    GB_ASSERT_LCD_ON;
    GB_LY_ASSERT_CONSISTENCY;
    AGE_ASSERT(current_cycle <= m_next_scanline_cycle);

    int cycle_offset = m_next_scanline_cycle - current_cycle;
    AGE_ASSERT(cycle_offset >= 0);

    return cycle_offset;
}



void age::gb_ly_counter::switch_off()
{
    GB_ASSERT_LCD_ON;
    GB_LY_ASSERT_CONSISTENCY;

    m_next_scanline_cycle = gb_no_clock_cycle;
    m_scanline = 0;
    m_mode1_ly0 = false;

    LOG("LY set to zero, next line cycle invalidated");
}

void age::gb_ly_counter::switch_on()
{
    AGE_ASSERT(m_next_scanline_cycle == gb_no_clock_cycle);
    AGE_ASSERT(m_scanline == 0);

    m_next_scanline_cycle = m_clock.get_clock_cycle() + gb_cycles_per_scanline;

    LOG("LY counter restarted");
}

void age::gb_ly_counter::next_line()
{
    GB_ASSERT_LCD_ON;
    GB_LY_ASSERT_CONSISTENCY;
    AGE_ASSERT(m_clock.get_clock_cycle() >= m_next_scanline_cycle);

    // LCD operation switching to the first scanline
    if (m_mode1_ly0)
    {
        m_mode1_ly0 = false;
        m_scanline = 0;
    }

    // LCD operation switching to any scanline after the first one
    else
    {
        ++m_scanline;
    }

    m_next_scanline_cycle += gb_cycles_per_scanline;
}

void age::gb_ly_counter::mode1_ly0()
{
    GB_ASSERT_LCD_ON;
    AGE_ASSERT(m_scanline == 153);
    AGE_ASSERT(!m_mode1_ly0);

    m_mode1_ly0 = true;
}



void age::gb_ly_counter::set_back_clock(int clock_cycle_offset)
{
    AGE_GB_SET_BACK_CLOCK(m_next_scanline_cycle, clock_cycle_offset);
}



#undef GB_LY_ASSERT_CONSISTENCY
#undef GB_ASSERT_LCD_ON





//---------------------------------------------------------
//
//   lyc_handler
//
//---------------------------------------------------------

// Assert that the cycle offset to the next LY change is bigger than 0.
// This is true only if all events for the current clock cycle have been processed.
// Thus we can only assert this on port reads & writes.
#define GB_LYC_ASSERT_LY_OFFSET AGE_ASSERT(get_next_scanline_cycle_offset(m_clock.get_clock_cycle()) > 0)



age::uint8_t age::gb_lyc_handler::get_lyc() const
{
    return m_lyc;
}

bool age::gb_lyc_handler::is_interruptable_coincidence() const
{
    return m_stat_coincidence_interrupt && (get_ly() == m_lyc);
}

bool age::gb_lyc_handler::get_stat_coincidence_interrupt() const
{
    return m_stat_coincidence_interrupt;
}

age::uint8_t age::gb_lyc_handler::get_stat_coincidence(bool lcd_enabled) const
{
    bool coincidence;

    if (lcd_enabled)
    {
        GB_LYC_ASSERT_LY_OFFSET;

        //
        // verified by gambatte tests
        //
        // - the coincidence flag is cleared 4 cycles before a scanline is finished
        //   (when running at double speed, the flag is cleared when switching to
        //   a new line)
        //
        //      ly0/lycint152_lyc0flag_3_dmg08_cgb_outC4
        //      ly0/lycint152_lyc0flag_4_dmg08_cgb_outC0
        //      ly0/lycint152_lyc0flag_ds_3_outC4
        //      ly0/lycint152_lyc0flag_ds_4_outC2
        //
        // - since the LYC check for the switch from LY 153 to LY 0 occurs 4 cycles
        //   later than described above
        //
        //      display_startstate/stat_1_dmg08_out85
        //      display_startstate/stat_2_dmg08_out84
        //      ly0/lycint152_lyc0flag_1_dmg08_cgb_outC1
        //      ly0/lycint152_lyc0flag_2_dmg08_cgb_outC5
        //      ly0/lycint152_lyc0flag_ds_1_outC1
        //      ly0/lycint152_lyc0flag_ds_2_outC5
        //

        auto scanline = get_scanline();
        int current_cycle = m_clock.get_clock_cycle();

        // next_scanline_cycles contains the number of cycles left until
        // the next LY switch
        int next_ly_cycles = get_next_scanline_cycle_offset(current_cycle);
        if (get_scanline() == 153)
        {
            if (next_ly_cycles <= gb_cycles_per_scanline - 8)
            {
                next_ly_cycles += gb_cycles_per_scanline;
                scanline = 0;
            }
            else
            {
                next_ly_cycles -= gb_cycles_per_scanline - 8;
            }
        }

        coincidence = next_ly_cycles > (m_clock.is_double_speed() ? 0 : 4);
        coincidence &= (scanline == m_lyc);
        //LOG("LY " << scanline << ", LYC " << AGE_LOG_DEC(m_lyc) << " -> " << coincidence << ", next_ly_cycles " << next_ly_cycles);
    }

    //
    // verified by gambatte tests
    //
    // the coincidence flag is preserved while the LCD is switched off
    //
    //      enable_display/disable_display_regs_1_dmg08_cgb_out66e46666009266666666
    //
    else
    {
        coincidence = m_stat_coincidence_lcd_off;
    }

    return coincidence ? gb_stat_coincidence : 0;
}



void age::gb_lyc_handler::switch_off()
{
    GB_LYC_ASSERT_LY_OFFSET;
    m_stat_coincidence_lcd_off = get_stat_coincidence(true) > 0;
    gb_ly_counter::switch_off();
}

void age::gb_lyc_handler::set_stat(uint8_t value, int mode, bool lcd_enabled)
{
    if (lcd_enabled)
    {
        GB_LYC_ASSERT_LY_OFFSET;
        int current_cycle = m_clock.get_clock_cycle();
        int next_scanline_offset = get_next_scanline_cycle_offset(current_cycle);

        //
        // verified by gambatte tests
        //
        //      lycEnable/lyc153_late_ff41_enable_ds_1_oute2
        //      lycEnable/lyc153_late_ff41_enable_ds_2_oute0
        //
        auto scanline = get_scanline();
        uint8_t ly = get_ly();
        if ((scanline == 153) && m_clock.is_double_speed() && (next_scanline_offset > gb_cycles_per_scanline - 8))
        {
            ly = 153;
        }

        //
        // DMG:
        // The LCD interrupt will be set, if LYC == LY and the LYC interrupt
        // flag was not set before the write (the current LYC interrupt flag
        // has no influence here).
        //
        // CGB:
        // The LCD interrupt will be set, if LYC == LY and the LYC interrupt
        // flag has been changed from 0 to 1 with this write.
        //
        //      miscmstatirq/lycflag_statwirq_1_dmg08_out2
        //      miscmstatirq/lycflag_statwirq_2_dmg08_out2
        //      miscmstatirq/lycflag_statwirq_3_dmg08_out2
        //      miscmstatirq/lycflag_statwirq_4_dmg08_out0
        //      miscmstatirq/lycstatwirq_trigger_00_00_dmg08_out2_cgb_out0
        //      miscmstatirq/lycstatwirq_trigger_00_40_dmg08_out2_cgb_out2
        //      miscmstatirq/lycstatwirq_trigger_00_bf_dmg08_out2_cgb_out0
        //      miscmstatirq/lycstatwirq_trigger_00_ff_dmg08_out2_cgb_out2
        //      miscmstatirq/lycstatwirq_trigger_40_00_dmg08_cgb_out0
        //      miscmstatirq/lycstatwirq_trigger_40_40_dmg08_cgb_out0
        //      miscmstatirq/lycstatwirq_trigger_40_bf_dmg08_cgb_out0
        //      miscmstatirq/lycstatwirq_trigger_40_ff_dmg08_cgb_out0
        //      miscmstatirq/lycstatwirq_trigger_bf_00_dmg08_out2_cgb_out0
        //      miscmstatirq/lycstatwirq_trigger_bf_40_dmg08_out2_cgb_out2
        //      miscmstatirq/lycstatwirq_trigger_bf_bf_dmg08_out2_cgb_out0
        //      miscmstatirq/lycstatwirq_trigger_bf_ff_dmg08_out2_cgb_out2
        //      miscmstatirq/lycstatwirq_trigger_ff_00_dmg08_cgb_out0
        //      miscmstatirq/lycstatwirq_trigger_ff_40_dmg08_cgb_out0
        //      miscmstatirq/lycstatwirq_trigger_ff_bf_dmg08_cgb_out0
        //      miscmstatirq/lycstatwirq_trigger_ff_ff_dmg08_cgb_out0
        //
        bool raise_lcd_interrupt = (ly == m_lyc)
                && (m_device.is_cgb() ? ((value & gb_stat_interrupt_coincidence) > 0) && !m_stat_coincidence_interrupt
                                      : !m_stat_coincidence_interrupt);
        //
        // If the above conditions are met but a vblank interrupt was possible
        // the the time of writing, no LCD interrupt is triggered.
        //
        //      miscmstatirq/lycstatwirq_trigger_ly00_10_50_1_dmg08_cgb_outE0
        //      miscmstatirq/lycstatwirq_trigger_ly00_10_50_2_dmg08_cgb_outE2
        //      miscmstatirq/lycstatwirq_trigger_ly00_10_50_ds_1_outE0
        //      miscmstatirq/lycstatwirq_trigger_ly00_10_50_ds_2_outE2
        //      miscmstatirq/lycstatwirq_trigger_ly94_00_50_dmg08_cgb_outE2
        //      miscmstatirq/lycstatwirq_trigger_ly94_10_40_dmg08_cgb_outE0
        //      miscmstatirq/lycstatwirq_trigger_ly94_10_50_dmg08_cgb_outE0
        //
        if (scanline >= gb_screen_height)
        {
            raise_lcd_interrupt &= !m_stat_mode1_interrupt;
        }
        //
        // If the above conditions are met but a mode 0 interrupt was possible
        // at the time of writing, no LCD interrupt is triggered (only if the
        // next scanline is more than 2 cycles away though).
        //
        //      miscmstatirq/lycstatwirq_trigger_m0_ly44_lyc44_00_40_dmg08_cgb_outE2
        //      miscmstatirq/lycstatwirq_trigger_m0_ly44_lyc44_00_48_dmg08_cgb_outE2
        //      miscmstatirq/lycstatwirq_trigger_m0_ly44_lyc44_08_40_dmg08_cgb_outE0
        //      miscmstatirq/lycstatwirq_trigger_m0_ly44_lyc44_08_48_dmg08_cgb_outE0
        //      miscmstatirq/lycstatwirq_trigger_m0_ly44_lyc44_08_ff_dmg08_cgb_outE0
        //      miscmstatirq/lycstatwirq_trigger_m0_ly44_lyc44_b7_40_dmg08_cgb_outE2
        //      miscmstatirq/lycstatwirq_trigger_m0_ly44_lyc44_b7_f7_dmg08_cgb_outE2
        //      miscmstatirq/lycstatwirq_trigger_m0_ly44_lyc44_bf_40_dmg08_cgb_outE0
        //      miscmstatirq/lycstatwirq_trigger_m0_ly44_lyc44_bf_ff_dmg08_cgb_outE0
        //      miscmstatirq/lycstatwirq_trigger_m0_late_ly44_lyc44_08_40_ds_1_oute0
        //      miscmstatirq/lycstatwirq_trigger_m0_late_ly44_lyc44_08_40_ds_2_oute0
        //      miscmstatirq/lycstatwirq_trigger_m0_late_ly44_lyc44_08_40_ds_3_oute2
        //      miscmstatirq/lycstatwirq_trigger_m0_late_ly44_lyc44_08_40_ds_4_oute0
        //
        else if ((mode == 0) && (next_scanline_offset > 2))
        {
            raise_lcd_interrupt &= !m_stat_mode0_interrupt;
        }
        //
        // On a CGB the LYC interrupt is only triggered, if we're more than
        // 4 cycles away from the next LY (not when running at double speed
        // though).
        //
        //      lycEnable/late_ff41_enable_1_dmg08_cgb_out2
        //      lycEnable/late_ff41_enable_2_dmg08_out2_cgb_out0
        //      lycEnable/late_ff41_enable_3_dmg08_cgb_out0
        //      lycEnable/late_ff41_enable_ds_1_out3
        //      lycEnable/late_ff41_enable_ds_2_out1
        //
        raise_lcd_interrupt &= !m_device.is_cgb() || m_clock.is_double_speed() || (next_scanline_offset > 4);

        LOG((raise_lcd_interrupt ? "" : "NOT ") << "triggering LYC interrupt, next_scanline_offset " << next_scanline_offset);
        if (raise_lcd_interrupt)
        {
            m_core.request_interrupt(gb_interrupt::lcd);
        }
    }

    m_stat_coincidence_interrupt = (value & gb_stat_interrupt_coincidence) > 0;
    m_stat_mode0_interrupt = (value & gb_stat_interrupt_mode0) > 0;
    m_stat_mode1_interrupt = (value & gb_stat_interrupt_mode1) > 0;
    m_stat_mode2_interrupt = (value & gb_stat_interrupt_mode2) > 0;
}

void age::gb_lyc_handler::set_lyc(uint8_t value, int mode, bool lcd_enabled)
{
    LOG(AGE_LOG_DEC(value) << ", current LY is " << AGE_LOG_DEC(get_ly()));
    m_lyc = value;

    if (lcd_enabled && m_stat_coincidence_interrupt)
    {
        int current_cycle = m_clock.get_clock_cycle();
        int next_scanline_offset = get_next_scanline_cycle_offset(current_cycle);
        auto scanline = get_scanline();
        int min_scanline_offset = m_device.is_cgb() && !m_clock.is_double_speed() ? 8 : 4;

        //
        // verified by gambatte tests
        //
        // An LCD interrupt may be immediately triggered, if the LCD is active,
        // the LYC interrupt is allowed and LY equals the written LYC value.
        //
        //      miscmstatirq/lycwirq_trigger_m0_late_ly44_lyc45_5_dmg08_cgb_outE2
        //      miscmstatirq/lycwirq_trigger_m0_late_ly44_lyc45_ds_4_outE2
        //      lycEnable/ff45_reenable_1_dmg08_cgb_out3
        //      lycEnable/ff45_reenable_2_dmg08_cgb_out2
        //
        bool raise_lcd_interrupt = get_ly() == m_lyc;
        //
        // The LYC interrupt is not triggered during mode 1, if
        // the STAT mode 1 interrupt flag is set.
        //
        //      lycEnable/lycwirq_trigger_ly94_stat50_dmg08_cgb_outE0
        //
        if (scanline >= gb_screen_height)
        {
            raise_lcd_interrupt &= !m_stat_mode1_interrupt;
        }
        //
        // When writing the LYC, an immediate LYC interrupt will be
        // blocked by a possible mode 0 interrupt.
        //
        //      miscmstatirq/lycwirq_trigger_m0_early_ly44_8_dmg08_cgb_outE2
        //      miscmstatirq/lycwirq_trigger_m0_early_ly44_9_dmg08_cgb_outE0
        //
        else if (mode == 0)
        {
            raise_lcd_interrupt &= !m_stat_mode0_interrupt;
        }
        //
        // The LYC interrupt may only be raised when the next scanline
        // is more than 8 cycles (CGB) or 4 cycles (DMG, CGB at
        // double speed) away.
        //
        //      lycEnable/late_ff45_enable_1_dmg08_cgb_out3
        //      lycEnable/late_ff45_enable_2_dmg08_out3_cgb_out1
        //      lycEnable/late_ff45_enable_3_dmg08_cgb_out1
        //
        raise_lcd_interrupt &= next_scanline_offset > min_scanline_offset;
        //
        // Special handling for scanline 153:
        //
        // the mode 1 interrupt flag is considered only until
        // 4 cycles before scanline 0, if running on a CGB
        // not at double speed.
        //
        //      lycEnable/lycwirq_trigger_ly00_stat50_1_dmg08_cgb_outE0
        //      lycEnable/lycwirq_trigger_ly00_stat50_2_dmg08_outE0_cgb_outE2
        //      lycEnable/lycwirq_trigger_ly00_stat50_3_dmg08_cgb_outE2
        //      lycEnable/lycwirq_trigger_ly00_stat50_ds_1_outE0
        //      lycEnable/lycwirq_trigger_ly00_stat50_ds_2_outE2
        //
        if (scanline == 153)
        {
            if (next_scanline_offset <= min_scanline_offset - 4)
            {
                raise_lcd_interrupt = m_lyc == 0;
            }
            //
            // No immediate interrupt will be triggered when setting
            // LYC to 153 the same time that scanline was entered
            // on a CGB not running at double speed.
            //
            //      lycEnable/lyc153_late_ff45_enable_4_dmg08_outE2_cgb_outE0
            //      lycEnable/lyc153_late_ff45_enable_ds_4_oute2
            //
            else if (m_device.is_cgb() && !m_clock.is_double_speed())
            {
                raise_lcd_interrupt &= next_scanline_offset < gb_cycles_per_scanline;
            }
        }

        LOG((raise_lcd_interrupt ? "" : "NOT ") << "triggering LYC interrupt, next_scanline_offset " << next_scanline_offset);
        if (raise_lcd_interrupt)
        {
            //
            // The interrupt is delayed by at least 5 cycles on a CGB
            // when not running at double speed.
            //
            //      lycEnable/lyc_ff45_trigger_delay_1_dmg08_cgb_out0
            //      lycEnable/lyc_ff45_trigger_delay_2_dmg08_out0_cgb_out2
            //      lycEnable/lyc_ff45_trigger_delay_3_dmg08_cgb_out2
            //      lycEnable/lyc_ff45_trigger_delay_ds_1_out0
            //      lycEnable/lyc_ff45_trigger_delay_ds_2_out2
            //
            if (m_device.is_cgb() && !m_clock.is_double_speed())
            {
                LOG("delaying immediate LCD interrupt");
                m_core.insert_event(5, gb_event::lcd_late_lyc_interrupt);
            }
            else
            {
                m_core.request_interrupt(gb_interrupt::lcd);
            }
        }
    }
}



bool age::gb_lyc_handler::get_stat_mode2_interrupt() const
{
    return m_stat_mode2_interrupt;
}

bool age::gb_lyc_handler::get_stat_mode1_interrupt() const
{
    return m_stat_mode1_interrupt;
}



#undef GB_LYC_ASSERT_LY_OFFSET





//---------------------------------------------------------
//
//   lyc_interrupter
//
//---------------------------------------------------------

#define GB_LYC_INT_ASSERT_LCD_ON AGE_ASSERT(get_next_scanline_cycle_offset(m_clock.get_clock_cycle()) <= gb_cycles_per_scanline)



void age::gb_lyc_interrupter::lyc_event()
{
    GB_LYC_INT_ASSERT_LCD_ON;

    //
    // verified by gambatte tests
    //
    // the LYC interrupt is flagged only, if no mode2/mode1 interrupts are possible
    //
    //      lcdirq_precedence/lcdirqprecedence_lycirq_ly44_lcdstat58_dmg08_cgb_out2
    //      lcdirq_precedence/lcdirqprecedence_lycirq_ly44_lcdstat68_dmg08_cgb_out0
    //      lcdirq_precedence/lycirq_ly91_lcdstat50_dmg08_cgb_out0
    //      lcdirq_precedence/lycirq_ly99_lcdstat50_dmg08_cgb_out0
    //

    // check conditions for interrupt, since this event can be executed
    // any time, sometimes just to set a new m_lyc_int value
    uint8_t ly = get_ly();
    uint8_t int_lyc = ly - 1; // ly == 0 -> ly = 255   ly == 144 -> ly = 143   => use vblank flag for ly == 0
    //
    // There is no delay for enabling the STAT coincidence
    // interrupt.
    //
    //      lycEnable/lyc_ff41_enable_1_dmg08_cgb_out2
    //      lycEnable/lyc_ff41_enable_2_dmg08_cgb_out2
    //      lycEnable/lyc_ff41_enable_3_dmg08_cgb_out2
    //      lycEnable/lyc_ff41_enable_4_dmg08_cgb_out2
    //      lycEnable/lyc_ff41_enable_5_dmg08_cgb_out2
    //
    bool stat_coincidence_interrupt = m_stat_coincidence_interrupt_int || get_stat_coincidence_interrupt();

    bool interrupt = stat_coincidence_interrupt
            && (ly == m_lyc_int)
            && ((int_lyc < (gb_screen_height - 1)) ? !m_stat_mode2_interrupt_int : !m_stat_mode1_interrupt_int);

    if (interrupt)
    {
        m_core.request_interrupt(gb_interrupt::lcd);
    }

    // switch to new value, if necessary
    m_stat_coincidence_interrupt_int = get_stat_coincidence_interrupt();
    m_stat_mode2_interrupt_int = get_stat_mode2_interrupt();
    m_stat_mode1_interrupt_int = get_stat_mode1_interrupt();
    m_lyc_int = get_lyc();

    // schedule next event, if required
    int next_event_cycle = calculate_next_event_cycle(m_stat_coincidence_interrupt_int, m_lyc_int);
    schedule_next_event(next_event_cycle);
}

void age::gb_lyc_interrupter::switch_off()
{
    gb_lyc_handler::switch_off();
    schedule_next_event(int_max);
}

void age::gb_lyc_interrupter::switch_on()
{
    gb_lyc_handler::switch_on();

    //
    // verified by gambatte tests
    //
    // when switching on the LCD an LCD interrupt will be generated,
    // if LYC is 0 and the STAT LYC interrupt flag is set
    //
    //      enable_display/lcdcenable_lyc0irq_1_dmg08_cgb_out2
    //
    m_stat_coincidence_interrupt_int = get_stat_coincidence_interrupt();
    m_stat_mode2_interrupt_int = get_stat_mode2_interrupt();
    m_stat_mode1_interrupt_int = get_stat_mode1_interrupt();
    m_lyc_int = get_lyc();
    lyc_event(); // sets m_next_event_cycle
}



void age::gb_lyc_interrupter::set_stat(uint8_t value, int mode, bool lcd_enabled)
{
//    bool old_stat_coincidence_interrupt = get_stat_coincidence_interrupt();
    gb_lyc_handler::set_stat(value, mode, lcd_enabled);

    // LCD inactive -> just copy the new value
    if (!lcd_enabled)
    {
        m_stat_coincidence_interrupt_int = get_stat_coincidence_interrupt();
        m_stat_mode2_interrupt_int = get_stat_mode2_interrupt();
        m_stat_mode1_interrupt_int = get_stat_mode1_interrupt();
    }

    // LCD active -> we may have to delay copying the new value
    else
    {
        GB_LYC_INT_ASSERT_LCD_ON;

        // next_event_cycle != m_next_event_cycle, if the LYC interrupt was disabled
        int next_event_cycle = calculate_next_event_cycle(get_stat_coincidence_interrupt(), m_lyc_int);
        int new_next_event_cycle = std::min(next_event_cycle, m_next_event_cycle);

        // we need that event at least to copy the new value
        schedule_next_event(new_next_event_cycle);

        int current_cycle = m_clock.get_clock_cycle();
        int next_event_offset = m_next_event_cycle - current_cycle;
        AGE_ASSERT(m_next_event_cycle > current_cycle);
        AGE_ASSERT(next_event_offset > 0);

        bool delayed = next_event_offset <= 4;
        if (m_device.is_cgb())
        {
            //
            // verified by gambatte tests
            //
            // The STAT change does not affect the next LYC interrupt on a CGB
            // when not running at double speed, the LYC interrupt is only
            // 4 cycles away and:
            //
            //  - trying to disable the LYC interrupt
            //
            //      lycEnable/ff41_disable_1_dmg08_cgb_out0
            //      lycEnable/ff41_disable_2_dmg08_out0_cgb_out2
            //      lycEnable/ff41_disable_3_dmg08_cgb_out2
            //      lycEnable/ff41_disable_ds_1_out1
            //      lycEnable/ff41_disable_ds_2_out3
            //
            //  - changing any influencing interrupt flags (like mode 1)
            //
            //      lycEnable/lyc0_m1disable_1_dmg08_cgb_outE2
            //      lycEnable/lyc0_m1disable_2_dmg08_outE2_cgb_outE0
            //      lycEnable/lyc0_m1disable_3_dmg08_cgb_outE0
            //      lycEnable/lyc0_m1disable_ds_1_outE2
            //      lycEnable/lyc0_m1disable_ds_2_outE0
            //
            delayed &= !m_clock.is_double_speed();
        }
        else
        {
            //
            // verified by gambatte tests
            //
            // The STAT change does not affect the next LYC interrupt on a DMG
            // when the LYC interrupt is only 4 cycles away and:
            //
            //  - trying to disable the LYC interrupt for LYC 0
            //
            //      lycEnable/lyc0_ff41_disable_1_dmg08_outE2_cgb_outE0
            //      lycEnable/lyc0_ff41_disable_2_dmg08_cgb_outE2
            //
            delayed &= m_lyc_int == 0;
        }

        LOG("LYC STAT change delayed: " << delayed << ", next_event_offset " << next_event_offset);
        if (!delayed)
        {
            m_stat_coincidence_interrupt_int = get_stat_coincidence_interrupt();
        }
        if (!delayed || !m_device.is_cgb())
        {
            //
            // verified by gambatte tests
            //
            // Changing the STAT interrupt flags on a DMG for mode 1
            // is not delayed (I guess it's the same for the mode 2 flag,
            // but I did not find any test for that yet).
            //
            //      lycEnable/lyc0_m1disable_2_dmg08_outE2_cgb_outE0
            //
            m_stat_mode1_interrupt_int = get_stat_mode1_interrupt();
            m_stat_mode2_interrupt_int = get_stat_mode2_interrupt();
        }
    }
}

void age::gb_lyc_interrupter::set_lyc(uint8_t value, int mode, bool lcd_enabled)
{
    uint8_t old_lyc = get_lyc();
    gb_lyc_handler::set_lyc(value, mode, lcd_enabled);

    // LCD inactive -> just copy the new value
    if (!lcd_enabled)
    {
        m_lyc_int = get_lyc();
    }

    // LCD active -> we may have to delay copying the new value
    else
    {
        GB_LYC_INT_ASSERT_LCD_ON;

        //
        // next_event_cycle != m_next_event_cycle, if
        //  - the LYC interrupt was disabled
        //  - new-LYC > LYC > LY
        //
        int next_event_cycle = calculate_next_event_cycle(m_stat_coincidence_interrupt_int, get_lyc());
        int new_next_event_cycle = std::min(next_event_cycle, m_next_event_cycle);

        // we need that event at least to copy the new value
        schedule_next_event(new_next_event_cycle);

        int current_cycle = m_clock.get_clock_cycle();
        int next_event_offset = m_next_event_cycle - current_cycle;
        AGE_ASSERT(m_next_event_cycle > current_cycle);
        AGE_ASSERT(next_event_offset > 0);

        //
        // verified by gambatte tests
        //
        // the LYC change does not affect the next LYC interrupt:
        //
        //  - when trying to disable/delay the LYC interrupt on a CGB, not running
        //    at double speed and the LYC interrupt is only 4 cycles away.
        //
        //      lycEnable/ff45_disable_1_dmg08_cgb_out1
        //      lycEnable/ff45_disable_2_dmg08_out1_cgb_out3
        //      lycEnable/ff45_disable_3_dmg08_cgb_out3
        //      lycEnable/ff45_disable_ds_1_out1
        //      lycEnable/ff45_disable_ds_2_out3
        //
        bool delayed = false;
        if (m_next_event_cycle != next_event_cycle)
        {
            delayed = m_device.is_cgb() && !m_clock.is_double_speed() && (next_event_offset <= 4);
        }
        //
        //  - when trying to move the LYC interrupt to a scanline LY+1 that
        //    is 5-8 cycles (CGB) or 1-4 cycles (DMG) away.
        //
        //      lycEnable/ff45_enable_weirdpoint_1_dmg08_cgb_out3
        //      lycEnable/ff45_enable_weirdpoint_2_dmg08_out3_cgb_out1
        //      lycEnable/ff45_enable_weirdpoint_3_dmg08_out1_cgb_out3
        //      lycEnable/ff45_enable_weirdpoint_4_dmg08_cgb_out3
        //
        //    this does not apply for LYC 0
        //
        //      lycEnable/lyc0_ff45_enable_weirdpoint_1_dmg08_cgb_oute2
        //      lycEnable/lyc0_ff45_enable_weirdpoint_2_dmg08_cgb_oute2
        //      lycEnable/lyc0_ff45_enable_weirdpoint_3_dmg08_cgb_oute2
        //      lycEnable/lyc0_ff45_enable_weirdpoint_4_dmg08_cgb_oute2
        //
        //    this does not apply when the old LYC was not equal to
        //    the current LY
        //
        //      miscmstatirq/lycwirq_trigger_m0_late_ly44_lyc45_3_dmg08_cgb_outE2
        //      miscmstatirq/lycwirq_trigger_m0_late_ly44_lyc45_4_dmg08_cgb_outE2
        //      miscmstatirq/lycwirq_trigger_m0_late_ly44_lyc45_ds_2_outE2
        //      miscmstatirq/lycwirq_trigger_m0_late_ly44_lyc45_ds_3_outE2
        //
        else if ((value != 0) && (old_lyc == get_ly()))
        {
            int offset = (m_device.is_cgb() && !m_clock.is_double_speed()) ? 8 : 4;
            delayed = (next_event_offset <= offset) && (next_event_offset > offset - 4);
        }

        LOG("LYC change delayed: " << delayed << ", next_event_offset " << next_event_offset);
        if (!delayed)
        {
            m_lyc_int = get_lyc();
        }
    }
}



void age::gb_lyc_interrupter::set_back_clock(int clock_cycle_offset)
{
    gb_ly_counter::set_back_clock(clock_cycle_offset);
    if (m_next_event_cycle != int_max)
    {
        AGE_GB_SET_BACK_CLOCK(m_next_event_cycle, clock_cycle_offset);
    }
}



int age::gb_lyc_interrupter::calculate_next_event_cycle(bool stat_coincidence_interrupt, uint8_t for_lyc)
{
    int result = int_max;

    if (stat_coincidence_interrupt && (for_lyc < 154))
    {
        int current_cycle = m_clock.get_clock_cycle();
        int next_scanline_cycle_offset = get_next_scanline_cycle_offset(current_cycle);
        int remaining_lines = (154 - 1 - get_scanline()) * gb_cycles_per_scanline;
        int next_frame = current_cycle + next_scanline_cycle_offset + remaining_lines;

        //
        // verified by gambatte tests
        //
        // the coincidence interrupt for LY 0 is triggered during the end of mode 1,
        // 4 cycles after changing LY to 0
        //
        //      ly0/lycint152_lyc0irq_1_dmg08_cgb_outE0
        //      ly0/lycint152_lyc0irq_2_dmg08_cgb_outE2
        //
        int lyc_offset = (for_lyc == 0) ? 153 * gb_cycles_per_scanline + 8 : for_lyc * gb_cycles_per_scanline;
        result = next_frame + lyc_offset;

        if (result > current_cycle + gb_cycles_per_frame)
        {
            result -= gb_cycles_per_frame;
        }
        AGE_ASSERT((result > current_cycle) && (result - current_cycle <= gb_cycles_per_frame));
    }
    return result;
}

void age::gb_lyc_interrupter::schedule_next_event(int next_event_cycle)
{
    m_next_event_cycle = next_event_cycle;

    if (m_next_event_cycle != int_max)
    {
        int current_cycle = m_clock.get_clock_cycle();
        AGE_ASSERT(current_cycle < m_next_event_cycle);
        int cycle_offset = m_next_event_cycle - current_cycle;
        m_core.insert_event(cycle_offset, gb_event::lcd_lyc_check);
    }
    else
    {
        m_core.remove_event(gb_event::lcd_lyc_check);
    }
}



#undef GB_LYC_INT_ASSERT_LCD_ON
