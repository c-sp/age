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

#ifndef AGE_GB_EMULATOR_IMPL_HPP
#define AGE_GB_EMULATOR_IMPL_HPP

//!
//! \file
//!

#include "common/age_gb_clock.hpp"
#include "common/age_gb_device.hpp"
#include "common/age_gb_events.hpp"
#include "common/age_gb_interrupts.hpp"
#include "common/age_gb_logger.hpp"
#include "lcd/age_gb_lcd.hpp"
#include "sound/age_gb_sound.hpp"

#include "age_gb_bus.hpp"
#include "age_gb_cpu.hpp"
#include "age_gb_joypad.hpp"
#include "age_gb_serial.hpp"
#include "age_gb_timer.hpp"
#include "memory/age_gb_memory.hpp"

#include <age_types.hpp>
#include <emulator/age_gb_types.hpp>



namespace age
{

    class gb_emulator_impl
    {
        AGE_DISABLE_COPY(gb_emulator_impl);
        AGE_DISABLE_MOVE(gb_emulator_impl);

    public:
        gb_emulator_impl(const uint8_vector& rom,
                         gb_device_type      device_type,
                         gb_colors_hint      colors_hint,
                         gb_log_categories   log_categories);
        ~gb_emulator_impl() = default;

        [[nodiscard]] std::string get_emulator_title() const;

        [[nodiscard]] int16_t             get_screen_width() const;
        [[nodiscard]] int16_t             get_screen_height() const;
        [[nodiscard]] const pixel_vector& get_screen_front_buffer() const;

        [[nodiscard]] const pcm_vector& get_audio_buffer() const;
        [[nodiscard]] int               get_pcm_sampling_rate() const;

        [[nodiscard]] int     get_cycles_per_second() const;
        [[nodiscard]] int64_t get_emulated_cycles() const;

        [[nodiscard]] uint8_vector get_persistent_ram() const;
        void                       set_persistent_ram(const uint8_vector& source);

        void set_buttons_down(int buttons);
        void set_buttons_up(int buttons);

        bool emulate(int cycles_to_emulate);

        [[nodiscard]] gb_test_info get_test_info() const;

        std::vector<gb_log_entry> get_and_clear_log_entries();

    private:
        int emulate_cycles(int cycles_to_emulate);
        int get_fast_forward_halt_cycles(int cycle_to_reach) const;

        screen_buffer m_screen_buffer;
        pcm_vector    m_audio_buffer;
        int64_t       m_emulated_cycles = 0;

        gb_logger               m_logger;
        gb_device               m_device;
        gb_clock                m_clock;
        gb_memory               m_memory;
        gb_interrupt_dispatcher m_interrupts;
        gb_events               m_events;
        gb_sound                m_sound;
        gb_lcd                  m_lcd;
        gb_timer                m_timer;
        gb_joypad               m_joypad;
        gb_serial               m_serial;
        gb_bus                  m_bus;
        gb_cpu                  m_cpu;
    };

} // namespace age



#endif // AGE_GB_EMULATOR_IMPL_HPP
