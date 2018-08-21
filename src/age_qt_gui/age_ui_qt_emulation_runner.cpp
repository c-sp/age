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

#include <algorithm>

#include <age_debug.hpp>

#include "age_ui_qt_emulation_runner.hpp"

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif



//---------------------------------------------------------
//
//   constructor & destructor
//
//---------------------------------------------------------

age::qt_emulation_runner::qt_emulation_runner(int emulation_interval_milliseconds)
    : QObject(),
      m_emulation_interval_milliseconds(emulation_interval_milliseconds),
      m_emulation_interval_nanos(static_cast<uint64>(m_emulation_interval_milliseconds * 1000000))
{
    AGE_ASSERT(m_emulation_interval_milliseconds > 0);

    LOG("emulation interval: " << m_emulation_interval_milliseconds
        << " milliseconds (" << m_emulation_interval_nanos << " nanos)");

    m_timer.start();
}

age::qt_emulation_runner::~qt_emulation_runner()
{
    // if the emulation timer has been activated, disconnect the timer event to prevent running
    // into a timer_event() during object destruction (which may trigger a seg fault)
    if (m_emulation_event_trigger != nullptr)
    {
        disconnect(m_emulation_event_trigger, SIGNAL(timeout()), this, SLOT(timer_event()));
    }
}



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

age::uint age::qt_emulation_runner::get_speed_percent() const
{
    return m_speed_percent;
}

age::uint64 age::qt_emulation_runner::get_emulated_milliseconds() const
{
    return m_emulation_timer_cycles / 1000000;
}



//---------------------------------------------------------
//
//   public slots
//
//---------------------------------------------------------

void age::qt_emulation_runner::initialize()
{
    // create a timer and connect the timeout signal to this object's timer_event() method
    // (this object takes ownership of the timer so we don't have to care about it's destruction)
    m_emulation_event_trigger = new QTimer(this);
    connect(m_emulation_event_trigger, SIGNAL(timeout()), this, SLOT(timer_event()));

    // start continuous emulation
    m_emulation_event_trigger->setTimerType(Qt::PreciseTimer); // if possible, use millisecond accuracy
    m_emulation_event_trigger->start(m_emulation_interval_milliseconds);

    LOG("started emulation timer with interval of " << m_emulation_interval_milliseconds << " millisecond(s)");
}



void age::qt_emulation_runner::set_emulator(std::shared_ptr<qt_emulator> new_emulator)
{
    LOG("");

    AGE_ASSERT(new_emulator != nullptr);
    std::shared_ptr<emulator> emu = new_emulator->get_emulator();
    AGE_ASSERT(emu != nullptr);

    m_speed_percent = 0;
    m_emulation_timer_cycles = 0;

    m_audio_output.set_input_sampling_rate(emu->get_pcm_sampling_rate());

    m_last_timer_nanos = m_timer.nsecsElapsed();
    m_speed_calculator.clear();

    m_emulator = new_emulator;
    m_buttons_down = 0;
    m_buttons_up = 0;
}

void age::qt_emulation_runner::set_emulator_buttons_down(uint buttons)
{
    LOG(buttons);
    m_buttons_down |= buttons;
}

void age::qt_emulation_runner::set_emulator_buttons_up(uint buttons)
{
    LOG(buttons);
    m_buttons_up |= buttons;
}



void age::qt_emulation_runner::set_emulator_synchronize(bool synchronize)
{
    LOG(synchronize);

    if (m_synchronize != synchronize)
    {
        m_synchronize = synchronize;
        set_emulation_timer_interval();
    }
}

void age::qt_emulation_runner::set_emulator_paused(bool paused)
{
    LOG(paused);

    if (m_paused != paused)
    {
        m_paused = paused;

        if (m_paused)
        {
            m_speed_percent = 0;
            m_speed_calculator.clear();
        }
        else
        {
            m_last_timer_nanos = m_timer.nsecsElapsed();
        }

        set_emulation_timer_interval();
    }
}



void age::qt_emulation_runner::set_audio_output(QAudioDeviceInfo device, QAudioFormat format)
{
    LOG(device.deviceName() << " @ " << format.sampleRate() << " hz");
    m_audio_output.set_output(device, format);
    emit_audio_output_activated();
}

void age::qt_emulation_runner::set_audio_volume(int volume_percent)
{
    LOG(volume_percent);
    m_audio_output.set_volume(volume_percent);
}

void age::qt_emulation_runner::set_audio_latency(int latency_milliseconds)
{
    LOG(latency_milliseconds);

    // We don't change this value immediately since it triggers an expensive
    // audio device reset every time. Instead we just store the changed value,
    // essentially compacting multiple events into one.
    m_audio_latency_milliseconds = latency_milliseconds;
    m_audio_latency_changed = true;
}

void age::qt_emulation_runner::set_audio_downsampler_quality(age::qt_downsampler_quality quality)
{
    LOG("");
    m_audio_output.set_downsampler_quality(quality);
    emit_audio_output_activated();
}



//---------------------------------------------------------
//
//   private slots
//
//---------------------------------------------------------

void age::qt_emulation_runner::timer_event()
{
    // check for changede audio settings
    if (m_audio_latency_changed)
    {
        m_audio_latency_changed = false;
        m_audio_output.set_latency(m_audio_latency_milliseconds);
        emit_audio_output_activated();
    }

    // run the emulation, if we have an emulator available
    bool buffer_audio_silence = true;

    if (m_emulator != nullptr)
    {
        std::shared_ptr<emulator> emu = m_emulator->get_emulator();

        // handle "button down" events even when paused
        emu->set_buttons_down(m_buttons_down);
        m_buttons_down = 0;

        // run the emulation, if we're not paused
        if (!m_paused)
        {
            emulate(emu);
            buffer_audio_silence = false;
        }

        // handle "button up" events even when paused
        emu->set_buttons_up(m_buttons_up);
        m_buttons_up = 0;
    }

    // Keep the audio stream alive. If no new audio data is available
    // (e.g. because the emulation is paused), allow silent samples
    // to be streamed.
    if (buffer_audio_silence)
    {
        m_audio_output.buffer_silence();
    }
    m_audio_output.stream_audio_data();
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

void age::qt_emulation_runner::emulate(std::shared_ptr<emulator> emu)
{
    uint64 current_timer_nanos = m_timer.nsecsElapsed();
    uint64 timer_nanos_elapsed = current_timer_nanos - m_last_timer_nanos;
    uint64 timer_nanos_to_add = m_synchronize ? timer_nanos_elapsed : m_emulation_interval_nanos;

    // limit the cycles to emulate to not stall the system in case the CPU can't keep up
    // (use a multiple of m_emulation_interval_nanos to allow evening out load spikes)
    timer_nanos_to_add = std::min(timer_nanos_to_add, m_emulation_interval_nanos << 1);

    // emulate
    uint64 emulated_cycles = emu->get_emulated_cycles();
    uint64 emulation_timer_nanos = (m_emulation_timer_cycles += timer_nanos_to_add);
    uint64 emulated_cycles_to_go = emulation_timer_nanos * emu->get_cycles_per_second() / 1000000000;

    bool new_frame = false;
    if (emulated_cycles_to_go > emulated_cycles)
    {
        uint64 cycles_to_emulate = emulated_cycles_to_go - emulated_cycles;
        new_frame = emu->emulate(cycles_to_emulate);

        // calculate emulation speed & time
        uint cycles_emulated = static_cast<uint>(emu->get_emulated_cycles() - emulated_cycles);
        m_speed_calculator.add_value(cycles_emulated, timer_nanos_elapsed);
        m_speed_percent = m_speed_calculator.get_speed_percent(emu->get_cycles_per_second(), 1000000000);
    }

    // update video & audio
    if (new_frame)
    {
        // copy screen buffer since the emulator will overwrite it eventually
        std::shared_ptr<age::pixel_vector> screen = std::make_shared<pixel_vector>(emu->get_screen_front_buffer());
        emit emulator_screen_updated(screen);
    }
    m_audio_output.buffer_samples(emu->get_audio_buffer());

    // store the current nanos as reference for future emulate() calls
    m_last_timer_nanos = current_timer_nanos;
}



void age::qt_emulation_runner::set_emulation_timer_interval()
{
    int millis = (!m_paused && !m_synchronize) ? 0 : m_emulation_interval_milliseconds;
    m_emulation_event_trigger->setInterval(millis);
}



void age::qt_emulation_runner::emit_audio_output_activated()
{
    QAudioDeviceInfo device_info = m_audio_output.get_device_info();
    QAudioFormat format = m_audio_output.get_format();
    int buffer_size = m_audio_output.get_buffer_size();
    int downsampler_fir_size = static_cast<int>(m_audio_output.get_downsampler_fir_size());

    LOG(device_info.deviceName() << ", " << format.sampleRate() << ", " << buffer_size << " bytes");
    emit audio_output_activated(device_info, format, buffer_size, downsampler_fir_size);
}
