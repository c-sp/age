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



void age::gb_memory::mbc3_write(gb_memory& memory, uint16_t address, uint8_t value)
{
    AGE_ASSERT(address < 0x8000)
    memory.log() << "(MBC3) writing " << log_hex16(address) << " = " << log_hex8(value);

    switch (address & 0x6000)
    {
        case 0x0000:
            // (de)activate ram
            memory.set_ram_accessible(value);
            break;

        case 0x2000: {
            // select rom bank
            int rom_bank_id = value & 0x7F;
            if (rom_bank_id == 0)
            {
                rom_bank_id = 1;
            }
            memory.set_rom_banks(0, rom_bank_id);
            break;
        }

        case 0x4000:
            // select ram bank
            memory.set_ram_bank(value & 0x03);
            break;

        case 0x6000:
            // latch real time clock
            break;
    }
}
