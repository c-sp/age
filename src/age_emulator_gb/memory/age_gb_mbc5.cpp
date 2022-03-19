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



void age::gb_memory::mbc5_write(gb_memory& memory, uint16_t address, uint8_t value)
{
    AGE_ASSERT(address < 0x8000)

    // workaround for older STL implementations
    // (we actually want to use std::get<gb_mbc1_data> here ...)
    auto* p_mbc_data = std::get_if<gb_mbc5_data>(&memory.m_mbc_data);
    auto& mbc_data   = *p_mbc_data;

    switch (address & 0x6000)
    {
        case 0x0000:
            // (de)activate ram
            memory.set_ram_accessible(value);
            break;

        case 0x2000:
            // select rom bank (lower 5 bits)
            if ((address & 0x1000) == 0)
            {
                mbc_data.m_2000 = value;
            }
            else
            {
                mbc_data.m_3000 = value;
            }
            memory.set_rom_banks(0, ((mbc_data.m_3000 & 0x01) << 8) + mbc_data.m_2000);
            break;

        case 0x4000:
            // select ram bank
            memory.set_ram_bank(value & 0x0F);
            break;
    }
}



void age::gb_memory::mbc5_rumble_write(gb_memory& memory, uint16_t address, uint8_t value)
{
    AGE_ASSERT(address < 0x8000)

    // clear rumble motor bit
    if ((address & 0x6000) == 0x4000)
    {
        if (value & 0x08)
        {
            memory.log_mbc() << "clearing bit 0x08 of " << log_hex8(value);
            value &= ~0x08;
        }
    }

    // behave like MBC5
    mbc5_write(memory, address, value);
}
