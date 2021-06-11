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
#include <emulator/age_gb_types.hpp>
#include <gfx/age_pixel.hpp>
#include <gfx/age_screen_buffer.hpp>
#include <pcm/age_pcm_sample.hpp>



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

    class gb_emulator
    {
    public:
        explicit gb_emulator(const uint8_vector& rom,
                             gb_hardware         hardware       = gb_hardware::auto_detect,
                             gb_colors_hint      colors_hint    = gb_colors_hint::default_colors,
                             gb_log_categories   log_categories = {});
        ~gb_emulator();

        //!
        //! \brief Get a human readable title for this emulator.
        //!
        //! The title is for informational purposes only and depends on the rom
        //! loaded into the emulator.
        //! The title is made of up to 32 characters and may be empty.
        //! It contains only the following characters:
        //! - a-z
        //! - A-Z
        //! - 0-9
        //! - _ (underscore)
        //!
        //! \return A human readable title for this emulator.
        //!
        [[nodiscard]] std::string get_emulator_title() const;

        //!
        //! \return The Game Boys native screen width (pixel).
        //! The returned value is greater than zero.
        //!
        [[nodiscard]] int16_t get_screen_width() const;

        //!
        //! \return The Game Boys native screen height (pixel).
        //! The returned value is greater than zero.
        //!
        [[nodiscard]] int16_t get_screen_height() const;

        //!
        //! \return The current front buffer containing the last fully rendered
        //! Game Boy screen.
        //!
        [[nodiscard]] const pixel_vector& get_screen_front_buffer() const;

        //!
        //! \brief Get the vector of {@link pcm_sample}s calculated by the last
        //! call to emulate().
        //!
        //! You will have to handle these samples before the next call to
        //! emulate() because emulate() discards the audio buffer's old contents.
        //! To use the returned PCM data for audio playback,
        //! it most likely has to be resampled.
        //!
        //! \return The PCM samples calculated by the last call to emulate().
        //!
        [[nodiscard]] const pcm_vector& get_audio_buffer() const;

        //!
        //! \brief Get the sampling rate of the PCM data in get_audio_buffer().
        //!
        //! The sampling rate is constant,
        //! it will not change during emulation.
        //! Note that the returned value is not one of the common sampling rates
        //! like 44100 or 48000.
        //!
        //! \return The sampling rate of the PCM data in get_audio_buffer().
        //! The returned value is greater than zero.
        //!
        [[nodiscard]] int get_pcm_sampling_rate() const;

        //!
        //! \brief Get the number of native cycles per seconds.
        //!
        //! This value can be used to synchronize the emulation to wall-clock time.
        //! The returned value is greater than zero.
        //!
        //! \return The number of native cycles per seconds.
        //!
        [[nodiscard]] int get_cycles_per_second() const;

        [[nodiscard]] int64_t get_emulated_cycles() const;

        //!
        //! \brief Get a copy of this emulator's persistent ram.
        //!
        //! You may want to save this ram to some file in order to reuse it.
        //! If this emulator has no persistent ram available,
        //! the returned vector is empty.
        //!
        //! \return A vector containing this emulator's persistent ram.
        //!
        [[nodiscard]] uint8_vector get_persistent_ram() const;

        //!
        //! \brief Set the contents of this emulator's persistent ram.
        //!
        //! The persistent ram is set to the contents of the specified vector.
        //! If this emulator does not have any persistent ram,
        //! this method has no effect.
        //! If the specified vector is bigger than the emulator's persistent ram,
        //! only the first bytes are copied.
        //! If the specified vector is smaller than the emulator's persistent ram,
        //! the gap is filled with zeroes.
        //!
        void set_persistent_ram(const uint8_vector& source);

        void set_buttons_down(int buttons);
        void set_buttons_up(int buttons);

        bool emulate(int cycles_to_emulate);

        [[nodiscard]] gb_test_info get_test_info() const;

        [[nodiscard]] const std::vector<gb_log_entry>& get_log_entries() const;

    private:
        // std::unique_ptr<gb_emulator_impl> does not work on incomplete types
        gb_emulator_impl* m_impl;
    };

} // namespace age



#endif // AGE_GB_EMULATOR_HPP
