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

#include "age_gb_bus.hpp"

#if 0
#define LOG(x) AGE_GB_CLOCK_LOG(x)
#else
#define LOG(x)
#endif

namespace
{

constexpr age::uint8_t gb_hdma_start = 0x80;

// memory dumps,
// based on *.bin files used by gambatte tests and gambatte source code (initstate.cpp)

constexpr const age::uint8_array<0x60> cgb_sparse_FEA0_dump =
{{
     // every line used four times:
     // 0x08, 0x01, 0xEF, 0xDE, ...
     // 0x08, 0x01, 0xEF, 0xDE, ...
     // ...
     0x08, 0x01, 0xEF, 0xDE, 0x06, 0x4A, 0xCD, 0xBD,
     0x00, 0x90, 0xF7, 0x7F, 0xC0, 0xB1, 0xBC, 0xFB,
     0x24, 0x13, 0xFD, 0x3A, 0x10, 0x10, 0xAD, 0x45
 }};

constexpr const age::uint8_array<0x80> dmg_FF80_dump =
{{
     0x2B, 0x0B, 0x64, 0x2F, 0xAF, 0x15, 0x60, 0x6D, 0x61, 0x4E, 0xAC, 0x45, 0x0F, 0xDA, 0x92, 0xF3,
     0x83, 0x38, 0xE4, 0x4E, 0xA7, 0x6C, 0x38, 0x58, 0xBE, 0xEA, 0xE5, 0x81, 0xB4, 0xCB, 0xBF, 0x7B,
     0x59, 0xAD, 0x50, 0x13, 0x5E, 0xF6, 0xB3, 0xC1, 0xDC, 0xDF, 0x9E, 0x68, 0xD7, 0x59, 0x26, 0xF3,
     0x62, 0x54, 0xF8, 0x36, 0xB7, 0x78, 0x6A, 0x22, 0xA7, 0xDD, 0x88, 0x15, 0xCA, 0x96, 0x39, 0xD3,
     0xE6, 0x55, 0x6E, 0xEA, 0x90, 0x76, 0xB8, 0xFF, 0x50, 0xCD, 0xB5, 0x1B, 0x1F, 0xA5, 0x4D, 0x2E,
     0xB4, 0x09, 0x47, 0x8A, 0xC4, 0x5A, 0x8C, 0x4E, 0xE7, 0x29, 0x50, 0x88, 0xA8, 0x66, 0x85, 0x4B,
     0xAA, 0x38, 0xE7, 0x6B, 0x45, 0x3E, 0x30, 0x37, 0xBA, 0xC5, 0x31, 0xF2, 0x71, 0xB4, 0xCF, 0x29,
     0xBC, 0x7F, 0x7E, 0xD0, 0xC7, 0xC3, 0xBD, 0xCF, 0x59, 0xEA, 0x39, 0x01, 0x2E, 0x00, 0x69, 0x00
 }};

constexpr const age::uint8_array<0x80> cgb_FF80_dump =
{{
     0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
     0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
     0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
     0x45, 0xEC, 0x42, 0xFA, 0x08, 0xB7, 0x07, 0x5D, 0x01, 0xF5, 0xC0, 0xFF, 0x08, 0xFC, 0x00, 0xE5,
     0x0B, 0xF8, 0xC2, 0xCA, 0xF4, 0xF9, 0x0D, 0x7F, 0x44, 0x6D, 0x19, 0xFE, 0x46, 0x97, 0x33, 0x5E,
     0x08, 0xFF, 0xD1, 0xFF, 0xC6, 0x8B, 0x24, 0x74, 0x12, 0xFC, 0x00, 0x9F, 0x94, 0xB7, 0x06, 0xD5,
     0x40, 0x7A, 0x20, 0x9E, 0x04, 0x5F, 0x41, 0x2F, 0x3D, 0x77, 0x36, 0x75, 0x81, 0x8A, 0x70, 0x3A,
     0x98, 0xD1, 0x71, 0x02, 0x4D, 0x01, 0xC1, 0xFF, 0x0D, 0x00, 0xD3, 0x05, 0xF9, 0x00, 0x0B, 0x00
 }};

}





//---------------------------------------------------------
//
//   Object creation.
//
//---------------------------------------------------------

age::gb_bus::gb_bus(const gb_device &device,
                    gb_clock &clock,
                    gb_interrupt_ports &interrupts,
                    gb_events &events,
                    gb_memory &memory,
                    gb_div &div,
                    gb_sound &sound,
                    gb_lcd &lcd,
                    gb_timer &timer,
                    gb_joypad &joypad,
                    gb_serial &serial)
    : m_device(device),
      m_clock(clock),
      m_interrupts(interrupts),
      m_events(events),
      m_memory(memory),
      m_div(div),
      m_sound(sound),
      m_lcd(lcd),
      m_timer(timer),
      m_joypad(joypad),
      m_serial(serial),
      m_oam_dma_byte(m_device.is_cgb() ? 0x00 : 0xFF)
{
    // clear high ram
    std::fill(begin(m_high_ram), end(m_high_ram), 0);

    // init 0xFF80 - 0xFFFE
    const uint8_array<0x80> &src = m_device.is_cgb() ? cgb_FF80_dump : dmg_FF80_dump;
    std::copy(begin(src), end(src), begin(m_high_ram) + 0x180);

    // init 0xFEA0 - 0xFEFF
    if (m_device.is_cgb())
    {
        for (int i = 0; i < 3; ++i)
        {
            int src_ofs = i * 8;
            int dst_ofs = 0xA0 + i * 0x20;
            std::copy(begin(cgb_sparse_FEA0_dump) + src_ofs, begin(cgb_sparse_FEA0_dump) + src_ofs + 8, begin(m_high_ram) + dst_ofs);
            std::copy(begin(cgb_sparse_FEA0_dump) + src_ofs, begin(cgb_sparse_FEA0_dump) + src_ofs + 8, begin(m_high_ram) + dst_ofs + 8);
            std::copy(begin(m_high_ram) + dst_ofs, begin(m_high_ram) + dst_ofs + 0x10, begin(m_high_ram) + dst_ofs + 0x10);
        }
    }
    else
    {
        std::fill(begin(m_high_ram) + 0xA0, begin(m_high_ram) + 0x100, 0);
    }
}





//---------------------------------------------------------
//
//   read byte & write byte
//
//---------------------------------------------------------

age::uint8_t age::gb_bus::read_byte(uint16_t address)
{
    uint8_t result = 0xFF;

    if ((address & 0xE000) == 0x8000)
    {
        //if (m_lcd.is_video_ram_accessible())
        {
            result = m_memory.read_byte(address);
        }
    }
    else if (address < 0xFE00)
    {
        result = m_memory.read_byte(address);
    }
    // 0xFE00 - 0xFE9F : object attribute memory
    else if (address < 0xFEA0)
    {
        //if (m_lcd.is_oam_readable() && !m_oam_dma_active)
        {
            result = m_lcd.get_oam()[address - 0xFE00];
        }
    }
    // 0xFEA0 - 0xFFFF : high ram & IE
    else if ((address & 0x0180) != 0x0100)
    {
        result = (to_integral(gb_io_port::ie) == address) ? m_interrupts.read_ie() : m_high_ram[address - 0xFE00];
    }
    // 0xFF00 - 0xFF7F : i/o ports & wave ram
    else
    {
        // ports & wave ram
        switch (address)
        {
            case to_integral(gb_io_port::p1): result = m_joypad.read_p1(); break;
            case to_integral(gb_io_port::sb): result = m_serial.read_sb(); break;
            case to_integral(gb_io_port::sc): result = m_serial.read_sc(); break;

            case to_integral(gb_io_port::div): result = m_div.read_div(); break;
            case to_integral(gb_io_port::tima): result = m_timer.read_tima(); break;
            case to_integral(gb_io_port::tma): result = m_timer.read_tma(); break;
            case to_integral(gb_io_port::tac): result = m_timer.read_tac(); break;

            case to_integral(gb_io_port::if_): result = m_interrupts.read_if(); break;

            case to_integral(gb_io_port::nr10): result = m_sound.read_nr10(); break;
            case to_integral(gb_io_port::nr11): result = m_sound.read_nr11(); break;
            case to_integral(gb_io_port::nr12): result = m_sound.read_nr12(); break;
            case to_integral(gb_io_port::nr13): result = m_sound.read_nr13(); break;
            case to_integral(gb_io_port::nr14): result = m_sound.read_nr14(); break;
            case to_integral(gb_io_port::nr21): result = m_sound.read_nr21(); break;
            case to_integral(gb_io_port::nr22): result = m_sound.read_nr22(); break;
            case to_integral(gb_io_port::nr23): result = m_sound.read_nr23(); break;
            case to_integral(gb_io_port::nr24): result = m_sound.read_nr24(); break;
            case to_integral(gb_io_port::nr30): result = m_sound.read_nr30(); break;
            case to_integral(gb_io_port::nr31): result = m_sound.read_nr31(); break;
            case to_integral(gb_io_port::nr32): result = m_sound.read_nr32(); break;
            case to_integral(gb_io_port::nr33): result = m_sound.read_nr33(); break;
            case to_integral(gb_io_port::nr34): result = m_sound.read_nr34(); break;
            case to_integral(gb_io_port::nr41): result = m_sound.read_nr41(); break;
            case to_integral(gb_io_port::nr42): result = m_sound.read_nr42(); break;
            case to_integral(gb_io_port::nr43): result = m_sound.read_nr43(); break;
            case to_integral(gb_io_port::nr44): result = m_sound.read_nr44(); break;
            case to_integral(gb_io_port::nr50): result = m_sound.read_nr50(); break;
            case to_integral(gb_io_port::nr51): result = m_sound.read_nr51(); break;
            case to_integral(gb_io_port::nr52): result = m_sound.read_nr52(); break;

            case 0xFF30:
            case 0xFF31:
            case 0xFF32:
            case 0xFF33:
            case 0xFF34:
            case 0xFF35:
            case 0xFF36:
            case 0xFF37:
            case 0xFF38:
            case 0xFF39:
            case 0xFF3A:
            case 0xFF3B:
            case 0xFF3C:
            case 0xFF3D:
            case 0xFF3E:
            case 0xFF3F:
                result = m_sound.read_wave_ram(address - 0xFF30);
                break;

            case to_integral(gb_io_port::lcdc): result = m_lcd.read_lcdc(); break;
            case to_integral(gb_io_port::stat): result = m_lcd.read_stat(); break;
            case to_integral(gb_io_port::scy): result = m_lcd.read_scy(); break;
            case to_integral(gb_io_port::scx): result = m_lcd.read_scx(); break;
            case to_integral(gb_io_port::ly): result = m_lcd.read_ly(); break;
            case to_integral(gb_io_port::lyc): result = m_lcd.read_lyc(); break;
            case to_integral(gb_io_port::dma): result = m_oam_dma_byte; break;
            case to_integral(gb_io_port::bgp): result = m_lcd.read_bgp(); break;
            case to_integral(gb_io_port::obp0): result = m_lcd.read_obp0(); break;
            case to_integral(gb_io_port::obp1): result = m_lcd.read_obp1(); break;
            case to_integral(gb_io_port::wy): result = m_lcd.read_wy(); break;
            case to_integral(gb_io_port::wx): result = m_lcd.read_wx(); break;

            case to_integral(gb_io_port::ie): result = m_interrupts.read_ie(); break;
        }

        // CGB ports
        if (m_device.is_cgb())
        {
            switch (address)
            {
                case to_integral(gb_io_port::key1): result = m_clock.read_key1(); break;
                case to_integral(gb_io_port::vbk): result = m_memory.read_vbk(); break;
                case to_integral(gb_io_port::hdma5): result = m_hdma5; break;
                case to_integral(gb_io_port::rp): result = m_rp; break;
                case to_integral(gb_io_port::bcps): result = m_lcd.read_bcps(); break;
                case to_integral(gb_io_port::bcpd): result = m_lcd.read_bcpd(); break;
                case to_integral(gb_io_port::ocps): result = m_lcd.read_ocps(); break;
                case to_integral(gb_io_port::ocpd): result = m_lcd.read_ocpd(); break;
                case to_integral(gb_io_port::un6c): result = m_un6c; break;
                case to_integral(gb_io_port::svbk): result = m_memory.read_svbk(); break;
                case to_integral(gb_io_port::un72): result = m_un72; break;
                case to_integral(gb_io_port::un73): result = m_un73; break;
                case to_integral(gb_io_port::un75): result = m_un75; break;
                case to_integral(gb_io_port::un76): result = 0; break;
                case to_integral(gb_io_port::un77): result = 0; break;
            }
        }

        // CGB ports when running a DMG rom
        else if (m_device.is_cgb_hardware())
        {
            switch (address)
            {
                case to_integral(gb_io_port::vbk):  result = 0xFE; break;
                case to_integral(gb_io_port::bcps): result = 0xC8; break;
                case to_integral(gb_io_port::ocps): result = 0xD0; break;
                case to_integral(gb_io_port::un72): result = m_un72; break;
                case to_integral(gb_io_port::un73): result = m_un73; break;
                case to_integral(gb_io_port::un75): result = m_un75; break;
                case to_integral(gb_io_port::un76): result = 0; break;
                case to_integral(gb_io_port::un77): result = 0; break;
            }
        }
    }

    return result;
}



void age::gb_bus::write_byte(uint16_t address, uint8_t byte)
{
    if ((address & 0xE000) == 0x8000)
    {
        //if (m_lcd.is_video_ram_accessible())
        {
            m_memory.write_byte(address, byte);
        }
    }
    else if (address < 0xFE00)
    {
        m_memory.write_byte(address, byte);
    }
    // 0xFE00 - 0xFE9F : object attribute memory
    else if (address < 0xFEA0)
    {
        //if (m_lcd.is_oam_writable() && !m_oam_dma_active)
        {
            m_lcd.get_oam()[address - 0xFE00] = byte;
        }
    }
    // 0xFEA0 - 0xFFFF : high ram & IE
    else if ((address & 0x0180) != 0x0100)
    {
        if (to_integral(gb_io_port::ie) == address)
        {
            m_interrupts.write_ie(byte);
        }
        else
        {
            m_high_ram[address - 0xFE00] = byte;
        }
    }
    // 0xFF00 - 0xFF7F : i/o ports & wave ram
    else
    {
        switch (address)
        {
            case to_integral(gb_io_port::p1): m_joypad.write_p1(byte); break;
            case to_integral(gb_io_port::sb): m_serial.write_sb(byte); break;
            case to_integral(gb_io_port::sc): m_serial.write_sc(byte); break;

            case to_integral(gb_io_port::div):
            {
                int old_div_offset = m_div.write_div();
                m_serial.on_div_reset(old_div_offset);
                m_timer.on_div_reset(old_div_offset);
            }
            break;

            case to_integral(gb_io_port::tima): m_timer.write_tima(byte); break;
            case to_integral(gb_io_port::tma): m_timer.write_tma(byte); break;
            case to_integral(gb_io_port::tac): m_timer.write_tac(byte); break;

            case to_integral(gb_io_port::if_): m_interrupts.write_if(byte); break;

            case to_integral(gb_io_port::nr10): m_sound.write_nr10(byte); break;
            case to_integral(gb_io_port::nr11): m_sound.write_nr11(byte); break;
            case to_integral(gb_io_port::nr12): m_sound.write_nr12(byte); break;
            case to_integral(gb_io_port::nr13): m_sound.write_nr13(byte); break;
            case to_integral(gb_io_port::nr14): m_sound.write_nr14(byte); break;
            case to_integral(gb_io_port::nr21): m_sound.write_nr21(byte); break;
            case to_integral(gb_io_port::nr22): m_sound.write_nr22(byte); break;
            case to_integral(gb_io_port::nr23): m_sound.write_nr23(byte); break;
            case to_integral(gb_io_port::nr24): m_sound.write_nr24(byte); break;
            case to_integral(gb_io_port::nr30): m_sound.write_nr30(byte); break;
            case to_integral(gb_io_port::nr31): m_sound.write_nr31(byte); break;
            case to_integral(gb_io_port::nr32): m_sound.write_nr32(byte); break;
            case to_integral(gb_io_port::nr33): m_sound.write_nr33(byte); break;
            case to_integral(gb_io_port::nr34): m_sound.write_nr34(byte); break;
            case to_integral(gb_io_port::nr41): m_sound.write_nr41(byte); break;
            case to_integral(gb_io_port::nr42): m_sound.write_nr42(byte); break;
            case to_integral(gb_io_port::nr43): m_sound.write_nr43(byte); break;
            case to_integral(gb_io_port::nr44): m_sound.write_nr44(byte); break;
            case to_integral(gb_io_port::nr50): m_sound.write_nr50(byte); break;
            case to_integral(gb_io_port::nr51): m_sound.write_nr51(byte); break;
            case to_integral(gb_io_port::nr52): m_sound.write_nr52(byte); break;

            case 0xFF30:
            case 0xFF31:
            case 0xFF32:
            case 0xFF33:
            case 0xFF34:
            case 0xFF35:
            case 0xFF36:
            case 0xFF37:
            case 0xFF38:
            case 0xFF39:
            case 0xFF3A:
            case 0xFF3B:
            case 0xFF3C:
            case 0xFF3D:
            case 0xFF3E:
            case 0xFF3F:
                m_sound.write_wave_ram(address - 0xFF30, byte);
                break;

            case to_integral(gb_io_port::lcdc): m_lcd.write_lcdc(byte); break;
            case to_integral(gb_io_port::stat): m_lcd.write_stat(byte); break;
            case to_integral(gb_io_port::scy): m_lcd.write_scy(byte); break;
            case to_integral(gb_io_port::scx): m_lcd.write_scx(byte); break;
            case to_integral(gb_io_port::ly): break; // cannot be written
            case to_integral(gb_io_port::lyc): m_lcd.write_lyc(byte); break;
            case to_integral(gb_io_port::dma): write_dma(byte); break;
            case to_integral(gb_io_port::bgp): m_lcd.write_bgp(byte); break;
            case to_integral(gb_io_port::obp0): m_lcd.write_obp0(byte); break;
            case to_integral(gb_io_port::obp1): m_lcd.write_obp1(byte); break;
            case to_integral(gb_io_port::wy): m_lcd.write_wy(byte); break;
            case to_integral(gb_io_port::wx): m_lcd.write_wx(byte); break;
        }

        // CGB ports
        if (m_device.is_cgb())
        {
            switch (address)
            {
                case to_integral(gb_io_port::key1): m_clock.write_key1(byte); break;
                case to_integral(gb_io_port::vbk): m_memory.write_vbk(byte); break;
                case to_integral(gb_io_port::hdma1): m_dma_source = (m_dma_source & 0xFF) + (byte << 8); break;
                case to_integral(gb_io_port::hdma2): m_dma_source = (m_dma_source & 0xFF00) + (byte & 0xF0); break;
                case to_integral(gb_io_port::hdma3): m_dma_destination = (m_dma_destination & 0xFF) + (byte << 8); break;
                case to_integral(gb_io_port::hdma4): m_dma_destination = (m_dma_destination & 0xFF00) + (byte & 0xF0); break;
                case to_integral(gb_io_port::hdma5): write_hdma5(byte); break;
                case to_integral(gb_io_port::rp): m_rp = byte | 0x3E; break;
                case to_integral(gb_io_port::bcps): m_lcd.write_bcps(byte); break;
                case to_integral(gb_io_port::bcpd): m_lcd.write_bcpd(byte); break;
                case to_integral(gb_io_port::ocps): m_lcd.write_ocps(byte); break;
                case to_integral(gb_io_port::ocpd): m_lcd.write_ocpd(byte); break;
                case to_integral(gb_io_port::un6c): m_un6c = byte | 0xFE; break;
                case to_integral(gb_io_port::svbk): m_memory.write_svbk(byte); break;
                case to_integral(gb_io_port::un72): m_un72 = byte; break;
                case to_integral(gb_io_port::un73): m_un73 = byte; break;
                case to_integral(gb_io_port::un75): m_un75 = byte | 0x8F; break;
            }
        }

        // CGB ports when running a DMG rom
        else if (m_device.is_cgb_hardware())
        {
            switch (address)
            {
                case to_integral(gb_io_port::un72): m_un72 = byte; break;
                case to_integral(gb_io_port::un73): m_un73 = byte; break;
                case to_integral(gb_io_port::un75): m_un75 = byte | 0x8F; break;
            }
        }
    }
}





//---------------------------------------------------------
//
//   event handling
//
//---------------------------------------------------------

void age::gb_bus::handle_events()
{
    // handle outstanding events
    gb_event event;
    while ((event = m_events.poll_event()) != gb_event::none)
    {
        switch (event)
        {
            case gb_event::lcd_interrupt:
                m_lcd.trigger_interrupt();
                break;

            case gb_event::serial_transfer_finished:
                m_serial.update_state();
                break;

            case gb_event::timer_interrupt:
                m_timer.trigger_interrupt();
                break;

            case gb_event::start_hdma:
                m_during_dma = true;
                break;

            case gb_event::start_oam_dma:
                m_oam_dma_last_cycle = m_clock.get_clock_cycle();
                m_oam_dma_active = true;
                //
                // verified by mooneye-gb tests
                //
                // Trying to trigger an OAM DMA transfer for a memory address
                // greater than 0xDFFF will instead trigger an OAM DMA transfer
                // for the corresponding 0xC000-0xDFFF memory range.
                //
                //      acceptance/oam_dma/sources-dmgABCmgbS
                //
                m_oam_dma_address = (m_oam_dma_byte * 0x100) & ((m_oam_dma_byte > 0xDF) ? 0xDF00 : 0xFF00);
                m_oam_dma_offset = 0;
                break;

            default:
                AGE_ASSERT(false);
                break;
        }
    }

    // emulate DMA, if necessary
    if (m_oam_dma_active)
    {
        handle_oam_dma();
    }
}



void age::gb_bus::handle_dma()
{
    // during HDMA/GDMA the CPU is halted, so we just copy
    // the required bytes in one go and increase the
    // clock accordingly

    // calculate remaining DMA length
    uint8_t dma_length = (m_hdma5 & ~gb_hdma_start) + 1;

    // calculate number of bytes to copy and update remaining DMA length
    int bytes = m_hdma_active ? 0x10 : dma_length * 0x10;
    AGE_ASSERT(bytes <= 0x800);
    AGE_ASSERT((bytes & 0xF) == 0);
    LOG("DMA copying " << bytes << " bytes");

    //
    // verified by gambatte tests
    //
    // If the DMA destination wraps around from 0xFFFF to 0x0000
    // during copying, the DMA transfer will be stopped at that
    // point.
    //
    //      dma/dma_dst_wrap_1_out1
    //      dma/dma_dst_wrap_2_out0
    //
    if ((m_dma_destination + bytes) > 0xFFFF)
    {
        bytes = 0x10000 - m_dma_destination;
    }
    AGE_ASSERT(bytes <= 0x800);
    AGE_ASSERT((bytes & 0xF) == 0);

    //
    // verified by gambatte tests
    //
    // DMA transfer takes 2 cycles per transferred byte and an
    // additional 4 cycles (2 cycles if running at double speed).
    //
    //      dma/gdma_cycles_long_1_out3
    //      dma/gdma_cycles_long_2_out0
    //      dma/gdma_cycles_long_ds_1_out3
    //      dma/gdma_cycles_long_ds_2_out0
    //      dma/gdma_weird_1_out3
    //      dma/gdma_weird_2_out0
    //

    // copy bytes
    for (int i = 0; i < bytes; ++i)
    {
        uint8_t byte = 0xFF;
        uint16_t src = m_dma_source & 0xFFFF;
        if (((src & 0xE000) != 0x8000) && (src < 0xFE00))
        {
            byte = read_byte(src);
        }

        uint16_t dest = 0x8000 + (m_dma_destination & 0x1FFF);
        handle_events();
        write_byte(dest, byte);
        m_clock.tick_2_clock_cycles();

        ++m_dma_source;
        ++m_dma_destination;
    }

    m_clock.tick_machine_cycle();

    // update HDMA5
    uint8_t remaining_dma_length = (dma_length - 1 - (bytes >> 4)) & 0x7F;
    if (remaining_dma_length == 0x7F)
    {
        LOG("DMA finished");
        m_hdma_active = false;
        AGE_ASSERT(m_events.get_event_cycle(gb_event::start_hdma) == gb_no_clock_cycle);
    }
    m_hdma5 = (m_hdma5 & gb_hdma_start) + remaining_dma_length;

    AGE_ASSERT((m_dma_source & 0xF) == 0);
    AGE_ASSERT((m_dma_destination & 0xF) == 0);
    m_during_dma = false;
}



bool age::gb_bus::during_dma() const
{
    return m_during_dma;
}



void age::gb_bus::set_back_clock(int clock_cycle_offset)
{
    AGE_GB_SET_BACK_CLOCK(m_oam_dma_last_cycle, clock_cycle_offset);
}





//---------------------------------------------------------
//
//   Private methods
//
//---------------------------------------------------------

void age::gb_bus::write_dma(uint8_t value)
{
    //
    // verified by mooneye-gb tests
    //
    // Reading the DMA register will always return the last value
    // written to it even if it did not trigger any DMA transfer.
    //
    //      acceptance/oam_dma/reg_read
    //
    m_oam_dma_byte = value;

    //
    // verified by mooneye-gb tests
    //
    // OAM DMA starts after the next cpu cycle finishes
    //
    //      acceptance/oam_dma_start
    //
    // verified by gambatte tests
    //
    // there is no delay for CGB, the OAM DMA starts after
    // the current cpu cycle finishes
    //
    //      oamdma/oamdma_src8000_vrambankchange_1_cgb04c_out0
    //      oamdma/oamdma_src8000_vrambankchange_2_cgb04c_out4
    //      oamdma/oamdma_src8000_vrambankchange_3_cgb04c_out0
    //      oamdma/oamdma_src8000_vrambankchange_4_cgb04c_out3
    //
    int factor = m_device.is_cgb() ? 1 : 2;
    m_events.schedule_event(gb_event::start_oam_dma, m_clock.get_machine_cycle_clocks() * factor);
}

void age::gb_bus::write_hdma5(uint8_t value)
{
    m_hdma5 = value & 0x7F; // store DMA data length

    if ((value & gb_hdma_start) > 0)
    {
        m_hdma_active = true;
        m_hdma5 |= gb_hdma_start;
        LOG("HDMA activated");
    }
    else
    {
        // HDMA not running: start GDMA
        if (!m_hdma_active)
        {
            m_during_dma = true;
            LOG("GDMA activated");
        }
        // HDMA running: stop it
        else
        {
            m_hdma_active = false;

            //
            // verified by gambatte tests
            //
            // An upcoming HDMA can only be aborted, if it does
            // not start on the current cycle.
            //
            //      dma/hdma_late_disable_1_out0
            //      dma/hdma_late_disable_2_out1
            //      dma/hdma_late_disable_ds_1_out0
            //      dma/hdma_late_disable_ds_2_out1
            //      dma/hdma_late_disable_scx2_1_out0
            //      dma/hdma_late_disable_scx2_2_out1
            //      dma/hdma_late_disable_scx3_1_out0
            //      dma/hdma_late_disable_scx3_2_out1
            //      dma/hdma_late_disable_scx5_1_out0
            //      dma/hdma_late_disable_scx5_2_out1
            //      dma/hdma_late_disable_scx5_ds_1_out0
            //      dma/hdma_late_disable_scx5_ds_2_out1
            //
            if (m_events.get_event_cycle(gb_event::start_hdma) > m_clock.get_clock_cycle())
            {
                m_events.remove_event(gb_event::start_hdma);
            }
            LOG("HDMA deactivated");
        }
    }

    LOG("HDMA5 = " << AGE_LOG_DEC(m_hdma5));
}



void age::gb_bus::handle_oam_dma()
{
    AGE_ASSERT(m_oam_dma_active);

    int current_clk = m_clock.get_clock_cycle();
    int cycles_elapsed = current_clk - m_oam_dma_last_cycle;
    cycles_elapsed &= ~(m_clock.get_machine_cycle_clocks() - 1);
    m_oam_dma_last_cycle += cycles_elapsed;
    cycles_elapsed <<= m_clock.is_double_speed() ? 1 : 0;

    AGE_ASSERT((cycles_elapsed & 3) == 0);
    int bytes = std::min(cycles_elapsed / 4, 160 - m_oam_dma_offset);

    for (int i = m_oam_dma_offset, max = m_oam_dma_offset + bytes; i < max; ++i)
    {
        uint8_t byte = read_byte((m_oam_dma_address + i) & 0xFFFF);
        m_lcd.get_oam()[i & 0xFFFF] = byte;
    }

    m_oam_dma_offset += bytes;
    AGE_ASSERT(m_oam_dma_offset <= 160);
    if (m_oam_dma_offset >= 160)
    {
        m_oam_dma_active = false;
        m_oam_dma_last_cycle = gb_no_clock_cycle;
    }
}
