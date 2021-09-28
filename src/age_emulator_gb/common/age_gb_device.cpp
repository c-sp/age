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
    constexpr age::uint16_t gb_cia_ofs_cgb = 0x0143;



    age::gb_device_mode calculate_device_mode(const age::uint8_vector&  rom,
                                              const age::gb_device_type device_type)
    {
        auto rom_byte_0x143 = (rom.size() > gb_cia_ofs_cgb) ? rom[gb_cia_ofs_cgb] : 0;

        const bool cgb_flag = rom_byte_0x143 >= 0x80;
        switch (device_type)
        {
            case age::gb_device_type::dmg:
                return age::gb_device_mode::dmg;

            case age::gb_device_type::cgb_abcd:
            case age::gb_device_type::cgb_e:
                return cgb_flag
                           ? age::gb_device_mode::cgb
                           : age::gb_device_mode::cgb_in_dmg_mode;

            default:
                // auto-detect device mode from cartridge CGB flag
                return cgb_flag
                           ? age::gb_device_mode::cgb
                           : age::gb_device_mode::dmg;
        }
    }



    age::gb_device_type calculate_device_type(const age::gb_device_type device_type,
                                              const age::gb_device_mode device_mode)
    {
        if (device_type != age::gb_device_type::auto_detect)
        {
            return device_type;
        }

        // auto-detect device type by mode
        return (device_mode == age::gb_device_mode::dmg)
                   ? age::gb_device_type::dmg
                   : age::gb_device_type::cgb_e;
    }

} // namespace



age::gb_device::gb_device(const uint8_vector&  rom,
                          const gb_device_type device_type)

    : m_device_mode(calculate_device_mode(rom, device_type)),
      m_device_type(calculate_device_type(device_type, m_device_mode))
{
}
