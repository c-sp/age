//
// © 2022 Christoph Sprenger <https://github.com/c-sp>
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

#include <cassert>



void age::gb_memory::mbc3_write(gb_memory& memory, uint16_t address, uint8_t value)
{
    assert(address < 0x8000);
    switch (address & 0x6000)
    {
        case 0x0000:
            // (de)activate ram
            memory.set_cart_ram_enabled(value);
            break;

        case 0x2000: {
            // select rom bank
            // (automatic MBC30 mode: we don't restrict the number of accessible rom banks here)
            memory.set_rom_banks(0, (value == 0) ? 1 : value);
            break;
        }

        case 0x4000:
            // select ram bank
            // (automatic MBC30 mode: we don't restrict the number of accessible ram banks here)
            memory.set_ram_bank(value);
            break;

        case 0x6000:
            // latch real time clock
            break;
    }
}
