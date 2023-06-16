//
// Copyright 2021 Christoph Sprenger
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

#include "age_gb_oam_dma.hpp"



namespace
{
    bool is_work_ram_dma_source(int oam_dma_src_address)
    {
        return (oam_dma_src_address >= 0xC000) && (oam_dma_src_address < 0xE000);
    }

    bool oam_dma_conflict(uint16_t address, int oam_dma_src_address, const age::gb_device& device)
    {
        // CGB:
        // using work ram as OAM DMA source will not cause conflicts when
        // accessing the external bus or the video bus:
        //      (gambatte) oamdma/oamdma_srcC000_busypop7FFF_dmg08_out657665AA_cgb04c_out657655AA
        //      (gambatte) oamdma/oamdma_srcC000_busypop9FFF_dmg08_out65765576_cgb04c_out657655AA
        //      (gambatte) oamdma/oamdma_srcC000_busypopBFFF_dmg08_out65766576_cgb04c_out65765576
        //      (gambatte) oamdma/oamdma_srcE000_busypop7FFF_dmg08_out657665AA_cgb04c_outFFFFFFAA
        //      (gambatte) oamdma/oamdma_srcE000_busypop9FFF_dmg08_out65765576_cgb04c_outFFFF55FF
        //
        if (device.cgb_mode())
        {
            return is_work_ram_dma_source(oam_dma_src_address)
                       ? (address >= 0xC000)
                       : (age::is_video_ram(address) == age::is_video_ram(oam_dma_src_address));
        }
        return age::is_video_ram(address) == age::is_video_ram(oam_dma_src_address);
    }

} // namespace



age::gb_oam_dma::gb_oam_dma(const gb_device& device,
                            const gb_clock&  clock,
                            gb_memory&       memory,
                            gb_events&       events,
                            gb_lcd&          lcd)
    : m_device(device),
      m_clock(clock),
      m_memory(memory),
      m_events(events),
      m_lcd(lcd),
      m_oam_dma_reg(device.is_cgb_device() ? 0x00 : 0xFF)
{
}



age::int16_t age::gb_oam_dma::conflicting_read(uint16_t address)
{
    AGE_ASSERT(address < 0xFE00)
    if (!dma_active())
    {
        return -1;
    }

    // no OAM DMA conflict: regular read
    if (!oam_dma_conflict(address, m_oam_dma_src_address, m_device))
    {
        return -1;
    }

    auto msg = log();
    msg << "OAM DMA read conflict:";
    auto result = m_next_oam_byte;

    // CGB OAM DMA conflict:
    // when work ram is not used as OAM DMA source,
    // reading from CGB work ram at 0xC000 - 0xDFFF will read either always
    // from 0xC000 - 0xCFFF or always from 0xD000 - 0xDFFF,
    // depending on 0xFF46 & 0x10:
    //      (gambatte) oamdma/oamdma_src0000_busypopDFFF_dmg08_out65766576_cgb04c_out657655AA
    //      (gambatte) oamdma/oamdma_src0000_busypopEFFF_dmg08_out65766576_cgb04c_out657655AA
    //      (gambatte) oamdma/oamdma_src7F00_busypopDFFF_dmg08_out65766576_cgb04c_out657655AA
    //      (gambatte) oamdma/oamdma_src7F00_busypopEFFF_dmg08_out65766576_cgb04c_out657655AA
    //      (gambatte) oamdma/oamdma_srcBF00_busypopDFFF_dmg08_out65766576_cgb04c_out657655AA
    //      (gambatte) oamdma/oamdma_srcBF00_busypopEFFF_dmg08_out65766576_cgb04c_out657655AA
    //      (gambatte) oamdma/oamdma_srcC000_busypopDFFF_dmg08_cgb04c_out65766576
    //      (gambatte) oamdma/oamdma_srcC000_busypopEFFF_dmg08_cgb04c_out65766576
    //      (gambatte) oamdma/oamdma_srcE000_busypopDFFF_dmg08_out65766576_cgb04c_outFFFF55AA
    //      (gambatte) oamdma/oamdma_srcE000_busypopEFFF_dmg08_out65766576_cgb04c_outFFFF55AA
    //
    if (m_device.cgb_mode() && (address >= 0xC000) && !is_work_ram_dma_source(m_oam_dma_src_address))
    {
        int work_ram_offset = (m_oam_dma_reg & 0x10) ? 0xD000 : 0xC000;
        result              = m_memory.read_byte(work_ram_offset + (address & 0xFFF));

        msg << "\n    * CGB: rewire to work ram at " << log_hex16(work_ram_offset);
    }

    // CGB:
    // OAM DMA VRAM conflicts will replace the next value written to
    // OAM with zero:
    //      (gambatte) oamdma/oamdma_src8000_busypop7FFF_dmg08_out65765576_cgb04c_out65005576
    //      (gambatte) oamdma/oamdma_src8000_busypop9FFF_2_dmg08_out657665FF_cgb04c_out007665FF
    //      (gambatte) oamdma/oamdma_src8000_busypop9FFF_dmg08_out657665AA_cgb04c_out007665AA
    //
    if (m_device.cgb_mode() && is_video_ram(address))
    {
        msg << "\n    * CGB: next write to OAM will be 0x00 (OAM DMA from VRAM)";
        m_override_next_oam_byte = 0;
    }

    // log read conflict
    msg << "\n    * read [" << age::log_hex16(address) << "] == " << age::log_hex8(result);
    return result;
}



bool age::gb_oam_dma::conflicting_write(uint16_t address, uint8_t value)
{
    AGE_ASSERT(address < 0xFE00)
    if (!dma_active())
    {
        return false;
    }

    // no OAM DMA conflict: regular write
    if (!oam_dma_conflict(address, m_oam_dma_src_address, m_device))
    {
        return false;
    }

    auto msg = log();
    msg << "OAM DMA conflict while trying to write [" << log_hex16(address) << "] = " << log_hex8(value);

    // CGB OAM DMA conflict:
    // when work ram is not used as OAM DMA source,
    // writing to CGB work ram at 0xC000 - 0xDFFF will write either always
    // to 0xC000 - 0xCFFF or always to 0xD000 - 0xDFFF,
    // depending on 0xFF46 & 0x10:
    //      (gambatte) oamdma/oamdma_src0000_busypushC001_dmg08_out55AA1234_cgb04c_out65AA1255
    //      (gambatte) oamdma/oamdma_src0000_busypushE001_dmg08_out55AA1234_cgb04c_out6576AA55
    //      (gambatte) oamdma/oamdma_srcA000_busypushC001_dmg08_out55AA1234_cgb04c_out65AA1255
    //      (gambatte) oamdma/oamdma_srcA000_busypushE001_dmg08_out55AA1234_cgb04c_out6576AA55
    //      (gambatte) oamdma/oamdma_srcBF00_busypushC001_dmg08_out55AA1234_cgb04c_out65AA1255
    //      (gambatte) oamdma/oamdma_srcBF00_busypushE001_dmg08_out55AA1234_cgb04c_out6576AA55
    //      (gambatte) oamdma/oamdma_srcC000_busypushC001_dmg08_out45221234_cgb04c_out6576AA34
    //      (gambatte) oamdma/oamdma_srcC000_busypushE001_dmg08_out45221234_cgb04c_out65761234
    //      (gambatte) oamdma/oamdma_srcE000_busypushC001_dmg08_out45221234_cgb04c_outFFAA1255
    //      (gambatte) oamdma/oamdma_srcE000_busypushE001_dmg08_out45221234_cgb04c_outFFFFAA55
    //
    if (m_device.cgb_mode() && (address >= 0xC000))
    {
        if (!is_work_ram_dma_source(m_oam_dma_src_address))
        {
            int work_ram_offset  = (m_oam_dma_reg & 0x10) ? 0xD000 : 0xC000;
            int work_ram_address = work_ram_offset + (address & 0xFFF);
            m_memory.write_byte(work_ram_address, value);

            msg << "\n    * CGB: will write [" << log_hex16(work_ram_address) << "] = " << log_hex8(value) << " instead";
            return true;
        }
        // When work ram is used as OAM DMA source,
        // writing to work ram during DMA has no effect:
        //      (gambatte) oamdma/oamdma_srcC000_busypushC001_dmg08_out45221234_cgb04c_out6576AA34
        //      (gambatte) oamdma/oamdma_srcC000_busypushE001_dmg08_out45221234_cgb04c_out65761234
        //      (gambatte) oamdma/oamdma_srcDF00_busypushC001_dmg08_out45221234_cgb04c_out6576AA34
        //      (gambatte) oamdma/oamdma_srcDF00_busypushE001_dmg08_out45221234_cgb04c_out65761234
        //
        msg << "\n    * CGB: work ram write ignored, won't affect OAM DMA from work ram";
        return true;
    }

    m_override_next_oam_byte = value;

    // CGB:
    // OAM DMA VRAM conflicts will write zero to the OAM:
    //      (gambatte) oamdma/oamdma_src8000_busypush8001_dmg08_out55761234_cgb04c_out00761234
    //      (gambatte) oamdma/oamdma_src8000_busypushA001_dmg08_out65AA1255_cgb04c_out65001255
    //      (gambatte) oamdma/oamdma_src9F00_busypush8001_dmg08_out55761234_cgb04c_out00761234
    //      (gambatte) oamdma/oamdma_src9F00_busypushA001_dmg08_out65AA1255_cgb04c_out65001255
    //
    if (m_device.cgb_mode())
    {
        if (is_video_ram(address))
        {
            m_override_next_oam_byte = 0;
            msg << "\n    * CGB: replacing " << log_hex8(value)
                << " with " << log_hex8(m_override_next_oam_byte) << " (OAM DMA from VRAM)";
        }
    }

    // DMG:
    // any writes during OAM DMA transfer from work ram will mix up existing
    // OAM data and the data to be written:
    //      (gambatte) oamdma/oamdma_srcC000_busypush0001_dmg08_out4576AA34_cgb04c_out6576AA34
    //      (gambatte) oamdma/oamdma_srcC000_busypush8001_dmg08_out65221255_cgb04c_out65761255
    //      (gambatte) oamdma/oamdma_srcC000_busypushC001_dmg08_out45221234_cgb04c_out6576AA34
    //      (gambatte) oamdma/oamdma_srcC000_busypushE001_dmg08_out45221234_cgb04c_out65761234
    //! \todo DMG: mixing OAM data & work ram writes: we should probably examine this further
    //
    else
    {
        if ((m_oam_dma_src_address >= 0xC000) && (m_oam_dma_src_address < 0xFE00))
        {
            m_override_next_oam_byte = m_next_oam_byte & value;
            msg << "\n    * DMG: replacing " << log_hex8(value)
                << " with " << log_hex8(m_override_next_oam_byte)
                << " (mixing OAM data when writing to work ram)";
        }
    }

    // log write conflict
    msg << "\n    * will write " << log_hex8(m_override_next_oam_byte) << " to OAM next"
        << "\n    * will NOT write [" << log_hex16(address) << "] = " << log_hex8(value);
    return true;
}



void age::gb_oam_dma::set_back_clock(int clock_cycle_offset)
{
    gb_set_back_clock_cycle(m_oam_dma_last_cycle, clock_cycle_offset);
}



void age::gb_oam_dma::write_dma_reg(uint8_t value)
{
    // Reading the DMA register will always return the last value
    // written to it even if no OAM DMA was triggered.
    //
    //      (mooneye-gb) acceptance/oam_dma/reg_read
    //
    m_oam_dma_reg = value;

    // DMG: OAM DMA starts after the next M-cycle
    //
    //      (mooneye-gb) acceptance/oam_dma_start
    //
    int clk_start = m_clock.get_machine_cycle_clocks() * 2;
    m_events.schedule_event(gb_event::start_oam_dma, clk_start);

    log() << "write DMA = " << log_hex8(value)
          << ", OAM DMA will start on clock cycle " << (m_clock.get_clock_cycle() + clk_start)
          << (m_oam_dma_active ? " and terminate the current OAM DMA" : "");
}



void age::gb_oam_dma::handle_start_dma_event()
{
    m_oam_dma_last_cycle = m_clock.get_clock_cycle();
    m_oam_dma_active     = true;
    m_oam_dma_offset     = 0;

    // Trying to trigger an OAM DMA transfer from a memory address
    // greater than 0xDFFF will instead trigger an OAM DMA transfer
    // for the corresponding 0xC000-0xDFFF memory range.
    //
    //      (mooneye-gb) acceptance/oam_dma/sources-dmgABCmgbS
    //
    // CGB: triggering an OAM DMA transfer from a memory address
    // greater than 0xDFFF will set OAM data to 0xFF.
    //
    //      (gambatte) oamdma/oamdma_srcE000_busypopFE9F_dmg08_out6576FFFF_cgb04c_outFFFFFFFF
    //      (gambatte) oamdma/oamdma_srcE000_busypushFEA1_dmg08_out65768700_cgb04c_outFFFFFF34
    //      (gambatte) oamdma/oamdma_srcEF00_busypopFE9F_dmg08_out6576FFFF_cgb04c_outFFFFFFFF
    //      (gambatte) oamdma/oamdma_srcEF00_busypushFEA1_dmg08_out65768700_cgb04c_outFFFFFF34
    //      (gambatte) oamdma/oamdma_srcF000_busypopFE9F_dmg08_out6576FFFF_cgb04c_outFFFFFFFF
    //      (gambatte) oamdma/oamdma_srcF000_busypushFEA1_dmg08_out65768700_cgb04c_outFFFFFF34
    //      (gambatte) oamdma/oamdma_srcFE00_busypopFE9F_dmg08_out6576FFFF_cgb04c_outFFFFFFFF
    //      (gambatte) oamdma/oamdma_srcFE00_busypushFEA1_dmg08_out65768700_cgb04c_outFFFFFF34
    //
    m_oam_dma_src_address = m_device.is_cgb_device()
                                ? m_oam_dma_reg * 0x100
                                : (m_oam_dma_reg * 0x100) & ((m_oam_dma_reg > 0xDF) ? 0xDF00 : 0xFF00);

    log() << "starting OAM DMA, reading from " << log_hex16(m_oam_dma_src_address);
}



void age::gb_oam_dma::continue_dma()
{
    if (!m_oam_dma_active)
    {
        return;
    }

    int current_clk    = m_clock.get_clock_cycle();
    int cycles_elapsed = current_clk - m_oam_dma_last_cycle;
    cycles_elapsed &= ~(m_clock.get_machine_cycle_clocks() - 1);
    m_oam_dma_last_cycle += cycles_elapsed;
    cycles_elapsed <<= m_clock.is_double_speed() ? 1 : 0;

    AGE_ASSERT((cycles_elapsed & 3) == 0)
    int bytes = std::min(cycles_elapsed / 4, 160 - m_oam_dma_offset);

    for (int i = m_oam_dma_offset, max = m_oam_dma_offset + bytes; i < max; ++i)
    {
        uint8_t byte = read_dma_byte(i);
        //! \todo apparently oam dma is not blocked by mode 2 and 3, is there any test rom for this?
        //        (see e.g. Zelda on DMG: sprites bugs when blocking oam writes here)
        m_lcd.write_oam_dma(i, byte);
        m_override_next_oam_byte = uint16_t_max;

        auto msg = log();
        msg << "write OAM [" << log_hex16(0xFE00 + i) << "] = " << log_hex8(byte);
        if (m_oam_dma_src_address >= 0xE000)
        {
            msg << " (CGB: OAM DMA source >= 0xE000 invalid)";
        }
        else
        {
            uint16_t address = m_oam_dma_src_address + i;
            msg << " == [" << log_hex16(address) << "]";
        }
    }

    m_oam_dma_offset += bytes;
    AGE_ASSERT(m_oam_dma_offset <= 160)
    if (m_oam_dma_offset >= 160)
    {
        m_oam_dma_active     = false;
        m_oam_dma_last_cycle = gb_no_clock_cycle;

        log() << "OAM DMA finished";
    }
    else
    {
        m_next_oam_byte = read_dma_byte(m_oam_dma_offset);
    }
}



age::uint8_t age::gb_oam_dma::read_dma_byte(int offset) const
{
    if (m_override_next_oam_byte < uint16_t_max)
    {
        return m_override_next_oam_byte;
    }
    if (m_oam_dma_src_address >= 0xE000)
    {
        AGE_ASSERT(m_device.is_cgb_device())
        return 0xFF;
    }
    return m_memory.read_byte(m_oam_dma_src_address + offset);
}
