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

#include <age_debug.hpp>

#include "age_ui_qt_emulation_runner.hpp"

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif

namespace
{

    constexpr int    emulation_interval_millis = 8;
    constexpr qint64 emulation_interval_nanos  = emulation_interval_millis * 1000000;

    static_assert(emulation_interval_millis > 0, "the emulation interval must be greater than zero");

    constexpr qint64 emulation_speed_interval_nanos = 1000000000 / age::stats_per_second;

} // namespace





//---------------------------------------------------------
//
//   constructor & destructor
//
//---------------------------------------------------------

age::qt_emulation_runner::qt_emulation_runner()
{
    LOG("emulation interval: " << emulation_interval_millis
                               << " milliseconds (" << emulation_interval_nanos << " nanos)");

    m_timer.start();
}

age::qt_emulation_runner::~qt_emulation_runner()
{
    // if the emulation timer has been activated, disconnect the timer event to prevent running
    // into a timer_event() during object destruction (which may trigger a seg fault)
    if (m_emulation_event_trigger != nullptr)
    {
        disconnect(m_emulation_event_trigger, &QTimer::timeout, this, &qt_emulation_runner::continue_emulation);
    }
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
    connect(m_emulation_event_trigger, &QTimer::timeout, this, &qt_emulation_runner::continue_emulation);

    // start continuous emulation
    m_emulation_event_trigger->setTimerType(Qt::PreciseTimer); // if possible, use millisecond accuracy
    m_emulation_event_trigger->start(emulation_interval_millis);

    LOG("started emulation timer with interval of " << emulation_interval_millis << " millisecond(s)");
}



void age::qt_emulation_runner::set_emulator(QSharedPointer<qt_emulator> new_emulator)
{
    LOG("");

    AGE_ASSERT(new_emulator != nullptr)
    auto emu = new_emulator->get_emulator();
    AGE_ASSERT(emu != nullptr)

    m_audio_output.set_input_sampling_rate(emu->get_pcm_sampling_rate());

    m_last_emulate_nanos = m_speed_last_nanos = m_timer.nsecsElapsed();
    m_emulated_cycles = m_speed_last_cycles = 0;

    m_emulator     = new_emulator;
    m_buttons_down = 0;
    m_buttons_up   = 0;
}

void age::qt_emulation_runner::set_emulator_buttons_down(int buttons)
{
    LOG(buttons);
    m_buttons_down |= buttons;
}

void age::qt_emulation_runner::set_emulator_buttons_up(int buttons)
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
            emit emulator_speed(0);
        }
        else
        {
            m_last_emulate_nanos = m_speed_last_nanos = m_timer.nsecsElapsed();
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
    m_audio_latency_changed      = true;
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

void age::qt_emulation_runner::continue_emulation()
{
    // check for modified audio settings
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
        auto emu = m_emulator->get_emulator();

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

void age::qt_emulation_runner::emulate(QSharedPointer<gb_emulator> emu)
{
    qint64 current_timer_nanos = m_timer.nsecsElapsed();
    qint64 timer_nanos_elapsed = current_timer_nanos - m_last_emulate_nanos;

    // Calculate the number of nanoseconds to emulate based on elapsed time.
    // Limit the nanos to emulate in order to not stall the system if the CPU cannot keep up
    // (allow evening out load spikes though, requiring a limit greater than emulation_interval_nanos).
    qint64 nanos_to_emulate = m_synchronize ? timer_nanos_elapsed : qint64_max;
    nanos_to_emulate        = qMin(nanos_to_emulate, emulation_interval_nanos * 2);
    LOG("nanos_to_emulate " << nanos_to_emulate);

    // convert nanoseconds to emulation cycles
    AGE_ASSERT(qint64_max / nanos_to_emulate > emu->get_cycles_per_second())
    qint64 cycles_to_emulate = nanos_to_emulate * emu->get_cycles_per_second() / 1000000000;

    // save the nanoseconds that make up just a fraction of an emulation cycle
    // for the next emulate() iteration
    m_last_emulate_nanos = current_timer_nanos - (nanos_to_emulate - cycles_to_emulate * 1000000000 / emu->get_cycles_per_second());
    AGE_ASSERT(m_last_emulate_nanos <= current_timer_nanos)
    AGE_ASSERT(m_last_emulate_nanos >= current_timer_nanos - 1000000000 / emu->get_cycles_per_second() - 1)

    // Calculate the number of emulation cycles we should have reached by now.
    // Since the last call to emulator::emulate() may have emulated more cycles
    // than requested, we have to do this to keep the emulation synchronous.
    m_emulated_cycles += cycles_to_emulate;
    qint64 cycles_left = m_emulated_cycles - emu->get_emulated_cycles();
    LOG("cycles_left " << cycles_left);
    if (cycles_left <= 0)
    {
        return;
    }

    AGE_ASSERT(cycles_left <= int_max)
    bool new_frame = emu->emulate(static_cast<int>(cycles_left));
    AGE_ASSERT(emu->get_emulated_cycles() >= m_emulated_cycles)

    // update video & audio
    if (new_frame)
    {
        // copy screen buffer since the emulator will overwrite it eventually
        QSharedPointer<age::pixel_vector> screen = QSharedPointer<pixel_vector>(new pixel_vector(emu->get_screen_front_buffer()));
        emit                              emulator_screen_updated(screen);
    }
    m_audio_output.buffer_samples(emu->get_audio_buffer());

    // calculate emulation speed
    AGE_ASSERT(current_timer_nanos >= m_speed_last_nanos)
    qint64 speed_diff_nanos = current_timer_nanos - m_speed_last_nanos;

    if (speed_diff_nanos >= emulation_speed_interval_nanos)
    {
        qint64 speed_diff_cycles = m_emulated_cycles - m_speed_last_cycles;

        auto   nanos           = speed_diff_nanos * emu->get_cycles_per_second();
        auto   cycles          = speed_diff_cycles * 1000000000;
        double speed_percent   = qRound(cycles * 100.0 / nanos);
        auto   emulated_millis = m_emulated_cycles * 1000 / emu->get_cycles_per_second();

        emit emulator_speed(static_cast<int>(speed_percent));
        emit emulator_milliseconds(emulated_millis);

        m_speed_last_nanos  = current_timer_nanos;
        m_speed_last_cycles = m_emulated_cycles;
    }
}



void age::qt_emulation_runner::set_emulation_timer_interval()
{
    int millis = (!m_paused && !m_synchronize) ? 0 : emulation_interval_millis;
    m_emulation_event_trigger->setInterval(millis);
}



void age::qt_emulation_runner::emit_audio_output_activated()
{
    QAudioDeviceInfo device_info          = m_audio_output.get_device_info();
    QAudioFormat     format               = m_audio_output.get_format();
    int              buffer_size          = m_audio_output.get_buffer_size();
    int              downsampler_fir_size = static_cast<int>(m_audio_output.get_downsampler_fir_size());

    LOG(device_info.deviceName() << ", " << format.sampleRate() << ", " << buffer_size << " bytes");
    emit audio_output_activated(device_info, format, buffer_size, downsampler_fir_size);
}
