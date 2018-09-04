//
// Copyright 2018 Christoph Sprenger
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

#ifndef AGE_UI_QT_HPP
#define AGE_UI_QT_HPP

//!
//! \file
//!

#include <ostream>
#include <string>
#include <vector>

#include <QList>
#include <QString>
#include <QtGui/qopengl.h> // GLint

#include <age_types.hpp>



// The following operators must be declared outside of any namespace.
std::ostream& operator<<(std::ostream &stream, const QString &string);
std::string operator+(const std::string &std_string, const QString &q_string);



namespace age
{

bool is_checked(int checked_state);

constexpr int stats_per_second = 4;



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
//!
constexpr int max_file_bytes = 32 * 1024 * 1024;



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

constexpr int qt_video_frame_history_size = 4;

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

typedef QList<qt_filter> qt_filter_list;

GLint get_qt_filter_factor(qt_filter filter);



} // namespace age



#endif // AGE_UI_QT_HPP
