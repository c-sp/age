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



void age::gb_memory::mbc1_write(gb_memory& memory, uint16_t address, uint8_t value)
{
    assert(address < 0x8000);

    auto& mbc_data = memory.get_mbc_data<gb_mbc1_data>();

    switch (address & 0x6000)
    {
        case 0x0000:
            // (de)activate ram
            memory.set_cart_ram_enabled(value);
            return;

        case 0x2000:
            // select rom bank (lower 5 bits)
            mbc_data.m_bank1 = ((value & 0x1F) == 0) ? value + 1 : value;
            break;

        case 0x4000:
            // select rom/ram bank
            mbc_data.m_bank2 = value;
            break;

        case 0x6000:
            // select MBC1 mode:
            //  - mode 0: bank2 affects 0x4000-0x7FFF
            //  - mode 1: bank2 affects 0x0000-0x7FFF, 0xA000-0xBFFF
            mbc_data.m_mode1 = (value & 0x01) > 0;
            break;
    }

    //
    // verified by mooneye-gb tests
    //
    // - The value written to 0x4000 is used for rom bank switching
    //   independent of the MBC1 mode.
    // - With MBC1 mode 1 the value written to 0x4000 switches the
    //   rom bank at 0x0000.
    //
    //      emulator-only/mbc1/rom_8Mb
    //      emulator-only/mbc1/rom_16Mb
    //

    int mbc_high_bits = mbc_data.m_bank2 & 0x03;

    int high_rom_bits    = mbc_high_bits << (mbc_data.m_multicart ? 4 : 5);
    int low_rom_bank_id  = mbc_data.m_mode1 ? high_rom_bits : 0;
    int high_rom_bank_id = high_rom_bits + (mbc_data.m_bank1 & (mbc_data.m_multicart ? 0x0F : 0x1F));

    int ram_bank_id = mbc_data.m_mode1 ? mbc_high_bits : 0;

    // set rom & ram banks
    assert(mbc_data.m_multicart || ((high_rom_bank_id & 0x1F) > 0));
    memory.set_rom_banks(low_rom_bank_id, high_rom_bank_id);
    memory.set_ram_bank(ram_bank_id);
}
