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

#ifndef AGE_EMULATOR_HPP
#define AGE_EMULATOR_HPP

//!
//! \file
//!

#include <string>

#include <age_types.hpp>
#include <gfx/age_pixel.hpp>
#include <gfx/age_screen_buffer.hpp>
#include <pcm/age_pcm_sample.hpp>



namespace age
{

    //!
    //! \brief This is the emulator base class. Every emulator derives from it.
    //!
    class emulator
    {
        AGE_DISABLE_COPY(emulator);

    public:
        virtual ~emulator() = default;

        //!
        //! \brief Get a human readable title for this emulator.
        //!
        //! The title is for informational purposes only and may depend on whatever data
        //! has been loaded into the emulator.
        //! E.g. the gameboy emulator returns the gameboy rom's title.
        //! A title is made of up to 32 characters and may be empty.
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
        //! \return The native screen width in pixel of the emulated device.
        //! The returned value is greater than zero.
        //!
        [[nodiscard]] int16_t get_screen_width() const;

        //!
        //! \return The native screen height in pixel of the emulated device.
        //! The returned value is greater than zero.
        //!
        [[nodiscard]] int16_t get_screen_height() const;

        //!
        //! \return The current front buffer containing the emulated device's screen.
        //!
        [[nodiscard]] const pixel_vector& get_screen_front_buffer() const;

        //!
        //! \brief Get the vector of {@link pcm_sample}s calculated by the last call to emulate().
        //!
        //! You will have to handle these frames before the next call to emulate(),
        //! because emulate() discards the audio buffer's old contents.
        //! To use the returned PCM data for audio playback,
        //! the data most likely has to be resampled.
        //!
        //! \return The PCM samples calculated by the last call to emulate().
        //!
        [[nodiscard]] const pcm_vector& get_audio_buffer() const;

        //!
        //! \brief Get the sampling rate of the PCM data calculated by this emulator.
        //!
        //! The sampling rate is constant,
        //! it will not change during emulation.
        //! Note that the returned values is not necessarily one of the common sampling rates
        //! like 44100 or 48000 and may even be in the MHz range.
        //! The returned value will be greater than zero.
        //!
        //! \return The sampling rate of the PCM data calculated by this emulator.
        //! The returned value will be >0.
        //!
        [[nodiscard]] int get_pcm_sampling_rate() const;

        //!
        //! \brief Get the number of cycles emulated per seconds.
        //!
        //! This value can be used to synchronize the emulation to wall-clock time.
        //! The returned value is greater than zero.
        //!
        //! \return The number of cycles emulated per seconds.
        //!
        [[nodiscard]] int get_cycles_per_second() const;

        [[nodiscard]] int64_t get_emulated_cycles() const;

        //!
        //! \brief Get a copy of this emulator's persistent ram.
        //!
        //! You may want to save this ram to some file in order to reuse it later.
        //! If this emulator has no persistent ram available, the returned vector is empty.
        //!
        //! \return A vector containing this emulator's persistent ram.
        //!
        [[nodiscard]] virtual uint8_vector get_persistent_ram() const = 0;

        //!
        //! \brief Set the contents of this emulator's persistent ram.
        //!
        //! The persistent ram is set to the contents of the specified vector.
        //! If this emulator does not have any persistent ram, this method has no effect.
        //! If the specified vector is bigger than the emulator's persistent ram,
        //! only the first bytes are copied.
        //! If the specified vector is smaller than the emulator's persistent ram,
        //! the gap is filled with zeroes.
        //!
        //! \param source
        //!
        virtual void set_persistent_ram(const uint8_vector& source) = 0;

        virtual void set_buttons_down(int buttons) = 0;
        virtual void set_buttons_up(int buttons)   = 0;

        bool emulate(int cycles_to_emulate);

    protected:
        emulator(int16_t screen_width, int16_t screen_height, int sampling_rate, int cycles_per_second);

        screen_buffer& get_screen_buffer();
        pcm_vector&    get_pcm_vector();

        virtual int inner_emulate(int cycles_to_emulate) = 0;

        [[nodiscard]] virtual std::string inner_get_emulator_title() const = 0;

    private:
        const int m_sampling_rate;
        const int m_cycles_per_second;

        screen_buffer m_screen_buffer;
        pcm_vector    m_audio_buffer;
        int64_t       m_emulated_cycles = 0;
    };

} // namespace age



#endif // AGE_EMULATOR_HPP
