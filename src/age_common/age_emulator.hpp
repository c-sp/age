//
// Copyright (c) 2010-2017 Christoph Sprenger
//
// This file is part of AGE ("Another Gameboy Emulator").
// <https://gitlab.com/csprenger/AGE>
//
// AGE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// AGE is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with AGE.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AGE_EMULATOR_HPP
#define AGE_EMULATOR_HPP

//!
//! \file
//!

#include <string>

#include <age_non_copyable.hpp>
#include <age_pcm_sample.hpp>
#include <age_pixel.hpp>
#include <age_screen_buffer.hpp>
#include <age_types.hpp>



namespace age {



//!
//! \brief This is the emulator base class. Every emulator derives from it.
//!
class emulator : public non_copyable
{
public:

    virtual ~emulator() = default;

    //!
    //! \brief Get a human readable title for this emulator.
    //!
    //! The title is for informational purposes only and may depend on what ever data
    //! has been loaded into the emulator.
    //! E.g. for gameboy cartridges the cartridge title is returned.
    //! A title is made of up to 32 characters and may be empty.
    //! It contains only the following characters:
    //! - a-z
    //! - A-Z
    //! - 0-9
    //! - _ (underscore)
    //!
    //! \return A human readable title for this emulator.
    //!
    std::string get_emulator_title() const;

    //!
    //! \return The native screen width in pixel of the emulated device. The returned value will be >0.
    //!
    uint get_screen_width() const;

    //!
    //! \return The native screen height in pixel of the emulated device. The returned value will be >0.
    //!
    uint get_screen_height() const;

    //!
    //! \return The current front buffer containing the emulated device's screen.
    //!
    const pixel_vector& get_screen_front_buffer() const;

    //!
    //! Get the vector of {@link pcm_sample}s calculated by the last call to emulate().
    //! You will have to handle these frames before the next call to emulate(),
    //! because the latter will discard the audio buffer'S old contents.
    //! To use the returned PCM data for audio playback, the data should be resampled.
    //!
    //! \return The PCM samples calculated by the last call to emulate().
    //!
    const pcm_vector& get_audio_buffer() const;

    //!
    //! Get the sampling rate of the PCM data calculated by this emulator.
    //! The sampling rate is expected to be constant.
    //! Note that the returned values is not necessarily one of the common sampling rates
    //! like 44100 or 48000 and may even be in the MHz range.
    //! The returned value will be greater than zero.
    //!
    //! \return The sampling rate of the PCM data calculated by this emulator.
    //!
    uint get_pcm_sampling_rate() const;

    uint get_cycles_per_second() const;
    uint64 get_emulated_cycles() const;

    //!
    //! Get a copy of this emulator's persistent ram.
    //! You may want to save this ram to some file to reuse it some other time.
    //! If this emulator does not contain any persistent ram, the returned vector is empty.
    //!
    //! \return A vector containing this emulator's persistent ram.
    //!
    virtual uint8_vector get_persistent_ram() const = 0;

    //!
    //! \brief Set the contents of this emulator's persistent ram.
    //!
    //! The persistent ram is set to the contents of the specified vector.
    //! If this emulator does not have any persistent ram, this method has no effect.
    //! If the specified vector is bigger than the persistent ram, only the first bytes are copied.
    //! If the specified vector is smaller than the persistent ram, the gap is filled with zeroes.
    //!
    //! \param source
    //!
    virtual void set_persistent_ram(const uint8_vector &source) = 0;

    virtual void set_buttons_down(uint buttons) = 0;
    virtual void set_buttons_up(uint buttons) = 0;

    bool emulate(uint64 min_cycles_to_emulate);

protected:

    emulator(uint screen_width, uint screen_height, uint sampling_rate, uint cycles_per_second);

    screen_buffer& get_screen_buffer();
    pcm_vector& get_pcm_vector();

    virtual uint64 inner_emulate(uint64 min_cycles_to_emulate) = 0;

    virtual std::string inner_get_emulator_title() const = 0;

private:

    const uint m_sampling_rate;
    const uint m_cycles_per_second;

    screen_buffer m_screen_buffer;
    pcm_vector m_audio_buffer;
    uint64 m_emulated_cycles = 0;
};



} // namespace age



#endif // AGE_EMULATOR_HPP
