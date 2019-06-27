//
// Copyright 2019 Christoph Sprenger
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

#include <QKeyEvent>
#include <QTabWidget>
#include <QVBoxLayout>

#include "age_ui_qt_settings.hpp"

constexpr const char *qt_settings_open_file_directory = "open_file_directory";





//---------------------------------------------------------
//
//   Object creation/destruction
//
//---------------------------------------------------------

age::qt_settings_dialog::qt_settings_dialog(QSharedPointer<qt_user_value_store> user_value_store, QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_user_value_store(user_value_store)
{
    setWindowTitle(QString(project_name) + " settings");

    // create tab widget

    m_settings_video = new qt_settings_video(m_user_value_store);
    m_settings_audio = new qt_settings_audio(m_user_value_store);
    m_settings_keys = new qt_settings_keys(m_user_value_store);
    m_settings_miscellaneous = new qt_settings_miscellaneous(m_user_value_store);

    QTabWidget *tab_widget = new QTabWidget;
    tab_widget->addTab(m_settings_video, "video");
    tab_widget->addTab(m_settings_audio, "audio");
    tab_widget->addTab(m_settings_keys, "keys");
    tab_widget->addTab(m_settings_miscellaneous, "miscellaneous");

    // set the dialog's layout

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tab_widget);
    setLayout(layout);

    // connect signals

    connect(m_settings_video, SIGNAL(use_bilinear_filter_changed(bool)), this, SLOT(emit_video_use_bilinear_filter_changed(bool)));
    connect(m_settings_video, SIGNAL(frames_to_blend_changed(int)), this, SLOT(emit_video_frames_to_blend_changed(int)));
    connect(m_settings_video, SIGNAL(filter_chain_changed(qt_filter_list)), this, SLOT(emit_video_post_processing_filter_changed(qt_filter_list)));

    connect(m_settings_audio, SIGNAL(output_changed(QAudioDeviceInfo,QAudioFormat)), this, SLOT(emit_audio_output_changed(QAudioDeviceInfo,QAudioFormat)));
    connect(m_settings_audio, SIGNAL(volume_changed(int)), this, SLOT(emit_audio_volume_changed(int)));
    connect(m_settings_audio, SIGNAL(latency_changed(int)), this, SLOT(emit_audio_latency_changed(int)));
    connect(m_settings_audio, SIGNAL(downsampler_quality_changed(age::qt_downsampler_quality)), this, SLOT(emit_audio_downsampler_quality_changed(age::qt_downsampler_quality)));

    connect(m_settings_miscellaneous, SIGNAL(pause_emulator_changed(bool)), this, SLOT(emit_misc_pause_emulator_changed(bool)));
    connect(m_settings_miscellaneous, SIGNAL(synchronize_emulator_changed(bool)), this, SLOT(emit_misc_synchronize_emulator_changed(bool)));
    connect(m_settings_miscellaneous, SIGNAL(show_menu_bar_changed(bool)), this, SLOT(emit_misc_show_menu_bar_changed(bool)));
    connect(m_settings_miscellaneous, SIGNAL(show_status_bar_changed(bool)), this, SLOT(emit_misc_show_status_bar_changed(bool)));
    connect(m_settings_miscellaneous, SIGNAL(show_menu_bar_fullscreen_changed(bool)), this, SLOT(emit_misc_show_menu_bar_fullscreen_changed(bool)));
    connect(m_settings_miscellaneous, SIGNAL(show_status_bar_fullscreen_changed(bool)), this, SLOT(emit_misc_show_status_bar_fullscreen_changed(bool)));
}





//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

age::qt_key_event age::qt_settings_dialog::get_event_for_key(Qt::Key key) const
{
    qt_key_event event = m_settings_keys->get_event_for_key(key);
    return event;
}

QString age::qt_settings_dialog::get_open_file_dialog_directory() const
{
    QString directory = m_user_value_store->get_value(qt_settings_open_file_directory, "").toString();
    return directory;
}

bool age::qt_settings_dialog::show_menu_bar(bool fullscreen) const
{
    return m_settings_miscellaneous->show_menu_bar(fullscreen);
}

bool age::qt_settings_dialog::show_status_bar(bool fullscreen) const
{
    return m_settings_miscellaneous->show_status_bar(fullscreen);
}



void age::qt_settings_dialog::set_open_file_dialog_directory(const QString &directory)
{
    m_user_value_store->set_value(qt_settings_open_file_directory, directory);
}

void age::qt_settings_dialog::set_pause_emulator(bool pause_emulator)
{
    m_settings_miscellaneous->set_pause_emulator(pause_emulator);
}



bool age::qt_settings_dialog::trigger_settings_event(qt_key_event event)
{
    bool result = true;

    switch (event)
    {
    case qt_key_event::video_toggle_filter_chain:
        m_settings_video->toggle_filter_chain();
        break;

    case qt_key_event::video_toggle_bilinear_filter:
        m_settings_video->toggle_bilinear_filter();
        break;

    case qt_key_event::video_cycle_frames_to_blend:
        m_settings_video->cycle_frames_to_blend();
        break;

    case qt_key_event::audio_toggle_mute:
        m_settings_audio->toggle_mute();
        break;

    case qt_key_event::audio_increase_volume:
        m_settings_audio->increase_volume();
        break;

    case qt_key_event::audio_decrease_volume:
        m_settings_audio->decrease_volume();
        break;

    case qt_key_event::misc_toggle_pause_emulator:
        m_settings_miscellaneous->toggle_pause_emulator();
        break;

    case qt_key_event::misc_toggle_synchronize_emulator:
        m_settings_miscellaneous->toggle_synchronize_emulator();
        break;

    default:
        result = false;
        break;
    }

    return result;
}

void age::qt_settings_dialog::emit_settings_signals()
{
    m_settings_video->emit_settings_signals();
    m_settings_audio->emit_settings_signals();
    m_settings_miscellaneous->emit_settings_signals();
}





//---------------------------------------------------------
//
//   public slots
//
//---------------------------------------------------------

void age::qt_settings_dialog::audio_output_activated(QAudioDeviceInfo device, QAudioFormat format, int buffer_size, int downsampler_fir_size)
{
    m_settings_audio->set_active_audio_output(device, format, buffer_size, downsampler_fir_size);
}

void age::qt_settings_dialog::set_emulator_screen_size(int16_t width, int16_t height)
{
    m_settings_video->set_emulator_screen_size(width, height);
}





//---------------------------------------------------------
//
//   protected methods
//
//---------------------------------------------------------

void age::qt_settings_dialog::keyPressEvent(QKeyEvent *key_event)
{
    // check, if this key may trigger a settings event
    Qt::Key qt_key = static_cast<Qt::Key>(key_event->key());
    qt_key_event event = get_event_for_key(qt_key);
    bool triggered_settings = trigger_settings_event(event);

    // if not, let QDialog process the keyPressEvent
    if (!triggered_settings)
    {
        QDialog::keyPressEvent(key_event);
    }
}



void age::qt_settings_dialog::closeEvent(QCloseEvent *close_event)
{
    m_user_value_store->sync();
    QDialog::closeEvent(close_event);
}





//---------------------------------------------------------
//
//   private slots
//
//---------------------------------------------------------

void age::qt_settings_dialog::emit_video_use_bilinear_filter_changed(bool use)
{
    emit video_use_bilinear_filter_changed(use);
}

void age::qt_settings_dialog::emit_video_frames_to_blend_changed(int frames_to_blend)
{
    emit video_frames_to_blend_changed(frames_to_blend);
}

void age::qt_settings_dialog::emit_video_post_processing_filter_changed(qt_filter_list filter_list)
{
    emit video_post_processing_filter_changed(filter_list);
}



void age::qt_settings_dialog::emit_audio_output_changed(QAudioDeviceInfo device, QAudioFormat format)
{
    emit audio_output_changed(device, format);
}

void age::qt_settings_dialog::emit_audio_volume_changed(int volume_percent)
{
    emit audio_volume_changed(volume_percent);
}

void age::qt_settings_dialog::emit_audio_latency_changed(int latency_milliseconds)
{
    emit audio_latency_changed(latency_milliseconds);
}

void age::qt_settings_dialog::emit_audio_downsampler_quality_changed(age::qt_downsampler_quality quality)
{
    emit audio_downsampler_quality_changed(quality);
}



void age::qt_settings_dialog::emit_misc_pause_emulator_changed(bool pause_emulator)
{
    emit misc_pause_emulator_changed(pause_emulator);
}

void age::qt_settings_dialog::emit_misc_synchronize_emulator_changed(bool synchronize_emulator)
{
    emit misc_synchronize_emulator_changed(synchronize_emulator);
}

void age::qt_settings_dialog::emit_misc_show_menu_bar_changed(bool show_menu_bar)
{
    emit misc_show_menu_bar_changed(show_menu_bar);
}

void age::qt_settings_dialog::emit_misc_show_status_bar_changed(bool show_status_bar)
{
    emit misc_show_status_bar_changed(show_status_bar);
}

void age::qt_settings_dialog::emit_misc_show_menu_bar_fullscreen_changed(bool show_menu_bar_fullscreen)
{
    emit misc_show_menu_bar_fullscreen_changed(show_menu_bar_fullscreen);
}

void age::qt_settings_dialog::emit_misc_show_status_bar_fullscreen_changed(bool show_status_bar_fullscreen)
{
    emit misc_show_status_bar_fullscreen_changed(show_status_bar_fullscreen);
}
