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

#include "age_gb_bus.hpp"

#include <algorithm>
#include <cassert>
#include <utility>



namespace
{

    constexpr age::uint8_t gb_hdma_start = 0x80;

    // memory dumps,
    // based on *.bin files used by gambatte tests and gambatte source code (initstate.cpp)

    constexpr const age::uint8_array<0x60> cgb_sparse_FEA0_dump
        = {{// every line used four times:
            // 0x08, 0x01, 0xEF, 0xDE, ...
            // 0x08, 0x01, 0xEF, 0xDE, ...
            // ...
            0x08, 0x01, 0xEF, 0xDE, 0x06, 0x4A, 0xCD, 0xBD,
            0x00, 0x90, 0xF7, 0x7F, 0xC0, 0xB1, 0xBC, 0xFB,
            0x24, 0x13, 0xFD, 0x3A, 0x10, 0x10, 0xAD, 0x45}};

    constexpr const age::uint8_array<0x80> dmg_FF80_dump
        = {{0x2B, 0x0B, 0x64, 0x2F, 0xAF, 0x15, 0x60, 0x6D, 0x61, 0x4E, 0xAC, 0x45, 0x0F, 0xDA, 0x92, 0xF3,
            0x83, 0x38, 0xE4, 0x4E, 0xA7, 0x6C, 0x38, 0x58, 0xBE, 0xEA, 0xE5, 0x81, 0xB4, 0xCB, 0xBF, 0x7B,
            0x59, 0xAD, 0x50, 0x13, 0x5E, 0xF6, 0xB3, 0xC1, 0xDC, 0xDF, 0x9E, 0x68, 0xD7, 0x59, 0x26, 0xF3,
            0x62, 0x54, 0xF8, 0x36, 0xB7, 0x78, 0x6A, 0x22, 0xA7, 0xDD, 0x88, 0x15, 0xCA, 0x96, 0x39, 0xD3,
            0xE6, 0x55, 0x6E, 0xEA, 0x90, 0x76, 0xB8, 0xFF, 0x50, 0xCD, 0xB5, 0x1B, 0x1F, 0xA5, 0x4D, 0x2E,
            0xB4, 0x09, 0x47, 0x8A, 0xC4, 0x5A, 0x8C, 0x4E, 0xE7, 0x29, 0x50, 0x88, 0xA8, 0x66, 0x85, 0x4B,
            0xAA, 0x38, 0xE7, 0x6B, 0x45, 0x3E, 0x30, 0x37, 0xBA, 0xC5, 0x31, 0xF2, 0x71, 0xB4, 0xCF, 0x29,
            0xBC, 0x7F, 0x7E, 0xD0, 0xC7, 0xC3, 0xBD, 0xCF, 0x59, 0xEA, 0x39, 0x01, 0x2E, 0x00, 0x69, 0x00}};

    constexpr const age::uint8_array<0x80> cgb_FF80_dump
        = {{0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
            0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
            0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
            0x45, 0xEC, 0x42, 0xFA, 0x08, 0xB7, 0x07, 0x5D, 0x01, 0xF5, 0xC0, 0xFF, 0x08, 0xFC, 0x00, 0xE5,
            0x0B, 0xF8, 0xC2, 0xCA, 0xF4, 0xF9, 0x0D, 0x7F, 0x44, 0x6D, 0x19, 0xFE, 0x46, 0x97, 0x33, 0x5E,
            0x08, 0xFF, 0xD1, 0xFF, 0xC6, 0x8B, 0x24, 0x74, 0x12, 0xFC, 0x00, 0x9F, 0x94, 0xB7, 0x06, 0xD5,
            0x40, 0x7A, 0x20, 0x9E, 0x04, 0x5F, 0x41, 0x2F, 0x3D, 0x77, 0x36, 0x75, 0x81, 0x8A, 0x70, 0x3A,
            0x98, 0xD1, 0x71, 0x02, 0x4D, 0x01, 0xC1, 0xFF, 0x0D, 0x00, 0xD3, 0x05, 0xF9, 0x00, 0x0B, 0x00}};

} // namespace



//---------------------------------------------------------
//
//   Object creation.
//
//---------------------------------------------------------

age::gb_bus::gb_bus(const gb_device&         device,
                    gb_clock&                clock,
                    gb_interrupt_dispatcher& interrupts,
                    gb_events&               events,
                    gb_memory&               memory,
                    gb_sound&                sound,
                    gb_lcd&                  lcd,
                    gb_timer&                timer,
                    gb_joypad&               joypad,
                    gb_serial&               serial)
    : m_device(device),
      m_clock(clock),
      m_interrupts(interrupts),
      m_events(events),
      m_memory(memory),
      m_sound(sound),
      m_lcd(lcd),
      m_timer(timer),
      m_joypad(joypad),
      m_serial(serial),
      m_oam_dma(device, clock, memory, events, lcd)
{
    // clear high ram
    std::fill(begin(m_high_ram), end(m_high_ram), 0);

    // init 0xFF80 - 0xFFFE
    const uint8_array<0x80>& src = m_device.is_cgb_device() ? cgb_FF80_dump : dmg_FF80_dump;
    std::copy(begin(src), end(src), begin(m_high_ram) + 0x180);

    // init 0xFEA0 - 0xFEFF
    if (m_device.is_cgb_device())
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
    // handle pending events to keep the system consistent
    // before writing any value
    //! \todo this might be optimized after OAM DMA is fully understood & implemented
    handle_events();

    if (address < 0xFE00)
    {
        auto conflicting_read = m_oam_dma.conflicting_read(address);
        if (conflicting_read >= 0)
        {
            return conflicting_read;
        }
        if (is_video_ram(address) && !m_lcd.is_video_ram_accessible())
        {
            m_clock.log(gb_log_category::lc_lcd_vram) << "read VRAM [" << log_hex16(address)
                                                      << "] == 0xFF (VRAM not accessible)";
            return 0xFF;
        }
        return m_memory.read_byte(address);
    }

    // 0xFE00 - 0xFE9F : object attribute memory
    if (address < 0xFEA0)
    {
        int oam_offset = address - 0xFE00;
        if (m_oam_dma.dma_active())
        {
            m_clock.log(gb_log_category::lc_lcd_oam) << "read OAM [" << log_hex16(address)
                                                     << "] == 0xFF (OAM DMA running)";
            return 0xFF;
        }
        return m_lcd.read_oam(oam_offset);
    }

    // 0xFEA0 - 0xFFFF : prohibited, high ram & IE
    if ((address & 0x0180) != 0x0100)
    {
        // prohibited area
        if (address < 0xFF00)
        {
            if (m_oam_dma.dma_active())
            {
                m_clock.log(gb_log_category::lc_memory) << "read [" << log_hex16(address)
                                                        << "] == 0xFF (OAM DMA running)";
                return 0xFF;
            }
            uint8_t result = !m_device.is_cgb_device()    ? 0
                             : m_device.is_cgb_e_device() ? ((address & 0xF0) + ((address & 0xF0) >> 4))
                                                          : m_high_ram[address - 0xFE00];

            m_clock.log(gb_log_category::lc_memory) << "read [" << log_hex16(address)
                                                    << "] == " << log_hex8(result);
            return result;
        }
        // high ram & ie
        return (std::to_underlying(gb_register::ie) == address) ? m_interrupts.read_ie() : m_high_ram[address - 0xFE00];
    }

    // 0xFF00 - 0xFF7F : registers & wave ram
    switch (address)
    {
        case std::to_underlying(gb_register::p1): return m_joypad.read_p1();
        case std::to_underlying(gb_register::sb): return m_serial.read_sb();
        case std::to_underlying(gb_register::sc): return m_serial.read_sc();
        case std::to_underlying(gb_register::div): return m_clock.read_div();
        case std::to_underlying(gb_register::tima): return m_timer.read_tima();
        case std::to_underlying(gb_register::tma): return m_timer.read_tma();
        case std::to_underlying(gb_register::tac): return m_timer.read_tac();
        case std::to_underlying(gb_register::if_): return m_interrupts.read_if();

        case std::to_underlying(gb_register::nr10): return m_sound.read_nr10();
        case std::to_underlying(gb_register::nr11): return m_sound.read_nr11();
        case std::to_underlying(gb_register::nr12): return m_sound.read_nr12();
        case std::to_underlying(gb_register::nr14): return m_sound.read_nr14();
        case std::to_underlying(gb_register::nr21): return m_sound.read_nr21();
        case std::to_underlying(gb_register::nr22): return m_sound.read_nr22();
        case std::to_underlying(gb_register::nr24): return m_sound.read_nr24();
        case std::to_underlying(gb_register::nr30): return m_sound.read_nr30();
        case std::to_underlying(gb_register::nr32): return m_sound.read_nr32();
        case std::to_underlying(gb_register::nr34): return m_sound.read_nr34();
        case std::to_underlying(gb_register::nr42): return m_sound.read_nr42();
        case std::to_underlying(gb_register::nr43): return m_sound.read_nr43();
        case std::to_underlying(gb_register::nr44): return m_sound.read_nr44();
        case std::to_underlying(gb_register::nr50): return m_sound.read_nr50();
        case std::to_underlying(gb_register::nr51): return m_sound.read_nr51();
        case std::to_underlying(gb_register::nr52): return m_sound.read_nr52();
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
            return m_sound.read_wave_ram(address - 0xFF30);

        case std::to_underlying(gb_register::lcdc): return m_lcd.read_lcdc();
        case std::to_underlying(gb_register::stat): return m_lcd.read_stat();
        case std::to_underlying(gb_register::scy): return m_lcd.read_scy();
        case std::to_underlying(gb_register::scx): return m_lcd.read_scx();
        case std::to_underlying(gb_register::ly): return m_lcd.read_ly();
        case std::to_underlying(gb_register::lyc): return m_lcd.read_lyc();
        case std::to_underlying(gb_register::dma): return m_oam_dma.read_dma_reg();
        case std::to_underlying(gb_register::bgp): return m_lcd.read_bgp();
        case std::to_underlying(gb_register::obp0): return m_lcd.read_obp0();
        case std::to_underlying(gb_register::obp1): return m_lcd.read_obp1();
        case std::to_underlying(gb_register::wy): return m_lcd.read_wy();
        case std::to_underlying(gb_register::wx): return m_lcd.read_wx();
        case std::to_underlying(gb_register::ie): return m_interrupts.read_ie();
        default: break;
    }

    // CGB registers
    if (m_device.cgb_mode())
    {
        switch (address)
        {
            case std::to_underlying(gb_register::key1): return m_clock.read_key1();
            case std::to_underlying(gb_register::vbk): return m_memory.read_vbk();
            case std::to_underlying(gb_register::hdma5): return m_hdma5;
            case std::to_underlying(gb_register::rp): return m_rp;
            case std::to_underlying(gb_register::bcps): return m_lcd.read_bcps();
            case std::to_underlying(gb_register::bcpd): return m_lcd.read_bcpd();
            case std::to_underlying(gb_register::ocps): return m_lcd.read_ocps();
            case std::to_underlying(gb_register::ocpd): return m_lcd.read_ocpd();
            case std::to_underlying(gb_register::un6c): return m_un6c;
            case std::to_underlying(gb_register::svbk): return m_memory.read_svbk();
            case std::to_underlying(gb_register::un72): return m_un72;
            case std::to_underlying(gb_register::un73): return m_un73;
            case std::to_underlying(gb_register::un75): return m_un75;
            case std::to_underlying(gb_register::pcm12): return m_sound.read_pcm12();
            case std::to_underlying(gb_register::pcm34): return m_sound.read_pcm34();
            default: break;
        }
    }

    // CGB registers when running in non-CGB mode
    if (m_device.non_cgb_mode())
    {
        switch (address)
        {
            case std::to_underlying(gb_register::vbk): return 0xFE;
            case std::to_underlying(gb_register::bcps): return 0xC8;
            case std::to_underlying(gb_register::ocps): return 0xD0;
            case std::to_underlying(gb_register::un72): return m_un72;
            case std::to_underlying(gb_register::un73): return m_un73;
            case std::to_underlying(gb_register::un75): return m_un75;
            case std::to_underlying(gb_register::pcm12): return m_sound.read_pcm12();
            case std::to_underlying(gb_register::pcm34): return m_sound.read_pcm34();
            default: break;
        }
    }

    return 0xFF;
}



void age::gb_bus::write_byte(uint16_t address, uint8_t byte)
{
    // handle pending events to keep the system consistent
    // before writing any value
    //! \todo this might be optimized after OAM DMA is fully understood & implemented
    handle_events();

    if (address < 0xFE00)
    {
        if (m_oam_dma.conflicting_write(address, byte))
        {
            return;
        }
        if (is_video_ram(address))
        {
            if (!m_lcd.is_video_ram_accessible())
            {
                m_clock.log(gb_log_category::lc_lcd_vram)
                    << "write VRAM [" << log_hex16(address) << "] = " << log_hex8(byte)
                    << " ignored (VRAM not accessible)";
                return;
            }
            m_lcd.update_state();
        }
        m_memory.write_byte(address, byte);
        return;
    }

    // 0xFE00 - 0xFE9F : object attribute memory
    if (address < 0xFEA0)
    {
        int oam_offset = address - 0xFE00;
        if (m_oam_dma.dma_active())
        {
            m_clock.log(gb_log_category::lc_lcd_oam) << "write OAM [" << log_hex16(address)
                                                     << "] = " << log_hex8(byte) << " ignored (OAM DMA running)";
            return;
        }
        m_lcd.write_oam(oam_offset, byte);
        return;
    }

    // 0xFEA0 - 0xFFFF : high ram & IE
    if ((address & 0x0180) != 0x0100)
    {
        if (address < 0xFF00)
        {
            // 0xFEA0 - 0xFEFF not writable during OAM DMA:
            //      (gambatte) oamdma/oamdma_src0000_busypushFEA1_dmg08_out65768700_cgb04c_out65768734
            //      (gambatte) oamdma/oamdma_src0000_busypushFF01_dmg08_out657600DF_cgb04c_out657612DF
            //      (gambatte) oamdma/oamdma_src8000_busypushFEA1_dmg08_out65768700_cgb04c_out65768734
            //      (gambatte) oamdma/oamdma_src8000_busypushFF01_dmg08_out657600DF_cgb04c_out657612DF
            //
            if (m_oam_dma.dma_active())
            {
                m_clock.log(gb_log_category::lc_memory) << "write [" << log_hex16(address)
                                                        << "] = " << log_hex8(byte) << " ignored (OAM DMA running)";
                return;
            }
        }
        if (std::to_underlying(gb_register::ie) == address)
        {
            m_interrupts.write_ie(byte);
        }
        else
        {
            m_high_ram[address - 0xFE00] = byte;
        }
        return;
    }

    // 0xFF00 - 0xFF7F : registers & wave ram
    switch (address)
    {
        case std::to_underlying(gb_register::p1): m_joypad.write_p1(byte); return;
        case std::to_underlying(gb_register::sb): m_serial.write_sb(byte); return;
        case std::to_underlying(gb_register::sc): m_serial.write_sc(byte); return;
        case std::to_underlying(gb_register::div): reset_div(false); return;
        case std::to_underlying(gb_register::tima): m_timer.write_tima(byte); return;
        case std::to_underlying(gb_register::tma): m_timer.write_tma(byte); return;
        case std::to_underlying(gb_register::tac): m_timer.write_tac(byte); return;
        case std::to_underlying(gb_register::if_): m_interrupts.write_if(byte); return;

        case std::to_underlying(gb_register::nr10): m_sound.write_nr10(byte); return;
        case std::to_underlying(gb_register::nr11): m_sound.write_nr11(byte); return;
        case std::to_underlying(gb_register::nr12): m_sound.write_nr12(byte); return;
        case std::to_underlying(gb_register::nr13): m_sound.write_nr13(byte); return;
        case std::to_underlying(gb_register::nr14): m_sound.write_nr14(byte); return;
        case std::to_underlying(gb_register::nr21): m_sound.write_nr21(byte); return;
        case std::to_underlying(gb_register::nr22): m_sound.write_nr22(byte); return;
        case std::to_underlying(gb_register::nr23): m_sound.write_nr23(byte); return;
        case std::to_underlying(gb_register::nr24): m_sound.write_nr24(byte); return;
        case std::to_underlying(gb_register::nr30): m_sound.write_nr30(byte); return;
        case std::to_underlying(gb_register::nr31): m_sound.write_nr31(byte); return;
        case std::to_underlying(gb_register::nr32): m_sound.write_nr32(byte); return;
        case std::to_underlying(gb_register::nr33): m_sound.write_nr33(byte); return;
        case std::to_underlying(gb_register::nr34): m_sound.write_nr34(byte); return;
        case std::to_underlying(gb_register::nr41): m_sound.write_nr41(byte); return;
        case std::to_underlying(gb_register::nr42): m_sound.write_nr42(byte); return;
        case std::to_underlying(gb_register::nr43): m_sound.write_nr43(byte); return;
        case std::to_underlying(gb_register::nr44): m_sound.write_nr44(byte); return;
        case std::to_underlying(gb_register::nr50): m_sound.write_nr50(byte); return;
        case std::to_underlying(gb_register::nr51): m_sound.write_nr51(byte); return;
        case std::to_underlying(gb_register::nr52): m_sound.write_nr52(byte); return;
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
            return;

        case std::to_underlying(gb_register::lcdc): m_lcd.write_lcdc(byte); return;
        case std::to_underlying(gb_register::stat): m_lcd.write_stat(byte); return;
        case std::to_underlying(gb_register::scy): m_lcd.write_scy(byte); return;
        case std::to_underlying(gb_register::scx): m_lcd.write_scx(byte); return;
        case std::to_underlying(gb_register::ly): return; // cannot be written
        case std::to_underlying(gb_register::lyc): m_lcd.write_lyc(byte); return;
        case std::to_underlying(gb_register::dma): m_oam_dma.write_dma_reg(byte); return;
        case std::to_underlying(gb_register::bgp): m_lcd.write_bgp(byte); return;
        case std::to_underlying(gb_register::obp0): m_lcd.write_obp0(byte); return;
        case std::to_underlying(gb_register::obp1): m_lcd.write_obp1(byte); return;
        case std::to_underlying(gb_register::wy): m_lcd.write_wy(byte); return;
        case std::to_underlying(gb_register::wx): m_lcd.write_wx(byte); return;
        default: break;
    }

    // CGB registers
    if (m_device.cgb_mode())
    {
        switch (address)
        {
            case std::to_underlying(gb_register::key1): m_clock.write_key1(byte); return;
            case std::to_underlying(gb_register::vbk): m_memory.write_vbk(byte); return;
            case std::to_underlying(gb_register::hdma1): m_hdma_source = (m_hdma_source & 0xFF) + (byte << 8); return;
            case std::to_underlying(gb_register::hdma2): m_hdma_source = (m_hdma_source & 0xFF00) + (byte & 0xF0); return;
            case std::to_underlying(gb_register::hdma3): m_hdma_destination = (m_hdma_destination & 0xFF) + (byte << 8); return;
            case std::to_underlying(gb_register::hdma4): m_hdma_destination = (m_hdma_destination & 0xFF00) + (byte & 0xF0); return;
            case std::to_underlying(gb_register::hdma5): write_hdma5(byte); return;
            case std::to_underlying(gb_register::rp): m_rp = byte | 0x3E; return;
            case std::to_underlying(gb_register::bcps): m_lcd.write_bcps(byte); return;
            case std::to_underlying(gb_register::bcpd): m_lcd.write_bcpd(byte); return;
            case std::to_underlying(gb_register::ocps): m_lcd.write_ocps(byte); return;
            case std::to_underlying(gb_register::ocpd): m_lcd.write_ocpd(byte); return;
            case std::to_underlying(gb_register::un6c): m_un6c = byte | 0xFE; return;
            case std::to_underlying(gb_register::svbk): m_memory.write_svbk(byte); return;
            case std::to_underlying(gb_register::un72): m_un72 = byte; return;
            case std::to_underlying(gb_register::un73): m_un73 = byte; return;
            case std::to_underlying(gb_register::un75): m_un75 = byte | 0x8F; return;
            default: break;
        }
    }

    // CGB registers when running in non-CGB mode
    if (m_device.non_cgb_mode())
    {
        switch (address)
        {
            case std::to_underlying(gb_register::un72): m_un72 = byte; return;
            case std::to_underlying(gb_register::un73): m_un73 = byte; return;
            case std::to_underlying(gb_register::un75): m_un75 = byte | 0x8F; return;
            default: break;
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
    gb_event event{};
    while ((event = m_events.poll_next_event()) != gb_event::none)
    {
        switch (event)
        {
            case gb_event::lcd_interrupt_vblank:
                m_lcd.trigger_irq_vblank();
                break;

            case gb_event::lcd_interrupt_lyc:
                m_lcd.trigger_irq_lyc();
                break;

            case gb_event::lcd_interrupt_mode2:
                m_lcd.trigger_irq_mode2();
                break;

            case gb_event::lcd_interrupt_mode0:
                m_lcd.trigger_irq_mode0();
                break;

            case gb_event::serial_transfer_finished:
                m_serial.update_state(); // triggers serial i/o interrupt
                break;

            case gb_event::timer_interrupt:
                m_timer.trigger_interrupt();
                break;

            case gb_event::unhalt:
                m_interrupts.unhalt();
                break;

            case gb_event::next_empty_frame:
                m_lcd.next_empty_frame();
                break;

            case gb_event::start_oam_dma:
                m_oam_dma.handle_start_dma_event();
                break;

            case gb_event::none:
                break;
        }
    }

    // continue OAM DMA, if active
    m_oam_dma.continue_dma();
}



bool age::gb_bus::handle_gp_dma()
{
    if (!m_gp_dma_active)
    {
        return false;
    }

    // during general purpose DMA the CPU is halted,
    // so we just copy the required bytes all at once
    // and increase the clock accordingly

    // calculate remaining DMA length
    uint8_t dma_length = (m_hdma5 & 0x7F) + 1;

    // calculate number of bytes to copy and update remaining DMA length
    int bytes = dma_length * 0x10;
    assert(bytes > 0);
    assert(bytes <= 0x800);
    assert((bytes % 0x10) == 0);
    auto msg = log_hdma();
    msg << "starting general purpose DMA"
        << "\n    * from " << log_hex16(m_hdma_source) << " to " << log_hex16(m_hdma_destination)
        << "\n    * DMA length == " << log_hex8(m_hdma5 & 0x7F) << " => " << log_hex16(bytes) << " bytes";

    if (m_hdma_source + bytes > 0x10000)
    {
        msg << "\n    * source address will wrap around to 0x0000";
    }

    // If the general purpose DMA destination wraps around
    // from 0xFFFF to 0x0000 during copying,
    // the DMA transfer will be stopped at that point.
    //
    //      (gambatte) dma/dma_dst_wrap_1_out1
    //      (gambatte) dma/dma_dst_wrap_2_out0
    //
    if ((m_hdma_destination + bytes) > 0x10000)
    {
        bytes = 0x10000 - m_hdma_destination;
        msg << "\n    * reducing to " << log_hex16(bytes) << " bytes to prevent destination address from wrapping around to 0x0000";
    }

    // general purpose DMA transfer takes 2 clock cycles per transferred byte
    // and an additional m-cycle at the end.
    //
    //      (gambatte) dma/gdma_cycles_long_1_out3
    //      (gambatte) dma/gdma_cycles_long_2_out0
    //      (gambatte) dma/gdma_cycles_long_ds_1_out3
    //      (gambatte) dma/gdma_cycles_long_ds_2_out0
    //      (gambatte) dma/gdma_weird_1_out3
    //      (gambatte) dma/gdma_weird_2_out0
    //
    for (int i = 0; i < bytes; ++i)
    {
        m_clock.tick_clock_cycles(2);

        uint16_t src = m_hdma_source & 0xFFFF; // may wrap around
        uint16_t dst = 0x8000 + (m_hdma_destination & 0x1FFF);

        uint8_t byte = (((src & 0xE000) != 0x8000) && (src < 0xFE00)) ? read_byte(src) : 0xFF;
        write_byte(dst, byte);

        ++m_hdma_source;
        ++m_hdma_destination;
        assert(m_hdma_destination <= 0x10000);
    }

    m_clock.tick_machine_cycle();

    // update HDMA5
    m_hdma5         = 0xFF;
    m_gp_dma_active = false;

    // create new log entry for "finished" message instead of using msg
    // as reading/writing data may have caused additional logging
    log_hdma() << "general purpose DMA finished";
    return true;
}



void age::gb_bus::execute_stop()
{
    // STOP resets the DIV
    reset_div(true);

    // state update in case of an upcoming speed switch
    m_timer.update_state();

    // check for speed change
    bool speed_changed = m_clock.change_speed();
    if (!speed_changed)
    {
        return;
    }

    // adjust for new speed
    m_lcd.after_speed_change();
    m_sound.after_speed_change();
    m_timer.after_speed_change();
}

void age::gb_bus::set_back_clock(int clock_cycle_offset)
{
    m_oam_dma.set_back_clock(clock_cycle_offset);
}



//---------------------------------------------------------
//
//   Private methods
//
//---------------------------------------------------------

void age::gb_bus::reset_div(bool during_stop)
{
    m_sound.update_state();
    m_timer.update_state();

    m_clock.write_div();

    m_serial.after_div_reset();
    m_sound.after_div_reset(during_stop);
    m_timer.after_div_reset(during_stop);
}

void age::gb_bus::write_hdma5(uint8_t value)
{
    m_hdma5 = value;

    if (!(value & gb_hdma_start))
    {
        // start general purpose DMA after the current CPU instruction
        m_gp_dma_active = true;
        log_hdma() << "starting general purpose DMA after this CPU instruction";
    }
    //! \todo implement h-blank DMA

    //    else
    //    {
    //        // HDMA not running: start general purpose DMA
    //        if (!m_hdma_active)
    //        {
    //            m_during_dma = true;
    //        }
    //        // HDMA running: stop it
    //        else
    //        {
    //            m_hdma_active = false;
    //
    //            //
    //            // verified by gambatte tests
    //            //
    //            // An upcoming HDMA can only be aborted, if it does
    //            // not start on the current cycle.
    //            //
    //            //      dma/hdma_late_disable_1_out0
    //            //      dma/hdma_late_disable_2_out1
    //            //      dma/hdma_late_disable_ds_1_out0
    //            //      dma/hdma_late_disable_ds_2_out1
    //            //      dma/hdma_late_disable_scx2_1_out0
    //            //      dma/hdma_late_disable_scx2_2_out1
    //            //      dma/hdma_late_disable_scx3_1_out0
    //            //      dma/hdma_late_disable_scx3_2_out1
    //            //      dma/hdma_late_disable_scx5_1_out0
    //            //      dma/hdma_late_disable_scx5_2_out1
    //            //      dma/hdma_late_disable_scx5_ds_1_out0
    //            //      dma/hdma_late_disable_scx5_ds_2_out1
    //            //
    //            if (m_events.get_event_cycle(gb_event::start_hdma) > m_clock.get_clock_cycle())
    //            {
    //                m_events.remove_event(gb_event::start_hdma);
    //            }
    //        }
    //    }
}
