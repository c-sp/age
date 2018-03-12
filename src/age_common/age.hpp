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

#ifndef AGE_HPP
#define AGE_HPP

//!
//! \file
//!

//
// Compile for debugging?
// This will enable e.g. logging macros.
//
#if defined _DEBUG || defined DEBUG
#define AGE_DEBUG

#endif

// we skip the following includes for doxygen output since they would bloat the include graphs
//! \cond

#include <limits>
#include <algorithm> // std::generate, std::fill, std::sort
#include <memory> // std::shared_ptr
#include <functional>

#include <array>
#include <vector>

//! \endcond





//---------------------------------------------------------
//
//   debug build macros
//
//---------------------------------------------------------

#ifdef AGE_DEBUG

// we skip the following includes for doxygen output since they would bloat the include graphs
//! \cond

#include <cassert>
#include <chrono>
#include <iostream> // std::cout
#include <iomanip> // std::quoted
#include <sstream> // std::stringstream

#include <thread>
#include <mutex>

//! \endcond

namespace age
{

//!
//! Utility class to handle concurrent logging to std::cout.
//! It uses the usual fluent interface for passing values to an std::ostream using operator<<
//! but collects the logging output to some buffer until we explicitly tell it to pass it to std::cout.
//! Streaming to std::cout is done after locking a mutex, so that there is always only one thread
//! using st::cout (provided all threads use concurrent_cout).
//!
class concurrent_cout
{
public:

    template<class T>
    concurrent_cout& operator<<(const T &t)
    {
        m_ss << t;
        return *this;
    }

    void log_line();

private:

    static std::mutex M_mutex;
    std::stringstream m_ss;
};

//!
//! \brief Utility class for creating an std::string containing a human readable form of the current time.
//!
class age_log_time : public std::string
{
public:

    age_log_time();

private:

    static std::string get_timestamp();
};

} // namespace age



#define AGE_ASSERT(x) assert(x)
#define AGE_LOG(x) (age::concurrent_cout() << age::age_log_time() << " thread " << std::this_thread::get_id() << " " << __func__ << "  -  " << x).log_line()

#else // #ifdef AGE_DEBUG

#define AGE_ASSERT(x)
#define AGE_LOG(x)

#endif // #ifdef AGE_DEBUG





//---------------------------------------------------------
//
//   types & constants
//
//---------------------------------------------------------

namespace age
{

// typedefs

typedef std::uint8_t  uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;

typedef std::int8_t  int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;

//!
//! Defines an unsigned integer type that matches the architecture's default CPU register
//! size (32 bits or 64 bits) but is guaranteed to be at least 32 bits wide.
//!
typedef size_t uint;

template<uint _size>
using uint8_array = std::array<uint8, _size>;

class lpcm_stereo_sample; // class defined below, we just want all typedefs in one place ...

typedef std::vector<uint8> uint8_vector;
typedef std::vector<std::string> string_vector;
typedef std::vector<lpcm_stereo_sample> pcm_vector;



// constants and constant expressions

const uint8_vector empty_uint8_vector = uint8_vector();

constexpr const char *project_name = "AGE";
constexpr const char *project_version = "1.0";

constexpr uint uint_max = std::numeric_limits<uint>::max();

template<typename E>
constexpr auto to_integral(E e) -> typename std::underlying_type<E>::type
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}





//---------------------------------------------------------
//
//   utility classes
//
//---------------------------------------------------------

//!
//! \brief Base class for non-copyable instances.
//!
class non_copyable
{
public:

    non_copyable() {}

    non_copyable(const non_copyable&) = delete;
    non_copyable(non_copyable&&) = delete;
    non_copyable& operator=(const non_copyable&) = delete;
    non_copyable& operator=(non_copyable&&) = delete;
};



struct pixel
{
    pixel()
        : pixel(0, 0, 0)
    {}

    pixel(uint32 x8r8g8b8)
        : m_x8r8g8b8(x8r8g8b8)
    {}

    pixel(uint r, uint g, uint b)
    {
        AGE_ASSERT(r <= 255);
        AGE_ASSERT(g <= 255);
        AGE_ASSERT(b <= 255);

        m_x8r8g8b8 = 0xFF000000 + (r << 16) + (g << 8) + b;
    }

    bool operator==(const pixel &other) const
    {
        return m_x8r8g8b8 == other.m_x8r8g8b8;
    }

    bool operator!=(const pixel &other) const
    {
        return m_x8r8g8b8 != other.m_x8r8g8b8;
    }

    uint32 m_x8r8g8b8;
};

typedef std::vector<pixel> pixel_vector;



struct lpcm_stereo_sample
{
    lpcm_stereo_sample()
        : lpcm_stereo_sample(0, 0)
    {}

    lpcm_stereo_sample(int16 left_sample, int16 right_sample)
    {
        m_samples[0] = left_sample;
        m_samples[1] = right_sample;
    }

    lpcm_stereo_sample& operator*=(float factor)
    {
        AGE_ASSERT(factor <= 1);
        AGE_ASSERT(factor >= 0);
        m_samples[0] = static_cast<int16>(m_samples[0] * factor);
        m_samples[1] = static_cast<int16>(m_samples[1] * factor);
        return *this;
    }

    lpcm_stereo_sample& operator-=(const lpcm_stereo_sample &other)
    {
        m_samples[0] -= other.m_samples[0];
        m_samples[1] -= other.m_samples[1];
        return *this;
    }

    lpcm_stereo_sample& operator+=(const lpcm_stereo_sample &other)
    {
        m_samples[0] += other.m_samples[0];
        m_samples[1] += other.m_samples[1];
        return *this;
    }

    bool operator==(const lpcm_stereo_sample &other) const
    {
        return m_stereo_sample == other.m_stereo_sample;
    }

    bool operator!=(const lpcm_stereo_sample &other) const
    {
        return m_stereo_sample != other.m_stereo_sample;
    }

    union
    {
        uint32 m_stereo_sample;
        int16 m_samples[2];
    };
};



class video_buffer_handler
{
public:

    video_buffer_handler(uint screen_width, uint screen_height);

    uint get_front_buffer_index() const;
    uint get_screen_width() const;
    uint get_screen_height() const;

    const pixel_vector& get_front_buffer() const;
    const pixel_vector& get_back_buffer() const;

    pixel_vector& get_back_buffer();
    pixel* get_first_scanline_pixel(uint scanline);
    void switch_buffers();

private:

    const uint m_screen_width;
    const uint m_screen_height;

    std::array<pixel_vector, 2> m_buffers;
    uint m_current_front_buffer = 0;
};





//---------------------------------------------------------
//
//   emulator base class
//
//---------------------------------------------------------

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
    //! \return The current video front buffer containing the emulated device's screen.
    //!
    const pixel_vector& get_video_front_buffer() const;

    //!
    //! Get the vector of {@link lpcm_stereo_sample}s calculated by the last call to emulate().
    //! You will have to handle these frames before the next call to emulate(),
    //! because the latter will discard the audio buffer'S old contents.
    //! To use the returned PCM data for audio playback, the data should be resampled.
    //!
    //! \return The LPCM samples calculated by the last call to emulate().
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

    video_buffer_handler& get_video_buffer_handler();
    pcm_vector& get_pcm_vector();

    virtual uint64 inner_emulate(uint64 min_cycles_to_emulate) = 0;

    virtual std::string inner_get_emulator_title() const = 0;

private:

    const uint m_sampling_rate;
    const uint m_cycles_per_second;

    video_buffer_handler m_video_buffer_handler;
    pcm_vector m_audio_buffer;
    uint64 m_emulated_cycles = 0;
};



} // namespace age

#endif // AGE_HPP
