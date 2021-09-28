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

#ifndef AGE_GB_DEVICE_HPP
#define AGE_GB_DEVICE_HPP

//!
//! \file
//!

#include <emulator/age_gb_types.hpp>



namespace age
{
    //!
    //! \brief The device mode is derived from the emulated device and
    //!        cartridge capabilities.
    //!
    enum class gb_device_mode
    {
        dmg,
        cgb_in_dmg_mode,
        cgb
    };



    class gb_device
    {
    public:
        gb_device(const uint8_vector& rom, gb_device_type device_type);

        [[nodiscard]] bool cgb_in_dmg_mode() const
        {
            return m_device_mode == gb_device_mode::cgb_in_dmg_mode;
        }

        [[nodiscard]] bool cgb_mode() const
        {
            return m_device_mode == gb_device_mode::cgb;
        }

        [[nodiscard]] bool is_cgb_device() const
        {
            return m_device_mode != gb_device_mode::dmg;
        }

        [[nodiscard]] bool is_cgb_e_device() const
        {
            return m_device_type == gb_device_type::cgb_e;
        }

    private:
        const gb_device_mode m_device_mode;
        const gb_device_type m_device_type;
    };

} // namespace age



#endif // AGE_GB_DEVICE_HPP
