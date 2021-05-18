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

#ifndef AGE_GB_EMULATOR_HPP
#define AGE_GB_EMULATOR_HPP

//!
//! \file
//!

#include <string>

#include <age_types.hpp>
#include <emulator/age_emulator.hpp>
#include <emulator/age_gb_types.hpp>



namespace age
{

    constexpr int gb_right  = 0x01;
    constexpr int gb_left   = 0x02;
    constexpr int gb_up     = 0x04;
    constexpr int gb_down   = 0x08;
    constexpr int gb_a      = 0x10;
    constexpr int gb_b      = 0x20;
    constexpr int gb_select = 0x40;
    constexpr int gb_start  = 0x80;



    class gb_emulator_impl; // forward class declaration to decouple the actual implementation

    class gb_emulator : public emulator
    {
    public:
        explicit gb_emulator(const uint8_vector& rom,
                             gb_hardware         hardware    = gb_hardware::auto_detect,
                             gb_colors_hint      colors_hint = gb_colors_hint::default_colors);
        ~gb_emulator() override;

        [[nodiscard]] uint8_vector get_persistent_ram() const override;
        void                       set_persistent_ram(const uint8_vector& source) override;

        void set_buttons_down(int buttons) override;
        void set_buttons_up(int buttons) override;

        [[nodiscard]] gb_test_info get_test_info() const;

    protected:
        int inner_emulate(int cycles_to_emulate) override;

        [[nodiscard]] std::string inner_get_emulator_title() const override;

    private:
        // cannot use std::unique_ptr<gb_emulator_impl> since it does not work on incomplete types
        gb_emulator_impl* m_impl;
    };

} // namespace age



#endif // AGE_GB_EMULATOR_HPP
