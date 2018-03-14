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

#include <algorithm>

#include <age_debug.hpp>

#include "age_gb_lcd.hpp"

#if 0
#define LOG(x) if ((m_next_event_cycle - 0 < 15000)) { AGE_LOG("cycle " << m_next_event_cycle << ": " << x); }
#else
#define LOG(x)
#endif



// memory dumps,
// based on *.bin files used by gambatte tests and gambatte source code (initstate.cpp)

constexpr const age::uint8_array<0x40> cgb_objp_dump =
{{
     0x00, 0x00, 0xF2, 0xAB, 0x61, 0xC2, 0xD9, 0xBA, 0x88, 0x6E, 0xDD, 0x63, 0x28, 0x27, 0xFB, 0x9F,
     0x35, 0x42, 0xD6, 0xD4, 0x50, 0x48, 0x57, 0x5E, 0x23, 0x3E, 0x3D, 0xCA, 0x71, 0x21, 0x37, 0xC0,
     0xC6, 0xB3, 0xFB, 0xF9, 0x08, 0x00, 0x8D, 0x29, 0xA3, 0x20, 0xDB, 0x87, 0x62, 0x05, 0x5D, 0xD4,
     0x0E, 0x08, 0xFE, 0xAF, 0x20, 0x02, 0xD7, 0xFF, 0x07, 0x6A, 0x55, 0xEC, 0x83, 0x40, 0x0B, 0x77
 }};





//---------------------------------------------------------
//
//   object creation.
//
//---------------------------------------------------------

age::gb_lcd::gb_lcd(gb_core &core, const gb_memory &memory, video_buffer_handler &frame_handler, bool dmg_green)
    : gb_lcd_ppu(core, memory, dmg_green),
      m_cgb(core.is_cgb()),
      m_core(core),
      m_video_buffer_handler(frame_handler)
{
    // initialize color palettes
    std::fill(begin(m_palette), begin(m_palette) + 0x40, 0xFF);
    std::copy(begin(cgb_objp_dump), end(cgb_objp_dump), begin(m_palette) + 0x40);

    write_bgp(0xFC);
    if (m_cgb)
    {
        for (uint i = 0; i < m_palette.size() / 2; ++i)
        {
            update_color(i);
        }
    }
    else
    {
        write_obp0(0xFF);
        write_obp1(0xFF);
    }

    // init event handling and precalculated values
    m_next_event_cycle = m_core.get_oscillation_cycle();
    if (m_cgb)
    {
        next_step(get_next_scanline_cycle_offset(m_next_event_cycle), &mode1_start_line);
    }
    else
    {
        next_step(56, &mode1_last_cycles_mode0);
    }
    write_lcdc(0x91);
}





//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

bool age::gb_lcd::is_video_ram_accessible() const
{
    bool result = (m_stat & gb_stat_modes) != 3;
    return result;
}

bool age::gb_lcd::is_oam_readable() const
{
    bool result = (m_stat & gb_stat_modes) < 2;
    //
    // verified by gambatte tests
    //
    // OAM is not readable once the (potential) mode 2 interrupt
    // was triggered. This does however not apply when running
    // at double speed.
    //
    //      oam_access/preread_1_dmg08_cgb_out0
    //      oam_access/preread_2_dmg08_cgb_out3
    //      oam_access/preread_ds_1_out0
    //      oam_access/preread_ds_2_out3
    //
    if (m_lcd_enabled && !m_core.is_double_speed())
    {
        if (get_scanline() < gb_screen_height)
        {
            int current_cycle = m_core.get_oscillation_cycle();
            uint next_scanline_offset = get_next_scanline_cycle_offset(current_cycle);
            result &= next_scanline_offset > 4;
        }
    }
    return result;
}

bool age::gb_lcd::is_oam_writable() const
{
    bool result = (m_stat & gb_stat_modes) < 2;
    //
    // verified by gambatte tests
    //
    // On a CGB OAM is not writable once the (potential) mode 2
    // interrupt was triggered. This does not apply when running
    // on a DMG.
    //
    //      oam_access/prewrite_1_dmg08_cgb_out1
    //      oam_access/prewrite_2_dmg08_out1_cgb_out0
    //      oam_access/prewrite_3_dmg08_cgb_out0
    //      oam_access/prewrite_ds_1_out1
    //      oam_access/prewrite_ds_2_out0
    //
    if (m_lcd_enabled && m_cgb)
    {
        if (get_scanline() < gb_screen_height)
        {
            int current_cycle = m_core.get_oscillation_cycle();
            uint next_scanline_offset = get_next_scanline_cycle_offset(current_cycle);
            result &= next_scanline_offset > m_core.get_machine_cycles_per_cpu_cycle();
        }
    }
    return result;
}

bool age::gb_lcd::is_hdma_active() const
{
    return m_hdma_active;
}



void age::gb_lcd::emulate()
{
    uint current_cycle = m_core.get_oscillation_cycle();
    emulate(current_cycle);
}

void age::gb_lcd::set_hdma_active(bool hdma_active)
{
    //
    // verified by gambatte tests
    //
    // HDMA can be activated for a currently active
    // mode 0, if the next scanline is more than
    // 4 cycles away (2 cycles when running at
    // double speed).
    //
    //      dma/hdma_late_enable_1_out1
    //      dma/hdma_late_enable_2_out0
    //      dma/hdma_late_enable_ds_1_out1
    //      dma/hdma_late_enable_ds_2_out0
    //
    if (!m_hdma_active && hdma_active)
    {
        uint8 mode = m_stat & gb_stat_modes;
        uint next_scanline_offset = m_lcd_enabled ? get_next_scanline_cycle_offset(m_core.get_oscillation_cycle()) : gb_no_cycle;
        if ((mode == 0) && (next_scanline_offset > m_core.get_machine_cycles_per_cpu_cycle()))
        {
            m_core.insert_event(0, gb_event::start_hdma);
        }
    }

    m_hdma_active = hdma_active;
}





//---------------------------------------------------------
//
//   misc private methods
//
//---------------------------------------------------------

void age::gb_lcd::emulate(uint to_cycle)
{
    while (to_cycle >= m_next_event_cycle)
    {
        AGE_ASSERT(m_next_event != nullptr);
        m_next_event(*this);
    }
}

void age::gb_lcd::next_step(uint cycle_offset, std::function<void(gb_lcd&)> step)
{
    m_next_event_cycle += cycle_offset;
    m_next_event = step;
    AGE_ASSERT(get_next_scanline_cycle_offset(m_next_event_cycle) < gb_cycles_per_scanline);
}

void age::gb_lcd::switch_frames()
{
    m_video_buffer_handler.switch_buffers();
}





//---------------------------------------------------------
//
//   emulate mode 0
//
//---------------------------------------------------------

void age::gb_lcd::mode0_start_lcd_enabled()
{
    AGE_ASSERT(get_scanline() == 0);

    //
    // verified by gambatte tests
    //
    // When the LCD is enabled, it will start with mode 0 instead
    // of mode 2 (it will still last for 80 cycles though). After
    // that, mode 3 will be started.
    //
    //      enable_display/nextstat_1_dmg08_cgb_out84
    //      enable_display/nextstat_2_dmg08_cgb_out87
    //      enable_display/stat_dmg08_cgb_out84
    //
    m_stat &= ~gb_stat_modes;
    next_step(gb_cycles_mode2, &mode3_start);
}



void age::gb_lcd::mode0_interrupt()
{
    //AGE_ASSERT((m_stat & gb_stat_modes) == 0);
    AGE_ASSERT(get_scanline() < gb_screen_height);

    //
    // verified by gambatte tests
    //
    // The LCD interrupt for mode 0 is not triggered, if an
    // LYC interrupt could have been triggered for this LY.
    //
    //      lcdirq_precedence/m0irq_ly44_lcdstat48_lyc43_dmg08_cgb_out2
    //      lcdirq_precedence/m0irq_ly44_lcdstat48_lyc44_dmg08_cgb_out0
    //      lcdirq_precedence/m0irq_ly44_lcdstat48_lyc45_dmg08_cgb_out2
    //      lyc0int_m0irq/lyc0int_m0irq_1_dmg08_cgb_out0
    //      lyc0int_m0irq/lyc0int_m0irq_2_dmg08_cgb_out2
    //      lyc0int_m0irq/lyc0int_m0irq_ds_1_out0
    //      lyc0int_m0irq/lyc0int_m0irq_ds_2_out2
    //
    if (((m_stat_mode0_int & gb_stat_interrupt_mode0) > 0)
            && !(((m_stat_mode0_int & gb_stat_interrupt_coincidence) > 0)
                 && (get_ly() == m_lyc_mode0_int)))
    {
        LOG("triggering mode 0 interrupt");
        m_core.request_interrupt(gb_interrupt::lcd);
    }
    m_stat_mode0_int = m_stat;
    m_lyc_mode0_int = get_lyc();
}



void age::gb_lcd::mode0_next_event()
{
    AGE_ASSERT((m_stat & gb_stat_modes) == 0);

    // start mode 1 or mode 2 after mode 0 finished
    // (when starting mode 1, we have finished rendering another frame)
    if (get_scanline() == (gb_screen_height - 1))
    {
        switch_frames();
        next_step(get_next_scanline_cycle_offset(m_next_event_cycle), &mode1_start_line);
    }

    //
    // verified by gambatte tests
    //
    // the mode 2 interrupt for LY >= 1 is triggered 4 cycles earlier
    // than the actual mode 2 routine (when running at double speed,
    // the number of cycles is reduced from 4 to 2)
    //
    //      m2int_m2stat/m2int_m2stat_1_dmg08_cgb_out2
    //      m2int_m2stat/m2int_m2stat_2_dmg08_cgb_out3
    //      m2int_m2stat/m2int_m2stat_ds_1_out2
    //      m2int_m2stat/m2int_m2stat_ds_2_out3
    //
    else
    {
        uint offset = get_next_scanline_cycle_offset(m_next_event_cycle) - m_core.get_machine_cycles_per_cpu_cycle();
        next_step(offset, &mode2_early_interrupt);
    }
}





//---------------------------------------------------------
//
//   emulate mode 1
//
//---------------------------------------------------------

void age::gb_lcd::mode1_start_line(gb_lcd &lcd)
{
    AGE_ASSERT((((lcd.m_stat & gb_stat_modes) == 0) && (lcd.get_scanline() == (gb_screen_height - 1)))
               || (((lcd.m_stat & gb_stat_modes) == 1) && (lcd.get_scanline() >= gb_screen_height) && (lcd.get_scanline() <= 153)));

    // update STAT and interrupt, if we just entered v-blank
    if (lcd.get_scanline() == (gb_screen_height - 1))
    {
        ++lcd.m_stat;
        lcd.m_core.request_interrupt(gb_interrupt::vblank);

        if (lcd.m_allow_mode1_interrupt && !lcd.m_skip_next_mode1_interrupt)
        {
            lcd.m_core.request_interrupt(gb_interrupt::lcd);
        }
        lcd.m_skip_next_mode1_interrupt = false;
    }
    AGE_ASSERT((lcd.m_stat & gb_stat_modes) == 1);
    AGE_ASSERT(!lcd.m_skip_next_mode1_interrupt);

    // enter the next scanline
    lcd.next_line();

    // stay at mode 1
    if (lcd.get_scanline() < 153)
    {
        lcd.next_step(lcd.get_next_scanline_cycle_offset(lcd.m_next_event_cycle), &mode1_start_line);
    }
    // special hanling for LY 153
    else
    {
        lcd.next_step(4, &mode1_start_ly0);
    }
}



void age::gb_lcd::mode1_start_ly0(gb_lcd &lcd)
{
    AGE_ASSERT((lcd.m_stat & gb_stat_modes) == 1);
    AGE_ASSERT(lcd.get_scanline() == 153);

    //
    // verified by gambatte tests
    //
    // LY 0 is triggered during mode 1, LY 153 is present only for 4 cycles
    //
    //      ly0/lycint152_ly153_1_dmg08_cgb_out98
    //      ly0/lycint152_ly153_2_dmg08_cgb_out99
    //      ly0/lycint152_ly153_3_dmg08_cgb_out00
    //
    lcd.mode1_ly0();

    lcd.next_step(gb_cycles_ly153, &mode1_update_ly0_lyc);
}

void age::gb_lcd::mode1_update_ly0_lyc(gb_lcd &lcd)
{
    AGE_ASSERT((lcd.m_stat & gb_stat_modes) == 1);
    AGE_ASSERT((lcd.get_ly() == 0) && (lcd.get_scanline() == 153));

    lcd.next_step(lcd.get_next_scanline_cycle_offset(lcd.m_next_event_cycle) - 4, &mode1_last_cycles_mode0);
}

void age::gb_lcd::mode1_last_cycles_mode0(gb_lcd &lcd)
{
    AGE_ASSERT(((lcd.m_stat & gb_stat_modes) == 1) && (lcd.get_scanline() == 153));

    //
    // verified by gambatte tests
    //
    // the last 4 cycles of mode 1 are actually signalled as mode 0
    // (only when not running at double speed)
    //
    //      ly0/lycint152_ly0stat_1_dmg08_cgb_outC1
    //      ly0/lycint152_ly0stat_2_dmg08_cgb_outC0
    //      ly0/lycint152_ly0stat_3_dmg08_cgb_outC2
    //      ly0/lycint152_ly0stat_ds_1_outC1
    //      ly0/lycint152_ly0stat_ds_2_outC1
    //      ly0/lycint152_ly0stat_ds_3_outC2
    //
    if (!lcd.m_core.is_double_speed())
    {
        --lcd.m_stat;
    }

    lcd.next_step(4, &mode2_start_ly0);
}





//---------------------------------------------------------
//
//   emulate mode 2
//
//---------------------------------------------------------

void age::gb_lcd::mode2_start_ly0(gb_lcd &lcd)
{
    AGE_ASSERT(lcd.get_scanline() == 153);

    // do the usual mode 2 stuff
    // (no LYC interrupt here though)
    lcd.m_stat &= ~gb_stat_modes;
    mode2_start(lcd);
    AGE_ASSERT(lcd.get_scanline() == 0);

    // trigger LCD interrupt, if requested
    // (do this after next_line() was called)
    if (lcd.m_allow_mode2_ly0_interrupt_int && !lcd.is_interruptable_coincidence())
    {
        lcd.m_core.request_interrupt(gb_interrupt::lcd);
    }
    lcd.m_allow_mode2_ly0_interrupt_int = lcd.m_allow_mode2_ly0_interrupt;
}



void age::gb_lcd::mode2_early_interrupt(gb_lcd &lcd)
{
    AGE_ASSERT(lcd.get_scanline() < (gb_screen_height - 1));

    //
    // verified by gambatte tests
    //
    // the mode 2 interrupt is triggered only, if no LYC
    // interrupt is currently possible
    //
    //      lycm2int/lyc0m2int_m2irq_1_dmg08_cgb_out0
    //      lycm2int/lyc0m2int_m2irq_2_dmg08_cgb_out2
    //      lycm2int/lycm2int_m2irq_1_dmg08_cgb_out1
    //      lycm2int/lycm2int_m2irq_2_dmg08_cgb_out3
    //
    if (lcd.m_allow_mode2_interrupt && !lcd.is_interruptable_coincidence())
    {
        lcd.m_core.request_interrupt(gb_interrupt::lcd);
    }

    // execute the actual mode 2 actions after the interrupt was triggered
    lcd.next_step(lcd.get_next_scanline_cycle_offset(lcd.m_next_event_cycle), &mode2_start);
}



void age::gb_lcd::mode2_start(gb_lcd &lcd)
{
    AGE_ASSERT((lcd.get_scanline() < (gb_screen_height - 1)) || (lcd.get_scanline() == 153));

    // update STAT
    AGE_ASSERT((lcd.m_stat & gb_stat_modes) == 0);
    lcd.m_stat |= 0x02;

    // enter the next scanline
    lcd.next_line();
    AGE_ASSERT(lcd.get_scanline() < gb_screen_height);

    // search OAM ram for sprite information
    lcd.search_sprites();

    // start mode 3 after mode 2 has finished
    lcd.next_step(gb_cycles_mode2, &mode3_start);
}





//---------------------------------------------------------
//
//   emulate mode 3
//
//---------------------------------------------------------

void age::gb_lcd::mode3_start(gb_lcd &lcd)
{
    //
    // verified by gambatte tests
    //
    // mode 3 starting 77-80 cycles after mode 2 began
    // (VRAM accessible 80 cycles after mode 2 interrupt)
    //
    //      vram_m3/vramw_m3start_1_dmg08_cgb_out1
    //      vram_m3/vramw_m3start_2_dmg08_cgb_out0
    //
    // mode 3 starting 79-80 cycles after mode 2 began
    // (VRAM accessible 536 (80) cycles after mode 2 interrupt)
    //
    //      vram_m3/preread_ds_1_out0
    //      vram_m3/preread_ds_2_out3
    //      vram_m3/prewrite_ds_1_out1
    //      vram_m3/prewrite_ds_2_out0
    //
    AGE_ASSERT(((lcd.m_stat & gb_stat_modes) == 2) || ((lcd.get_scanline() == 0) && (((lcd.m_stat & gb_stat_modes) == 0))));
    AGE_ASSERT(lcd.get_scanline() < gb_screen_height);
    AGE_ASSERT(lcd.get_scanline() == lcd.get_ly());

    // update STAT
    lcd.m_stat &= ~gb_stat_modes;
    lcd.m_stat |= 3;
    LOG("mode 3 started");

    // init PPU for this scanline
    pixel *first_scanline_pixel = lcd.m_video_buffer_handler.get_first_scanline_pixel(lcd.get_scanline());
    lcd.scanline_init(first_scanline_pixel);

    // next step
    uint offset = lcd.m_core.is_double_speed() ? 6 : 3;
    lcd.next_step(offset, &mode3_render);
}



void age::gb_lcd::mode3_render(gb_lcd &lcd)
{
    AGE_ASSERT(((lcd.m_stat & gb_stat_modes) == 0) || ((lcd.m_stat & gb_stat_modes) == 3));

    lcd.scanline_step();

    // continue plotting
    if (!lcd.scanline_nearly_finished())
    {
        lcd.next_step(1, &mode3_render);
    }
    else
    {
        // handle mode 0 events
        if (lcd.scanline_flag_mode0())
        {
            lcd.m_stat &= ~gb_stat_modes;
            lcd.m_m3_last_finished = lcd.m_next_event_cycle;

            if (lcd.m_hdma_active)
            {
                uint cycle = lcd.m_core.get_oscillation_cycle();
                uint offset = lcd.m_core.is_double_speed() ? 1 : 2;
                offset += lcd.m_next_event_cycle;
                offset = (offset > cycle) ? offset - cycle : 0;
                LOG("scheduling HDMA event, offset " << offset);
                lcd.m_core.insert_event(offset, gb_event::start_hdma);
            }
        }
        if (lcd.scanline_mode0_interrupt())
        {
            LOG("trying to trigger mode 0 interrupt");
            lcd.mode0_interrupt();
        }

        // finish plotting
        if (lcd.scanline_finished())
        {
            LOG("scanline plotting finished");
            lcd.mode0_next_event();
        }
        else
        {
            lcd.next_step(1, &mode3_render);
        }
    }
}
