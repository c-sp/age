//
// Copyright 2022 Christoph Sprenger
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

#include "age_gb_memory.hpp"

namespace
{
    uint16_t mbc2_ram_address(uint16_t address)
    {
        uint16_t ram_address = address - 0xA000;
        return 0xA000 + (ram_address & 0x1FF);
    }

} // namespace



void age::gb_memory::mbc2_write(gb_memory& memory, uint16_t address, uint8_t value)
{
    AGE_ASSERT(address < 0x8000)

    // writes to $4000-$7FFF have no effect
    if (address >= 0x4000)
    {
        return;
    }

    // (de)activate ram
    if ((address & 0x100) == 0)
    {
        memory.set_cart_ram_enabled(value);
    }

    // switch rom bank
    else
    {
        int rom_bank_id = value & 0x0F;
        memory.set_rom_banks(0, (rom_bank_id == 0) ? 1 : rom_bank_id);
    }
}



void age::gb_memory::mbc2_cart_ram_write(gb_memory& memory, uint16_t address, uint8_t value)
{
    if (memory.m_num_cart_ram_banks > 0)
    {
        memory.m_memory[memory.get_offset(mbc2_ram_address(address))] = value | 0xF0;
    }
}

age::uint8_t age::gb_memory::mbc2_cart_ram_read(gb_memory& memory, uint16_t address)
{
    return (memory.m_num_cart_ram_banks > 0)
               ? memory.m_memory[memory.get_offset(mbc2_ram_address(address))] | 0xF0
               : 0xFF;
}
