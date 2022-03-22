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



void age::gb_memory::mbc7_write(gb_memory& memory, uint16_t address, uint8_t value)
{
    AGE_ASSERT(address < 0x8000)

    switch (address & 0x6000)
    {
        case 0x0000:
            // RAM enable 1
            memory.set_ram_accessible(value);
            break;

        case 0x2000:
            memory.set_rom_banks(0, value);
            break;

        case 0x4000:
            //! \todo MBC7: RAM enable 2
            memory.set_ram_bank(value & 0x0F);
            break;
    }
}



void age::gb_memory::mbc7_cart_ram_write(gb_memory& memory, uint16_t address, uint8_t value)
{
    //! \todo implement mbc7_cart_ram_write
    AGE_UNUSED(memory);
    AGE_UNUSED(address);
    AGE_UNUSED(value);
}



age::uint8_t age::gb_memory::mbc7_cart_ram_read(gb_memory& memory, uint16_t address)
{
    //! \todo implement mbc7_cart_ram_read
    AGE_UNUSED(memory);
    AGE_UNUSED(address);
    return 0xFF;
}
