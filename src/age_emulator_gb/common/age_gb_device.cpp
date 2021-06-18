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

#include "age_gb_device.hpp"



namespace
{

    age::gb_cart_mode calculate_cart_mode(const uint8_t          rom_byte_0x143,
                                          const age::gb_hardware hardware)
    {
        const bool cgb_flag = rom_byte_0x143 >= 0x80;
        switch (hardware)
        {
            case age::gb_hardware::dmg:
                return age::gb_cart_mode::dmg;

            case age::gb_hardware::cgb:
                return cgb_flag
                           ? age::gb_cart_mode::cgb
                           : age::gb_cart_mode::dmg_on_cgb;

            default:
                // auto-detect hardware
                return cgb_flag
                           ? age::gb_cart_mode::cgb
                           : age::gb_cart_mode::dmg;
        }
    }

} // namespace



age::gb_device::gb_device(const uint8_t     rom_byte_0x143,
                          const gb_hardware hardware)

    : m_cart_mode(calculate_cart_mode(rom_byte_0x143, hardware))
{
}

age::gb_cart_mode age::gb_device::get_cart_mode() const
{
    return m_cart_mode;
}

bool age::gb_device::is_cgb() const
{
    //! \todo some calls to is_cgb() should probably be replaced by is_cgb_hardware()
    return m_cart_mode == gb_cart_mode::cgb;
}

bool age::gb_device::is_cgb_hardware() const
{
    return m_cart_mode != gb_cart_mode::dmg;
}
