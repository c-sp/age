//
// Copyright 2019 Christoph Sprenger
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

class gb_device
{
public:

    gb_device(uint8_t rom_byte_0x143, gb_hardware hardware);

    gb_cart_mode get_cart_mode() const;
    bool is_cgb() const; //!< get_cart_mode() == gb_cart_mode::cgb
    bool is_cgb_hardware() const; //!< get_cart_mode() != gb_cart_mode::dmg

private:

    const gb_cart_mode m_cart_mode;
};

} // namespace age



#endif // AGE_GB_DEVICE_HPP
