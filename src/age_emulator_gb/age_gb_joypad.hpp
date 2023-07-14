//
// © 2017 Christoph Sprenger <https://github.com/c-sp>
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

#ifndef AGE_GB_JOYPAD_HPP
#define AGE_GB_JOYPAD_HPP

//!
//! \file
//!

#include "common/age_gb_device.hpp"
#include "common/age_gb_interrupts.hpp"

#include <age_types.hpp>



namespace age
{

    class gb_joypad
    {
        AGE_DISABLE_COPY(gb_joypad);
        AGE_DISABLE_MOVE(gb_joypad);

    public:
        gb_joypad(const gb_device& device, gb_interrupt_trigger& interrupts);
        ~gb_joypad() = default;

        [[nodiscard]] uint8_t read_p1() const;
        void                  write_p1(uint8_t byte);
        void                  set_buttons_down(int buttons);
        void                  set_buttons_up(int buttons);

    private:
        gb_interrupt_trigger& m_interrupts;
        uint8_t               m_p1;
        uint8_t               m_p14 = 0x0F;
        uint8_t               m_p15 = 0x0F;
    };

} // namespace age



#endif // AGE_GB_JOYPAD_HPP
