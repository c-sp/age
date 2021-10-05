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
    constexpr const std::array dmg_bus_id = {
        1, // 0x0000 - 0x1FFF : rom
        1, // 0x2000 - 0x3FFF : rom
        1, // 0x4000 - 0x5FFF : rom
        1, // 0x6000 - 0x7FFF : rom
        2, // 0x8000 - 0x9FFF : video ram
        1, // 0xA000 - 0xBFFF : cart ram
        1, // 0xC000 - 0xDFFF : internal ram
        1  // 0xE000 - 0xFE00 : internal ram echo
    };

    constexpr const std::array cgb_bus_id = {
        1, // 0x0000 - 0x1FFF : rom
        1, // 0x2000 - 0x3FFF : rom
        1, // 0x4000 - 0x5FFF : rom
        1, // 0x6000 - 0x7FFF : rom
        2, // 0x8000 - 0x9FFF : video ram
        1, // 0xA000 - 0xBFFF : cart ram
        3, // 0xC000 - 0xDFFF : internal ram
        3  // 0xE000 - 0xFE00 : internal ram echo
    };

} // namespace



age::gb_oam_dma::gb_oam_dma(const gb_device& device,
                            const gb_clock&  clock,
                            const gb_memory& memory,
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



bool age::gb_oam_dma::is_on_dma_bus(uint16_t address) const
{
    // According to Gekkio's "Game Boy: Complete Technical Reference" OAM DMA
    // uses either the external bus or the video bus.
    //
    // However, there are several Gambatte test roms indicating that
    // internal ram on the CGB is on a separate bus:
    //      (gambatte) oamdma/oamdma_src0000_busypopBFFF
    //      (gambatte) oamdma/oamdma_src0000_busypopBFFF_2
    //
    AGE_ASSERT(address < 0xFE00);
    const auto& bus_id = m_device.is_cgb_device() ? cgb_bus_id : dmg_bus_id;
    return bus_id[address >> 13] == bus_id[m_oam_dma_src_address >> 13];
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

    // Trying to trigger an OAM DMA transfer for a memory address
    // greater than 0xDFFF will instead trigger an OAM DMA transfer
    // for the corresponding 0xC000-0xDFFF memory range.
    //
    //      (mooneye-gb) acceptance/oam_dma/sources-dmgABCmgbS
    //
    m_oam_dma_src_address = (m_oam_dma_reg * 0x100) & ((m_oam_dma_reg > 0xDF) ? 0xDF00 : 0xFF00);
    m_oam_dma_offset      = 0;

    log() << "starting OAM DMA, reading from " << log_hex16(m_oam_dma_src_address);
}



void age::gb_oam_dma::override_next_oam_byte(uint8_t value)
{
    m_override_next_oam_byte = value;
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
        uint16_t address = (m_oam_dma_src_address + i) & 0xFFFF;
        uint8_t  byte    = (m_override_next_oam_byte >= 0)
                               ? m_override_next_oam_byte
                               : m_memory.read_byte(address);
        //! \todo apparently oam dma is not blocked by mode 2 and 3, is there any test rom for this?
        //        (see e.g. Zelda on DMG: sprites bugs when blocking oam writes here)
        m_lcd.write_oam_dma(i, byte);

        log() << "write OAM[" << log_hex8(i)
              << "] = " << log_hex8(byte)
              << " = [" << log_hex16(address) << "]";

        m_override_next_oam_byte = -1;
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
        m_next_oam_byte = m_memory.read_byte(m_oam_dma_src_address + m_oam_dma_offset);
    }
}
