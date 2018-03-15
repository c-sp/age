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

#ifndef AGE_UI_QT_HPP
#define AGE_UI_QT_HPP

//!
//! \file
//!

#include <atomic>
#include <ostream>
#include <string>
#include <vector>

#include <QString>
#include <QtGui/qopengl.h> // GLint

#include <age_types.hpp>



// The following operators must be declared outside of any namespace.
std::ostream& operator<<(std::ostream &stream, const QString &string);
std::string operator+(const std::string &std_string, const QString &q_string);



namespace age
{

typedef std::atomic_size_t  atomic_uint;
typedef std::atomic<uint64> atomic_uint64;



bool is_checked(int checked_state);





//---------------------------------------------------------
//
//   settings
//
//---------------------------------------------------------

//!
//! \brief The maximal size in bytes of files allowed to be handled.
//!
//! While in theory we could handle bigger files, it does not make sense for our use cases.
//! There are no cartridge or RAM files bigger than this.
//! In fact it could even lower performance and stability to try loading such big files
//! (think of a RAM file of several gigabytes being loaded into memory).
//!
constexpr qint64 max_file_bytes = 32 * 1024 * 1024;



//---------------------------------------------------------
//
//   audio
//
//---------------------------------------------------------

enum class qt_downsampler_quality
{
    low,
    high,
    highest
};

//!
//! \brief The minimal supported audio latency in milliseconds.
//!
constexpr int qt_audio_latency_milliseconds_min = 30;

//!
//! \brief The maximal supported audio latency in milliseconds.
//!
constexpr int qt_audio_latency_milliseconds_max = 500;
constexpr int qt_audio_latency_milliseconds_step = 10;

static_assert((qt_audio_latency_milliseconds_min % qt_audio_latency_milliseconds_step) == 0, "audio latency step size does not match");
static_assert((qt_audio_latency_milliseconds_max % qt_audio_latency_milliseconds_step) == 0, "audio latency step size does not match");

constexpr const char *qt_downsampler_quality_low = "low";
constexpr const char *qt_downsampler_quality_high = "high";
constexpr const char *qt_downsampler_quality_highest = "highest";

QString get_name_for_qt_downsampler_quality(qt_downsampler_quality quality);
qt_downsampler_quality get_qt_downsampler_quality_for_name(const QString &quality);



//---------------------------------------------------------
//
//   renderer
//
//---------------------------------------------------------

constexpr uint qt_video_frame_history_size = 4;



// post processing filter

enum class qt_filter : int
{
    none = 0,

    scale2x = 1,
    age_scale2x = 2,

    gauss3x3 = 3,
    gauss5x5 = 4,

    emboss3x3 = 5,
    emboss5x5 = 6
};

typedef std::vector<qt_filter> qt_filter_vector;

GLint get_qt_filter_factor(qt_filter filter);



} // namespace age



#endif // AGE_UI_QT_HPP
