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

#include <memory> // std::shared_ptr

#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

#include <age_non_copyable.hpp>
#include <age_speed_calculator.hpp>
#include <age_types.hpp>

#include "age_ui_qt_renderer.hpp"
#include "age_ui_qt_audio.hpp"
#include "age_ui_qt_emulator.hpp"



namespace age
{

//!
//! \brief The qt_emulation_runner runs an emulation and handles audio output.
//!
//! Can be passed to another thread for async emulation (slots triggered automatically
//! as events).
//!
class qt_emulation_runner : public QObject, non_copyable
{
    Q_OBJECT
public:

    qt_emulation_runner(qt_renderer &renderer, int emulation_interval_milliseconds);
    virtual ~qt_emulation_runner();

    uint get_speed_percent() const;
    uint64 get_emulated_milliseconds() const;

signals:

    void audio_output_activated(QAudioDeviceInfo device, QAudioFormat format, int buffer_size, int downsampler_fir_size);

public slots:

    // interesting read about threaded signals & slots:
    // https://woboq.com/blog/how-qt-signals-slots-work-part3-queuedconnection.html

    void initialize();

    void set_emulator(std::shared_ptr<age::qt_emulator> new_emulator);
    void set_emulator_buttons_down(uint buttons);
    void set_emulator_buttons_up(uint buttons);

    void set_emulator_synchronize(bool synchronize);
    void set_emulator_paused(bool paused);

    void set_audio_output(QAudioDeviceInfo device, QAudioFormat format);
    void set_audio_volume(int volume_percent);
    void set_audio_latency(int latency_milliseconds);
    void set_audio_downsampler_quality(age::qt_downsampler_quality quality);



private slots:

    void timer_event();

private:

    void emulate(std::shared_ptr<emulator> emu);
    void set_emulation_timer_interval();
    void emit_audio_output_activated();

    // usable by multiple threads

    atomic_uint m_speed_percent = {0};
    atomic_uint64 m_emulation_timer_cycles = {0};

    // used only by event handling thread

    const int m_emulation_interval_milliseconds;
    const uint64 m_emulation_interval_nanos;
    QElapsedTimer m_timer;
    QTimer *m_emulation_event_trigger = nullptr;

    qt_renderer &m_renderer;
    qt_audio_output m_audio_output;
    int m_audio_latency_milliseconds = qt_audio_latency_milliseconds_min;
    bool m_audio_latency_changed = false;

    bool m_synchronize = true;
    bool m_paused = false;
    uint64 m_last_timer_nanos = 0;
    speed_calculator<50> m_speed_calculator;

    std::shared_ptr<qt_emulator> m_emulator;
    uint m_buttons_down = 0;
    uint m_buttons_up = 0;
};

} // namespace age



#endif // AGE_UI_QT_EMULATION_RUNNER_HPP
