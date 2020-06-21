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

#include <age_debug.hpp>

#include "age_gb_lcd.hpp"

#if 0
#define LOG(x) AGE_GB_CLOCK_LOG("ly " << AGE_LOG_DEC(get_ly()) << ", " << x)
#else
#define LOG(x)
#endif



namespace
{

constexpr unsigned gb_palette_bgp = 0x00;
constexpr unsigned gb_palette_obp0 = 0x20;
constexpr unsigned gb_palette_obp1 = 0x30;

}




//---------------------------------------------------------
//
//   lcdc
//
//---------------------------------------------------------

void age::gb_lcd::write_lcdc(uint8_t value)
{
    LOG(AGE_LOG_HEX(value));
    uint8_t old_lcdc = read_lcdc();

    // LCD switched on or off
    if (((value ^ old_lcdc) & gb_lcdc_enable) > 0)
    {
        bool lcd_enabled = (value & gb_lcdc_enable) > 0;

        if (lcd_enabled)
        {
            switch_on();
            m_next_event_cycle = m_clock.get_clock_cycle();
            m_last_cycle_m3_finished = 0;
            m_skip_next_mode1_interrupt = false;

            mode0_start_lcd_enabled();
            AGE_ASSERT(get_scanline() == 0);
        }
        else
        {
            switch_off();
            //
            // verified by gambatte tests
            //
            // stat is set to 0 when disabling the LCD
            //
            //      enable_display/disable_display_regs_1_dmg08_cgb_out66e46666009266666666
            //      enable_display/disable_display_regs_2_dmg08_cgb_out66e06666006666666666
            //
            m_stat &= ~gb_stat_modes;

            m_next_event_cycle = gb_no_clock_cycle;
            m_next_event = nullptr;

            pixel_vector &back_buffer = m_screen_buffer.get_back_buffer();
            white_screen(back_buffer);
            switch_frames();

            //
            // verified by gambatte tests
            //
            // Switching off the LCD will trigger the HDMA transfer,
            // if it's active.
            //
            //      dma/hdma_disable_display_1_out1
            //
            if (m_hdma_active)
            {
                m_events.schedule_event(gb_event::start_hdma, 0);
            }
        }

        LOG("lcd_enabled changed from " << m_lcd_enabled << " to " << lcd_enabled);
        m_lcd_enabled = lcd_enabled;
    }

    // value changed while LCD kept active
    else if (m_lcd_enabled)
    {
        //
        // verified by gambatte tests
        //
        // On a CGB writes to LCDC are delayed by 1 cycle while
        // the LCD is active.
        //
        //      bgtilemap/bgtilemap_spx09_1
        //      bgtilemap/bgtilemap_spx09_2
        //      bgtilemap/bgtilemap_spx09_3
        //      bgtilemap/bgtilemap_spx09_4
        //
        // Writing the bit 0x10 (background & window character
        // data selection) is never delayed.
        //
        //      bgtiledata/bgtiledata_spx08_1
        //      bgtiledata/bgtiledata_spx08_2
        //      bgtiledata/bgtiledata_spx08_3
        //      bgtiledata/bgtiledata_spx08_4
        //
        // Writing the bit 0x20 (window enable) is never delayed.
        //
        //      window/late_disable_early_scx03_wx0f_1_dmg08_cgb_out0
        //      window/late_disable_early_scx03_wx0f_2_dmg08_cgb_out3
        //
        if (m_device.is_cgb())
        {
            constexpr uint8_t not_delayed_bits = gb_lcdc_bg_win_data | gb_lcdc_win_enable;
            uint8_t not_delayed = (old_lcdc & ~not_delayed_bits) | (value & not_delayed_bits);
            gb_lcd_ppu::write_lcdc(not_delayed);

            emulate(m_clock.get_clock_cycle() + 1);
        }
    }

    gb_lcd_ppu::write_lcdc(value);
}





//---------------------------------------------------------
//
//   stat
//
//---------------------------------------------------------

age::uint8_t age::gb_lcd::read_stat() const
{
    uint8_t result = m_stat | get_stat_coincidence(m_lcd_enabled) | 0x80;
    LOG(AGE_LOG_HEX(result));
    return result;
}

void age::gb_lcd::write_stat(uint8_t value)
{
    LOG(AGE_LOG_HEX(value));

    auto scanline = get_scanline();
    int mode = (scanline >= gb_screen_height)
            ? 1 // the 4-cycle mode 0 at the end of mode 1 is not recognized here
            : (m_stat & gb_stat_modes);

    // store new STAT value
    uint8_t old_stat = m_stat;
    m_stat &= gb_stat_modes;
    m_stat |= value & ~(gb_stat_coincidence | gb_stat_modes);

    m_stat_mode0_int = m_stat;
    //
    // verified by gambatte tests
    //
    // the mode1 LCD interrupts is flagged only, if no mode0 interrupt is possible
    //
    //      lcdirq_precedence/m1irq_lcdstat18_dmg08_cgb_out1
    //      lcdirq_precedence/m1irq_lcdstat30_dmg08_cgb_out3
    //
    m_allow_mode1_interrupt = (m_stat & (gb_stat_interrupt_mode1 | gb_stat_interrupt_mode0)) == gb_stat_interrupt_mode1;
    //
    // trigger LCD mode 2 interrupt, if
    //  - mode 0 interrupt is not allowed (LY > 0)
    //  - mode 1 interrupt is not allowed (LY == 0)
    //
    //      lcdirq_precedence/m2irq_ly8f_lcdstat28_dmg08_cgb_out0
    //
    m_allow_mode2_interrupt = (m_stat & (gb_stat_interrupt_mode2 | gb_stat_interrupt_mode0)) == gb_stat_interrupt_mode2;
    m_allow_mode2_ly0_interrupt = (m_stat & (gb_stat_interrupt_mode2 | gb_stat_interrupt_mode1)) == gb_stat_interrupt_mode2;
    m_allow_mode2_ly0_interrupt_int = m_allow_mode2_ly0_interrupt;

    if (m_lcd_enabled)
    {
        uint8_t bits_cleared = old_stat & ~value;
        uint8_t bits_set = ~old_stat & value;
        LOG("cleared bits " << AGE_LOG_HEX(bits_cleared) << ", set bits " << AGE_LOG_HEX(bits_set));

        int current_cycle = m_clock.get_clock_cycle();
        int next_scanline_offset = get_next_scanline_cycle_offset(current_cycle);

        //
        // verified by gambatte tests
        //
        //
        // The mode 2 interrupt for LY 0 cannot be disabled by setting
        // the STAT mode 1 interrupt flag, if we're 4 cycles or less
        // away from that mode 2 interrupt.
        //
        //      m2enable/disable_by_m1enable_ly0_1_dmg08_cgb_out2
        //      m2enable/disable_by_m1enable_ly0_2_dmg08_cgb_out2
        //
        // Disabling the mode 2 interrupt by clearing the STAT mode 2
        // interrupt flag 4 cycles before it is raised for LY 0 is
        // possible (during the test for DMG the LCD interrupt will
        // be raised for mode 1, causing different results for DMG
        // and CGB).
        //
        //      m2enable/disable_ly0_1_dmg08_out2_cgb_out0
        //      m2enable/disable_ly0_2_dmg08_cgb_out2
        //
        // When adjusting the STAT mode 1 and mode 2 interrupt flags
        // 4 cycles (or less) before the mode 2 interrupt for LY 0 is
        // triggered, the newly written mode 2 interrupt flag is used
        // for both CGB and DMG. On a CGB however, the old mode 1
        // interrupt flag is taken into account while a DMG will use
        // the new mode 1 interrupt flag.
        //
        //      m2enable/late_enable_m1disable_ly0_1_dmg08_cgb_out2
        //      m2enable/late_enable_m1disable_ly0_2_dmg08_out2_cgb_out0
        //      m2enable/late_enable_m1disable_ly0_3_dmg08_cgb_out0
        //      m2enable/late_m1disable_ly0_1_dmg08_cgb_out2
        //      m2enable/late_m1disable_ly0_2_dmg08_out2_cgb_out0
        //      m2enable/late_m1disable_ly0_3_dmg08_cgb_out0
        //
        // The above rule does not apply when running at double speed.
        //
        //      m2enable/m2_late_m1disable_ly0_ds_1_out2
        //      m2enable/m2_late_m1disable_ly0_ds_2_out0
        //
        if ((scanline == 153) && (next_scanline_offset <= 4))
        {
            bool allow_mode1_interrupt = (((m_device.is_cgb() && !m_clock.is_double_speed()) ? old_stat : m_stat) & gb_stat_interrupt_mode1) > 0;
            m_allow_mode2_ly0_interrupt_int = ((m_stat & gb_stat_interrupt_mode2) > 0) && !allow_mode1_interrupt;
        }

        bool lyc = is_interruptable_coincidence();
        bool raise_lcd_interrupt = false;

        int mode0_interrupt_offset = get_mode0_interrupt_cycle_offset();
        LOG("mode0_interrupt_offset " << mode0_interrupt_offset);

        LOG("mode " << AGE_LOG_HEX(mode));
        switch (mode)
        {
            case 0:
            {
                //
                // verified by gambatte tests
                //
                // DMG:
                // During mode 0, an LCD interrupt will be triggered regardless of
                // the currently written value, if the mode 0 STAT interrupt flag
                // was not set before.
                //
                // CGB:
                // During mode 0, an LCD interrupt will be triggered, if the STAT
                // mode 0 interrupt flag was changed from 0 to 1 with this write.
                //
                //      miscmstatirq/m0statwirq_1_dmg08_out2   // end of mode 0
                //      miscmstatirq/m0statwirq_2_dmg08_out0   // right after mode 0
                //      miscmstatirq/m0statwirq_3_dmg08_out0   // right before mode 0
                //      miscmstatirq/m0statwirq_4_dmg08_out2   // beginning of mode 0, currently fails because of mode3 timing issues
                //      miscmstatirq/m0statwirq_trigger_00_00_dmg08_out2_cgb_out0
                //      miscmstatirq/m0statwirq_trigger_00_08_dmg08_out2_cgb_out2
                //      miscmstatirq/m0statwirq_trigger_00_f7_dmg08_out2_cgb_out0
                //      miscmstatirq/m0statwirq_trigger_00_ff_dmg08_out2_cgb_out2
                //      miscmstatirq/m0statwirq_trigger_08_00_dmg08_cgb_out0
                //      miscmstatirq/m0statwirq_trigger_08_08_dmg08_cgb_out0
                //      miscmstatirq/m0statwirq_trigger_08_f7_dmg08_cgb_out0
                //      miscmstatirq/m0statwirq_trigger_08_ff_dmg08_cgb_out0
                //      miscmstatirq/m0statwirq_trigger_f7_00_dmg08_out2_cgb_out0
                //      miscmstatirq/m0statwirq_trigger_f7_08_dmg08_out2_cgb_out2
                //      miscmstatirq/m0statwirq_trigger_f7_f7_dmg08_out2_cgb_out0
                //      miscmstatirq/m0statwirq_trigger_f7_ff_dmg08_out2_cgb_out2
                //      miscmstatirq/m0statwirq_trigger_ff_00_dmg08_cgb_out0
                //      miscmstatirq/m0statwirq_trigger_ff_08_dmg08_cgb_out0
                //      miscmstatirq/m0statwirq_trigger_ff_f7_dmg08_cgb_out0
                //      miscmstatirq/m0statwirq_trigger_ff_ff_dmg08_cgb_out0
                //
                raise_lcd_interrupt = m_device.is_cgb() ? ((bits_set & gb_stat_interrupt_mode0) > 0) : ((old_stat & gb_stat_interrupt_mode0) == 0);
                //
                // The LCD interrupt will not be triggered for mode 0
                // replacing mode 2 (LY 0) after enabling the LCD.
                //
                //      enable_display/enable_display_ly0_m0irq_trigger_dmg08_cgb_out0
                //
                raise_lcd_interrupt &= (scanline != 0) || (next_scanline_offset <= gb_cycles_per_scanline - gb_cycles_mode2);
                //
                // If an LYC interrupt was possible just when this write occurs,
                // (LY == LYC and the STAT LYC interrupt flag was set), no LCD
                // interrupt will be raised for mode 0.
                //
                //      miscmstatirq/m0statwirq_trigger_ly44_lyc44_00_08_dmg08_cgb_outE2
                //      miscmstatirq/m0statwirq_trigger_ly44_lyc44_40_08_dmg08_cgb_outE0
                //      miscmstatirq/m0statwirq_trigger_ly44_lyc44_40_48_dmg08_cgb_outE0
                //
                raise_lcd_interrupt &= !lyc;
                //
                // On a CGB, the mode 0 interrupt will only be raised immediately,
                // if the next scanline is more than 4 cycles away (2 cycles, if
                // running at double speed).
                //
                //      m0enable/late_enable_1_dmg08_cgb_out2
                //      m0enable/late_enable_2_dmg08_out2_cgb_out0
                //      m0enable/late_enable_3_dmg08_cgb_out0
                //      m0enable/late_enable_ds_1_out3
                //      m0enable/late_enable_ds_2_out1
                //
                raise_lcd_interrupt &= !m_device.is_cgb() || (next_scanline_offset > m_clock.get_machine_cycle_clocks());
                //
                // The mode 0 interrupt is not raised before it
                // would be raised automatically after mode 3
                // (there may be a 1 cycle delay).
                //
                //      miscmstatirq/m0statwirq_scx3_1_dmg08_out0
                //
                raise_lcd_interrupt &= mode0_interrupt_offset != 1;
                LOG((raise_lcd_interrupt ? "" : "NOT ") << "raising " << (m_cgb ? "CGB" : "DMG") << " LCD interrupt for mode 0");

                //
                // On a CGB not running at double speed, the LCD interrupt for
                // mode 1 is triggered only, if the LCD interrupt for mode 0
                // has been disabled for more than 4 cycles.
                //
                //      m1/m1irq_m0disable_1_dmg08_cgb_out3
                //      m1/m1irq_m0disable_2_dmg08_out3_cgb_out1
                //      m1/m1irq_m0disable_3_dmg08_cgb_out1
                //      m1/m1irq_m0disable_ds_1_out3
                //      m1/m1irq_m0disable_ds_2_out1
                //
                m_skip_next_mode1_interrupt = m_device.is_cgb() && !m_clock.is_double_speed()
                        && ((bits_cleared & gb_stat_interrupt_mode0) > 0)
                        && (scanline == gb_screen_height - 1)
                        && (next_scanline_offset <= 4);

                //
                // verified by gambatte tests
                //
                // The mode 2 interrupt will be triggered immediately, if we're
                // allowing it the same time it would normally be flagged.
                //
                //      m2enable/late_enable_1_dmg08_cgb_out2
                //      m2enable/late_enable_2_dmg08_cgb_out0
                //
                // On CGB, the STAT mode 0 interrupt flag of the value to be
                // written is taken into account, on DMG the old STAT mode 0
                // interrupt flag is checked.
                //
                //      m2enable/late_enable_m0disable_1_dmg08_cgb_out2
                //      m2enable/late_enable_m0disable_2_dmg08_out0_cgb_out2
                //      m2enable/late_enable_m0disable_3_dmg08_cgb_out0
                //
                // A DMG considers the current LYC coincidence state before
                // triggering a mode 2 interrupt. A CGB ignores the LYC here.
                //
                //      m2enable/late_enable_after_lycint_1_dmg08_cgb_out0
                //      m2enable/late_enable_after_lycint_2_dmg08_out0_cgb_out2
                //      m2enable/late_enable_after_lycint_3_dmg08_cgb_out0
                //      m2enable/late_enable_after_lycint_4_dmg08_cgb_out0
                //      m2enable/late_enable_after_lycint_disable_1_dmg08_cgb_out2
                //      m2enable/late_enable_after_lycint_disable_2_dmg08_out0_cgb_out2
                //      m2enable/late_enable_after_lycint_disable_3_dmg08_cgb_out0
                //
                LOG("next_scanline_offset " << next_scanline_offset);
                if (((bits_set & gb_stat_interrupt_mode2) > 0)
                        && (scanline < gb_screen_height - 1)
                        && (((m_device.is_cgb() ? value : old_stat) & gb_stat_interrupt_mode0) == 0)
                        && (m_device.is_cgb() || !is_interruptable_coincidence())
                        && (next_scanline_offset == 4))
                {
                    LOG("raising LCD interrupt for mode 2");
                    m_interrupts.trigger_interrupt(gb_interrupt::lcd);
                }
                break;
            }

            case 1:
            {
                //
                // verified by gambatte tests
                //
                // DMG:
                // During mode 1, an LCD interrupt will be triggered regardless of
                // the currently written value, if the mode 1 STAT interrupt flag
                // was not set before.
                //
                // CGB:
                // During mode 1, an LCD interrupt will be triggered, if the STAT
                // mode 1 interrupt flag was changed from 0 to 1 with this write.
                //
                //      miscmstatirq/m1statwirq_1_dmg08_out3
                //      miscmstatirq/m1statwirq_2_dmg08_out3
                //      miscmstatirq/m1statwirq_3_dmg08_out2
                //      miscmstatirq/m1statwirq_4_dmg08_out0
                //      miscmstatirq/m1statwirq_trigger_00_00_dmg08_out2_cgb_out0
                //      miscmstatirq/m1statwirq_trigger_00_10_dmg08_out2_cgb_out2
                //      miscmstatirq/m1statwirq_trigger_00_ef_dmg08_out2_cgb_out0
                //      miscmstatirq/m1statwirq_trigger_00_ff_dmg08_out2_cgb_out2
                //      miscmstatirq/m1statwirq_trigger_10_00_dmg08_cgb_out0
                //      miscmstatirq/m1statwirq_trigger_10_10_dmg08_cgb_out0
                //      miscmstatirq/m1statwirq_trigger_10_ef_dmg08_cgb_out0
                //      miscmstatirq/m1statwirq_trigger_10_ff_dmg08_cgb_out0
                //      miscmstatirq/m1statwirq_trigger_ef_00_dmg08_out2_cgb_out0
                //      miscmstatirq/m1statwirq_trigger_ef_10_dmg08_out2_cgb_out2
                //      miscmstatirq/m1statwirq_trigger_ef_ef_dmg08_out2_cgb_out0
                //      miscmstatirq/m1statwirq_trigger_ef_ff_dmg08_out2_cgb_out2
                //      miscmstatirq/m1statwirq_trigger_ff_00_dmg08_cgb_out0
                //      miscmstatirq/m1statwirq_trigger_ff_10_dmg08_cgb_out0
                //      miscmstatirq/m1statwirq_trigger_ff_ef_dmg08_cgb_out0
                //      miscmstatirq/m1statwirq_trigger_ff_ff_dmg08_cgb_out0
                //
                raise_lcd_interrupt = m_device.is_cgb() ? ((bits_set & gb_stat_interrupt_mode1) > 0) : ((old_stat & gb_stat_interrupt_mode1) == 0);
                //
                // If an LYC interrupt was possible just when this write occurs,
                // (LY == LYC and the STAT LYC interrupt flag was set), no LCD
                // interrupt will be raised for mode 1.
                //
                //      miscmstatirq/m1statwirq_trigger_ly94_lyc94_00_10_dmg08_cgb_outE2
                //      miscmstatirq/m1statwirq_trigger_ly94_lyc94_00_50_dmg08_cgb_outE2
                //      miscmstatirq/m1statwirq_trigger_ly94_lyc94_40_10_dmg08_cgb_outE0
                //      miscmstatirq/m1statwirq_trigger_ly94_lyc94_40_50_dmg08_cgb_outE0
                //
                // For CGB the above rule holds only until the last 4 cycles of
                // a scanline, if not running in double speed mode.
                //
                //      miscmstatirq/m1statwirq_trigger_ly94_lyc94_40_50_1_dmg08_cgb_outE0
                //      miscmstatirq/m1statwirq_trigger_ly94_lyc94_40_50_2_dmg08_outE0_cgb_outE2
                //      miscmstatirq/m1statwirq_trigger_ly94_lyc94_40_50_3_dmg08_cgb_outE2
                //      miscmstatirq/m1statwirq_trigger_ly94_lyc94_40_50_ds_1_outE0
                //      miscmstatirq/m1statwirq_trigger_ly94_lyc94_40_50_ds_2_outE2
                //
                raise_lcd_interrupt &= !lyc || (m_device.is_cgb() && (next_scanline_offset <= (m_clock.is_double_speed() ? 0 : 4)));
                //
                // For scanline 153 on a CGB not running at double speed, the mode 1
                // interrupt is not triggered immediately, if this scanline does not
                // have more than 4 cycles left.
                //
                //      m1/m1irq_late_enable_1_dmg08_cgb_out2
                //      m1/m1irq_late_enable_2_dmg08_out2_cgb_out0
                //      m1/m1irq_late_enable_3_dmg08_cgb_out0
                //      m1/m1irq_late_enable_ds_1_out2
                //      m1/m1irq_late_enable_ds_2_out0
                //
                raise_lcd_interrupt &= !m_device.is_cgb() || (scanline != 153) || m_clock.is_double_speed() || (next_scanline_offset > 4);
                LOG((raise_lcd_interrupt ? "" : "NOT ") << "raising " << (m_cgb ? "CGB" : "DMG") << " LCD interrupt for mode 1");
                break;
            }

            case 2:
            {
                //
                // verified by gambatte tests
                //
                // An LCD interrupt for mode 2 is not triggered when writing the
                // STAT during mode 2, except for when the write happens on the
                // exact first mode 2 cycle (see below).
                //
                //      miscmstatirq/m2disable_dmg08_cgb_dmg08_out0
                //      miscmstatirq/m2statwirq_trigger_00_00_dmg08_cgb_out0
                //      miscmstatirq/m2statwirq_trigger_00_20_dmg08_cgb_out0
                //      miscmstatirq/m2statwirq_trigger_00_df_dmg08_cgb_out0
                //      miscmstatirq/m2statwirq_trigger_00_ff_dmg08_cgb_out0
                //      miscmstatirq/m2statwirq_trigger_20_00_dmg08_cgb_out0
                //      miscmstatirq/m2statwirq_trigger_20_20_dmg08_cgb_out0
                //      miscmstatirq/m2statwirq_trigger_20_df_dmg08_cgb_out0
                //      miscmstatirq/m2statwirq_trigger_20_ff_dmg08_cgb_out0
                //      miscmstatirq/m2statwirq_trigger_df_00_dmg08_cgb_out0
                //      miscmstatirq/m2statwirq_trigger_df_20_dmg08_cgb_out0
                //      miscmstatirq/m2statwirq_trigger_df_df_dmg08_cgb_out0
                //      miscmstatirq/m2statwirq_trigger_df_ff_dmg08_cgb_out0
                //      miscmstatirq/m2statwirq_trigger_ff_00_dmg08_cgb_out0
                //      miscmstatirq/m2statwirq_trigger_ff_20_dmg08_cgb_out0
                //      miscmstatirq/m2statwirq_trigger_ff_df_dmg08_cgb_out0
                //      miscmstatirq/m2statwirq_trigger_ff_ff_dmg08_cgb_out0
                //
                // The mode 2 interrupt will be triggered immediately, when setting
                // the STAT mode 2 interrupt flag, running at double speed and
                // mode 2 was just started.
                //
                //      m2enable/late_enable_ds_1_out2
                //      m2enable/late_enable_ds_2_out0
                //
                // On CGB, the STAT mode 0 interrupt flag of the value currently
                // being written is checked before triggering the LCD interrupt.
                //
                //      m2enable/late_enable_m0disable_ds_1_out2
                //      m2enable/late_enable_m0disable_ds_2_out0
                //
                if (((bits_set & gb_stat_interrupt_mode2) > 0)
                        && m_clock.is_double_speed()
                        && ((value & gb_stat_interrupt_mode0) == 0)
                        && (next_scanline_offset == gb_cycles_per_scanline))
                {
                    LOG("raising CGB LCD interrupt for mode 2");
                    m_interrupts.trigger_interrupt(gb_interrupt::lcd);
                }
                break;
            }
        }

        // raise LCD interrupt, if required
        if (raise_lcd_interrupt)
        {
            m_interrupts.trigger_interrupt(gb_interrupt::lcd);
        }

        //
        // verified by gambatte tests
        //
        // On a CGB clearing the STAT mode 0 interrupt flag must
        // happen more than 2 cycles before the next mode 0 interrupt
        // to have any effect on that upcoming interrupt.
        //
        //      m0enable/disable_1_dmg08_cgb_out0
        //      m0enable/disable_2_dmg08_out0_cgb_out2
        //      m0enable/disable_3_dmg08_cgb_out2
        //
        // When running at double speed, the number of cycles in the rule
        // above is reduced from 2 to 1.
        //
        //      m0enable/disable_ds_1_out1
        //      m0enable/disable_ds_2_out3
        //      m0enable/disable_scx5_ds_1_out1
        //      m0enable/disable_scx5_ds_2_out3
        //
        // The same behaviour is observed for the STAT coincidence
        // interrupt flag influencing the mode 0 interrupt.
        //
        //      m0enable/lycdisable_ff41_1_dmg08_cgb_out2
        //      m0enable/lycdisable_ff41_2_dmg08_out2_cgb_out0
        //      m0enable/lycdisable_ff41_3_dmg08_cgb_out0
        //
        // If the interrupt just happened (mode0_interrupt_offset == 0),
        // it does of course not make any sense to delay any change.
        //
        if (m_device.is_cgb() && (mode0_interrupt_offset > 0) && (mode0_interrupt_offset <= (m_clock.is_double_speed() ? 1 : 2)))
        {
            LOG("delaying mode 0 stat change");
            m_stat_mode0_int = old_stat | bits_set;
        }
    }
    set_stat(m_stat, mode, m_lcd_enabled);
}





//---------------------------------------------------------
//
//   scx, scy, wx, wy
//
//---------------------------------------------------------

void age::gb_lcd::write_scx(uint8_t value)
{
    //
    // verified by gambatte tests
    //
    // On a CGB, writes to SCX are delayed by 1 cycle.
    //
    //      scx_during_m3/scx_0360c0/scx_during_m3_ds_3
    //      scx_during_m3/scx_0360c0/scx_during_m3_ds_4
    //
    if (m_device.is_cgb())
    {
        emulate(m_clock.get_clock_cycle() + 1);
    }

    gb_lcd_ppu::write_scx(value);
}

void age::gb_lcd::write_scy(uint8_t value)
{
    //
    // verified by gambatte tests
    //
    // On a CGB, writes to SCY are delayed by 1 cycle.
    //
    //      scy/scy_during_m3_2
    //      scy/scy_during_m3_4
    //      scy/scy_during_m3_6
    //      scy/scy_during_m3_ds_1
    //      scy/scy_during_m3_ds_2
    //      scy/scy_during_m3_ds_3
    //      scy/scy_during_m3_ds_4
    //      scy/scy_during_m3_ds_5
    //      scy/scy_during_m3_ds_6
    //      scy/scy_during_m3_ds_7
    //
    if (m_device.is_cgb())
    {
        emulate(m_clock.get_clock_cycle() + 1);
    }

    gb_lcd_ppu::write_scy(value);
}



void age::gb_lcd::write_wx(uint8_t value)
{
    //
    // verified by gambatte tests
    //
    // Writes to WX are delayed by 1 cycle.
    //
    //      window/late_wx_scx2_1_dmg08_cgb_out0
    //      window/late_wx_scx2_2_dmg08_cgb_out3
    //      window/late_wx_scx3_1_dmg08_cgb_out0
    //      window/late_wx_scx3_2_dmg08_out0_cgb_out3
    //      window/late_wx_ds_1_out0
    //      window/late_wx_ds_2_out3
    //
    LOG(AGE_LOG_HEX(value));
    emulate(m_clock.get_clock_cycle() + 1);
    gb_lcd_ppu::write_wx(value);
}

void age::gb_lcd::write_wy(uint8_t value)
{
    //
    // verified by gambatte tests
    //
    // Writes to WY are delayed by 1 cycle.
    //
    //      window/late_wy_ffto2_ly2_1_dmg08_cgb_out3
    //      window/late_wy_ffto2_ly2_scx2_1_dmg08_cgb_out3
    //      window/late_wy_ffto2_ly2_scx3_1_dmg08_cgb_out3
    //      window/late_wy_ffto2_ly2_scx5_1_dmg08_cgb_out3
    //
    LOG(AGE_LOG_HEX(value));
    emulate(m_clock.get_clock_cycle() + 1);
    gb_lcd_ppu::write_wy(value);
    write_late_wy();
}





//---------------------------------------------------------
//
//   ly & lyc
//
//---------------------------------------------------------

age::uint8_t age::gb_lcd::read_ly() const
{
    return get_ly_port(m_lcd_enabled);
}

age::uint8_t age::gb_lcd::read_lyc() const
{
    return get_lyc();
}

void age::gb_lcd::write_lyc(uint8_t value)
{
    LOG(AGE_LOG_HEX(value));
    if (value != get_lyc())
    {
        auto scanline = get_scanline();
        int mode = (scanline >= gb_screen_height)
                ? 1 // the 4-cycle mode 0 at the end of mode 1 is not recognized here
                : (m_stat & gb_stat_modes);

        uint8_t old_lyc = get_lyc();
        set_lyc(value, mode, m_lcd_enabled);

        //
        // verified by gambatte tests
        //
        // On a CGB changing the LYC value must happen more than
        // 6 cycles before the next mode 0 interrupt to have any
        // effect on that upcoming interrupt.
        //
        //      m0enable/lycdisable_ff45_1_dmg08_cgb_out2
        //      m0enable/lycdisable_ff45_2_dmg08_out2_cgb_out0
        //      m0enable/lycdisable_ff45_3_dmg08_out2_cgb_out0
        //      m0enable/lycdisable_ff45_4_dmg08_cgb_out0
        //
        // When running at double speed, the number of cycles in the rule
        // above is reduced from 6 to 2.
        //
        //      m0enable/lycdisable_ff45_scx1_ds_1_out2
        //      m0enable/lycdisable_ff45_scx1_ds_2_out0
        //
        // An a DMG, the mode 0 interrupt must be at least 2 cycles
        // away fo the LYC change to have an effect on it.
        //
        //      m0enable/lycdisable_ff45_3_dmg08_out2_cgb_out0
        //      m0enable/lycdisable_ff45_scx3_3_dmg08_cgb_out0
        //
        // If the interrupt just happened (mode0_interrupt_offset == 0),
        // it does of course not make any sense to delay any change.
        //
        // DMG & CGB single speed:
        //
        // SCX 0:  12932  LYC 255
        //         12941  mode 3 finished
        //         12942  mode 0 interrupt
        //         12996  IF 2
        //
        //         12936  LYC 255
        //         12941  mode 3 finished
        //         12942  mode 0 interrupt
        //         12996  IF 0 (CGB)  2 (DMG)
        //
        //         12940  LYC 255
        //         12941  mode 3 finished
        //         12942  mode 0 interrupt
        //         12996  IF 0 (CGB)  2 (DMG)
        //
        //         12941  mode 3 finished
        //         12942  mode 0 interrupt
        //         12944  LYC 255
        //         12996  IF 0
        //
        // SCX 3:  12936  LYC 255
        //         12944  mode 3 finished
        //         12945  mode 0 interrupt
        //         13000  IF 2
        //
        //         12940  LYC 255
        //         12944  mode 3 finished
        //         12945  mode 0 interrupt
        //         13000  IF 0 (CGB)  2 (DMG)
        //
        //         12944  mode 3 finished
        //         12944  LYC 255
        //         12945  mode 0 interrupt
        //         13000  IF 0
        //
        // CGB double speed:
        //
        // SCX 1:  12942  LYC 255, IF 0
        //         12945  mode 3 finished
        //         12945  mode 0 interrupt
        //         12984  IF 2
        //
        //         12944  LYC 255, IF 0
        //         12945  mode 3 finished
        //         12945  mode 0 interrupt
        //         12984  IF 0
        //
        int mode0_interrupt_offset = get_mode0_interrupt_cycle_offset();
        LOG("mode0_interrupt_offset " << mode0_interrupt_offset);

        int min_offset = m_device.is_cgb() ? (m_clock.is_double_speed() ? 2 : 6) : 1;
        LOG("min_offset " << min_offset);

        if ((mode0_interrupt_offset > 0) && (mode0_interrupt_offset <= min_offset))
        {
            LOG("delaying LYC change");
            m_lyc_mode0_int = old_lyc;
        }
        else
        {
            m_lyc_mode0_int = get_lyc();
        }
    }
}





//---------------------------------------------------------
//
//   color palettes
//
//---------------------------------------------------------

age::uint8_t age::gb_lcd::read_bgp() const
{
    return m_bgp;
}

age::uint8_t age::gb_lcd::read_obp0() const
{
    return m_obp0;
}

age::uint8_t age::gb_lcd::read_obp1() const
{
    return m_obp1;
}



void age::gb_lcd::write_bgp(uint8_t value)
{
    LOG(AGE_LOG_HEX(value));
    m_bgp = value;
    create_classic_palette(gb_palette_bgp, m_bgp);
}

void age::gb_lcd::write_obp0(uint8_t value)
{
    m_obp0 = value;
    create_classic_palette(gb_palette_obp0, m_obp0);
}

void age::gb_lcd::write_obp1(uint8_t value)
{
    m_obp1 = value;
    create_classic_palette(gb_palette_obp1, m_obp1);
}



age::uint8_t age::gb_lcd::read_bcps() const
{
    AGE_ASSERT(m_device.is_cgb());
    return m_bcps;
}

age::uint8_t age::gb_lcd::read_bcpd() const
{
    AGE_ASSERT(m_device.is_cgb());
    uint8_t result = is_cgb_palette_accessible() ? m_palette[m_bcps & 0x3F] : 0xFF;
    return result;
}

age::uint8_t age::gb_lcd::read_ocps() const
{
    AGE_ASSERT(m_device.is_cgb());
    return m_ocps;
}

age::uint8_t age::gb_lcd::read_ocpd() const
{
    AGE_ASSERT(m_device.is_cgb());
    uint8_t result = is_cgb_palette_accessible() ? m_palette[0x40 + (m_ocps & 0x3F)] : 0xFF;
    return result;
}



void age::gb_lcd::write_bcps(uint8_t value)
{
    AGE_ASSERT(m_device.is_cgb());
    m_bcps = value | 0x40;
}

void age::gb_lcd::write_bcpd(uint8_t value)
{
    AGE_ASSERT(m_device.is_cgb());
    if (is_cgb_palette_accessible())
    {
        m_palette[m_bcps & 0x3F] = value;
        update_color((m_bcps & 0x3F) / 2);

        // increment index
        if ((m_bcps & 0x80) > 0)
        {
            uint8_t index = (m_bcps + 1) & 0x3F;
            m_bcps &= 0xC0;
            m_bcps |= index;
        }
    }
}

void age::gb_lcd::write_ocps(uint8_t value)
{
    AGE_ASSERT(m_device.is_cgb());
    m_ocps = value | 0x40;
}

void age::gb_lcd::write_ocpd(uint8_t value)
{
    AGE_ASSERT(m_device.is_cgb());
    if (is_cgb_palette_accessible())
    {
        m_palette[0x40 + (m_ocps & 0x3F)] = value;
        update_color(0x20 + (m_ocps & 0x3F) / 2);

        // increment index
        if ((m_ocps & 0x80) > 0)
        {
            uint8_t index = (m_ocps + 1) & 0x3F;
            m_ocps &= 0xC0;
            m_ocps |= index;
        }
    }
}





//---------------------------------------------------------
//
//   palette utility methods
//
//---------------------------------------------------------

void age::gb_lcd::update_color(unsigned index)
{
    uint8_t low_byte = m_palette[index * 2];
    uint8_t high_byte = m_palette[index * 2 + 1];
    gb_lcd_ppu::update_color(index, high_byte, low_byte);
}

bool age::gb_lcd::is_cgb_palette_accessible() const
{
    bool allowed = true;
    if (m_lcd_enabled)
    {
        bool double_speed = m_clock.is_double_speed();
        int current_cycle = m_clock.get_clock_cycle();

        int mode = m_stat & gb_stat_modes;
        allowed = mode != 3;

        //
        // verified by gambatte tests
        //
        // CGB color palettes are accessible while not currently
        // running in mode 3 and if mode 3 was finished at least
        // 4 cycles ago.
        //
        //      cgbpal_m3/cgbpal_m3end_1_out7
        //      cgbpal_m3/cgbpal_m3end_2_out0
        //      cgbpal_m3/cgbpal_m3end_3_out0
        //      cgbpal_m3/cgbpal_m3end_4_out1
        //      cgbpal_m3/cgbpal_m3end_scx2_1_out7
        //      cgbpal_m3/cgbpal_m3end_scx2_2_out0
        //      cgbpal_m3/cgbpal_m3end_scx2_3_out0
        //      cgbpal_m3/cgbpal_m3end_scx2_4_out1
        //      cgbpal_m3/cgbpal_m3end_scx3_1_out7
        //      cgbpal_m3/cgbpal_m3end_scx3_2_out0
        //      cgbpal_m3/cgbpal_m3end_scx3_3_out0
        //      cgbpal_m3/cgbpal_m3end_scx3_4_out1
        //      cgbpal_m3/cgbpal_m3end_scx5_1_out7
        //      cgbpal_m3/cgbpal_m3end_scx5_2_out0
        //      cgbpal_m3/cgbpal_m3end_scx5_3_out0
        //      cgbpal_m3/cgbpal_m3end_scx5_4_out1
        //
        // The number of cycles used above is reduced from 4 to
        // 2 when running at double speed.
        //
        //      cgbpal_m3/cgbpal_m3end_ds_1_out7
        //      cgbpal_m3/cgbpal_m3end_ds_2_out0
        //      cgbpal_m3/cgbpal_m3end_ds_3_out0
        //      cgbpal_m3/cgbpal_m3end_ds_4_out1
        //      cgbpal_m3/cgbpal_m3end_scx5_ds_1_out7
        //      cgbpal_m3/cgbpal_m3end_scx5_ds_2_out0
        //      cgbpal_m3/cgbpal_m3end_scx5_ds_3_out0
        //      cgbpal_m3/cgbpal_m3end_scx5_ds_4_out1
        //
        AGE_ASSERT(m_last_cycle_m3_finished >= 0);
        AGE_ASSERT(m_last_cycle_m3_finished <= current_cycle);
        int m3_cycle_diff = current_cycle - m_last_cycle_m3_finished;
        int min_cycle_diff = m_clock.get_machine_cycle_clocks();
        allowed &= m3_cycle_diff >= min_cycle_diff;

        //
        // verified by gambatte tests
        //
        // When running at double speed, CGB color palettes can
        // still be accessed during the first cycle of mode 3.
        //
        //      cgbpal_m3/cgbpal_m3start_1_out1
        //      cgbpal_m3/cgbpal_m3start_2_out0
        //      cgbpal_m3/cgbpal_m3start_ds_1_out1
        //      cgbpal_m3/cgbpal_m3start_ds_2_out0
        //
        if (double_speed)
        {
            int next_scanline_offset = get_next_scanline_cycle_offset(current_cycle);
            allowed |= next_scanline_offset >= gb_cycles_per_scanline - gb_cycles_mode2;
        }
    }
    return allowed;
}
