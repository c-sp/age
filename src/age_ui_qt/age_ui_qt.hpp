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
#include <QtGui/qopengl.h> // GLenum, GLint

#include <age_types.hpp>



// The following operators must be declared outside of any namespace.
std::ostream& operator<<(std::ostream &stream, const QString &string);
std::string operator+(const std::string &std_string, const QString &q_string);



namespace age
{

typedef std::atomic_size_t  atomic_uint;
typedef std::atomic<uint64> atomic_uint64;

//!
//! The num contains all available key events.
//! Examples of key events are setting adjustments (e.g. increasing the audio volume)
//! or pressing a emulator specific button (e.g. pressing the Gameboy's "start" button).
//!
//! The value qt_key_event::none does not represent any actual key event.
//! It is used for example when parsing a key event from a string fails because the
//! string is not parsable.
//!
enum class qt_key_event
{
    none,

    gb_up,
    gb_down,
    gb_left,
    gb_right,
    gb_a,
    gb_b,
    gb_start,
    gb_select,

    video_toggle_filter_chain,
    video_toggle_bilinear_filter,
    video_cycle_frames_to_blend,

    audio_toggle_mute,
    audio_increase_volume,
    audio_decrease_volume,

    misc_toggle_pause_emulator,
    misc_toggle_synchronize_emulator
};



bool is_checked(int checked_state);





//---------------------------------------------------------
//
//   settings
//
//---------------------------------------------------------

constexpr const char *user_value_directory = ".age_emulator";

//!
//! \brief The maximal size in bytes of files allowed to be handled.
//!
//! While in theory we could handle bigger files, it does not make sense for our use cases.
//! There are no cartridge or RAM files bigger than this.
//! In fact it could even lower performance and stability to try loading such big files
//! (think of a RAM file of several gigabytes being loaded into memory).
//!
constexpr qint64 max_file_bytes = 32 * 1024 * 1024;

constexpr int qt_settings_layout_margin = 30;
constexpr int qt_settings_layout_spacing = 30;
constexpr int qt_settings_element_spacing = 15;

constexpr const char *qt_settings_default_audio_device_name = "default";
constexpr const char *qt_settings_dnd_mime_type = "application/x-age-filter-dnd";
constexpr const char *qt_settings_property_frames_to_blend = "age_frames_to_blend";
constexpr const char *qt_settings_property_filter = "age_filter";

// settings value keys

constexpr const char *qt_settings_open_file_directory = "open_file_directory";

constexpr const char *qt_settings_video_use_filter_chain = "video/use_filter_chain";
constexpr const char *qt_settings_video_bilinear_filter = "video/bilinear_filter";
constexpr const char *qt_settings_video_frames_to_blend = "video/frames_to_blend";
constexpr const char *qt_settings_video_filter_chain = "video/filter_chain";

constexpr const char *qt_settings_audio_device = "audio/device";
constexpr const char *qt_settings_audio_volume = "audio/volume";
constexpr const char *qt_settings_audio_mute = "audio/mute";
constexpr const char *qt_settings_audio_latency = "audio/latency";
constexpr const char *qt_settings_audio_downsampler_quality = "audio/downsampler_quality";

constexpr const char *qt_settings_keys_gameboy_up = "keys/gameboy_up";
constexpr const char *qt_settings_keys_gameboy_down = "keys/gameboy_down";
constexpr const char *qt_settings_keys_gameboy_left = "keys/gameboy_left";
constexpr const char *qt_settings_keys_gameboy_right = "keys/gameboy_right";
constexpr const char *qt_settings_keys_gameboy_a = "keys/gameboy_a";
constexpr const char *qt_settings_keys_gameboy_b = "keys/gameboy_b";
constexpr const char *qt_settings_keys_gameboy_start = "keys/gameboy_start";
constexpr const char *qt_settings_keys_gameboy_select = "keys/gameboy_select";
constexpr const char *qt_settings_keys_video_toggle_filter_chain = "keys/video_toggle_filter_chain";
constexpr const char *qt_settings_keys_video_toggle_bilinear_filter = "keys/video_toggle_bilinear_filter";
constexpr const char *qt_settings_keys_video_cycle_frames_to_blend = "keys/video_cycle_frames_to_blend";
constexpr const char *qt_settings_keys_audio_toggle_mute = "keys/audio_toggle_mute";
constexpr const char *qt_settings_keys_audio_increase_volume = "keys/audio_increase_volume";
constexpr const char *qt_settings_keys_audio_decrease_volume = "keys/audio_decrease_volume";
constexpr const char *qt_settings_keys_misc_toggle_pause_emulator = "keys/misc_toggle_pause_emulator";
constexpr const char *qt_settings_keys_misc_toggle_synchronize_emulator = "keys/misc_toggle_synchronize_emulator";

constexpr const char *qt_settings_misc_pause_emulator = "miscellaneous/pause_emulator";
constexpr const char *qt_settings_misc_synchronize_emulator = "miscellaneous/synchronize_emulator";
constexpr const char *qt_settings_misc_menu_bar = "miscellaneous/menu_bar";
constexpr const char *qt_settings_misc_status_bar = "miscellaneous/status_bar";
constexpr const char *qt_settings_misc_menu_bar_fullscreen = "miscellaneous/menu_bar_fullscreen";
constexpr const char *qt_settings_misc_status_bar_fullscreen = "miscellaneous/status_bar_fullscreen";



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

constexpr int qt_audio_volume_percent_min = 0;
constexpr int qt_audio_volume_percent_max = 100;

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

constexpr uint qt_renderer_stopped = 1;
constexpr uint qt_renderer_new_video_frame = 2;
constexpr uint qt_renderer_change_filter_chain = 4;
constexpr uint qt_renderer_change_bilinear = 8;
constexpr uint qt_renderer_change_viewport = 16;
constexpr uint qt_renderer_change_emulator_screen_size = 32;
constexpr uint qt_renderer_change_frames_to_blend = 64;

constexpr uint qt_video_frame_history_size = 4;
constexpr qint64 qt_fps_update_millis = 500;
constexpr uint qt_num_filter_positions = 7;

constexpr int qt_min_gl_width  = 160;
constexpr int qt_min_gl_height = 144;



// post processing filter

constexpr const char *qt_filter_name_none = "none";
constexpr const char *qt_filter_name_scale2x = "scale2x";
constexpr const char *qt_filter_name_age_scale2x = "age-scale2x";
constexpr const char *qt_filter_name_gauss3x3 = "weak blur";
constexpr const char *qt_filter_name_gauss5x5 = "strong blur";
constexpr const char *qt_filter_name_emboss3x3 = "weak emboss";
constexpr const char *qt_filter_name_emboss5x5 = "strong emboss";

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



// texture format

constexpr GLenum qt_texture_format = GL_BGRA;
constexpr GLenum qt_texture_type = GL_UNSIGNED_BYTE;

} // namespace age



#endif // AGE_UI_QT_HPP
