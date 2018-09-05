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

#ifndef AGE_UI_QT_EMULATION_RUNNER_HPP
#define AGE_UI_QT_EMULATION_RUNNER_HPP

//!
//! \file
//!

#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

#include <age_types.hpp>

#include "age_ui_qt_audio.hpp"
#include "age_ui_qt_emulator.hpp"



namespace age
{

//!
//! \brief The qt_emulation_runner runs an emulation and handles audio output.
//!
//! This can be passed to another thread for asynchronous emulation.
//!
class qt_emulation_runner : public QObject
{
    Q_OBJECT
    AGE_DISABLE_COPY(qt_emulation_runner);

public:

    qt_emulation_runner();
    virtual ~qt_emulation_runner();

signals:

    void audio_output_activated(QAudioDeviceInfo device, QAudioFormat format, int buffer_size, int downsampler_fir_size);
    void emulator_screen_update(QSharedPointer<const age::pixel_vector> screen);
    void emulator_speed(int speed_percent);
    void emulator_milliseconds(qint64 emulated_milliseconds);

public slots:

    void initialize();

    void set_emulator(QSharedPointer<age::qt_emulator> new_emulator);
    void set_emulator_buttons_down(int buttons);
    void set_emulator_buttons_up(int buttons);

    void set_emulator_synchronize(bool synchronize);
    void set_emulator_paused(bool paused);

    void set_audio_output(QAudioDeviceInfo device, QAudioFormat format);
    void set_audio_volume(int volume_percent);
    void set_audio_latency(int latency_milliseconds);
    void set_audio_downsampler_quality(age::qt_downsampler_quality quality);



private slots:

    void timer_event();

private:

    void emulate(QSharedPointer<emulator> emu);
    void set_emulation_timer_interval();
    void emit_audio_output_activated();

    QTimer *m_emulation_event_trigger = nullptr;
    QElapsedTimer m_timer;
    qint64 m_last_emulate_nanos = 0;
    qint64 m_emulated_cycles = 0;
    qint64 m_speed_last_nanos = 0;
    qint64 m_speed_last_cycles = 0;
    bool m_synchronize = true;
    bool m_paused = false;

    qt_audio_output m_audio_output;
    int m_audio_latency_milliseconds = qt_audio_latency_milliseconds_min;
    bool m_audio_latency_changed = false;

    QSharedPointer<qt_emulator> m_emulator;
    int m_buttons_down = 0;
    int m_buttons_up = 0;
};

} // namespace age



#endif // AGE_UI_QT_EMULATION_RUNNER_HPP
