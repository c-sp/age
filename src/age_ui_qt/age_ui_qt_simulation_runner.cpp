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

#include "age_ui_qt_simulation_runner.hpp"

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

age::qt_simulation_runner::qt_simulation_runner(qt_gl_renderer &renderer, int simulation_interval_milliseconds)
    : QObject(),
      m_simulation_interval_milliseconds(simulation_interval_milliseconds),
      m_simulation_interval_ticks(static_cast<uint64>(m_simulation_interval_milliseconds * 1000000)),
      m_renderer(renderer)
{
    AGE_ASSERT(m_simulation_interval_milliseconds > 0);

    LOG("simulation interval: " << m_simulation_interval_milliseconds
        << " milliseconds (" << m_simulation_interval_ticks << " ticks)");

    m_timer.start();
}

age::qt_simulation_runner::~qt_simulation_runner()
{
    // if the simulation timer has been activated, disconnect the timer event to prevent running
    // into a timer_event() during object destruction (which may trigger a seg fault)
    if (m_simulation_event_trigger != nullptr)
    {
        disconnect(m_simulation_event_trigger, SIGNAL(timeout()), this, SLOT(timer_event()));
    }
}



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

age::uint age::qt_simulation_runner::get_speed_percent() const
{
    return m_speed_percent;
}

age::uint64 age::qt_simulation_runner::get_simulated_milliseconds() const
{
    return m_simulation_timer_ticks / 1000000;
}



//---------------------------------------------------------
//
//   public slots
//
//---------------------------------------------------------

void age::qt_simulation_runner::initialize()
{
    // create a timer and connect the timeout signal to this object's timer_event() method
    // (this object takes ownership of the timer so we don't have to care about it's destruction)
    m_simulation_event_trigger = new QTimer(this);
    connect(m_simulation_event_trigger, SIGNAL(timeout()), this, SLOT(timer_event()));

    // start continuous simulation
    m_simulation_event_trigger->setTimerType(Qt::PreciseTimer); // if possible, use millisecond accuracy
    m_simulation_event_trigger->start(m_simulation_interval_milliseconds);

    LOG("started simulation timer with interval of " << m_simulation_interval_milliseconds << " millisecond(s)");
}



void age::qt_simulation_runner::set_simulator(std::shared_ptr<qt_simulator> new_simulator)
{
    LOG("");

    AGE_ASSERT(new_simulator != nullptr);
    std::shared_ptr<simulator> sim = new_simulator->get_simulator();
    AGE_ASSERT(sim != nullptr);

    m_speed_percent = 0;
    m_simulation_timer_ticks = 0;

    m_renderer.set_simulator_screen_size(sim->get_screen_width(), sim->get_screen_height());
    m_audio_output.set_input_sampling_rate(sim->get_pcm_sampling_rate());

    m_last_timer_ticks = m_timer.nsecsElapsed();
    m_speed_calculator.clear();

    m_simulator = new_simulator;
    m_buttons_down = 0;
    m_buttons_up = 0;
}

void age::qt_simulation_runner::set_simulator_buttons_down(uint buttons)
{
    LOG(buttons);
    m_buttons_down |= buttons;
}

void age::qt_simulation_runner::set_simulator_buttons_up(uint buttons)
{
    LOG(buttons);
    m_buttons_up |= buttons;
}



void age::qt_simulation_runner::set_simulation_synchronize(bool synchronize)
{
    LOG(synchronize);

    if (m_synchronize != synchronize)
    {
        m_synchronize = synchronize;
        set_simulation_timer_interval();
    }
}

void age::qt_simulation_runner::set_simulation_paused(bool paused)
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
            m_last_timer_ticks = m_timer.nsecsElapsed();
        }

        set_simulation_timer_interval();
    }
}



void age::qt_simulation_runner::set_audio_output(QAudioDeviceInfo device, QAudioFormat format)
{
    LOG(device.deviceName() << " @ " << format.sampleRate() << " hz");
    m_audio_output.set_output(device, format);
    emit_audio_output_activated();
}

void age::qt_simulation_runner::set_audio_volume(int volume_percent)
{
    LOG(volume_percent);
    m_audio_output.set_volume(volume_percent);
}

void age::qt_simulation_runner::set_audio_latency(int latency_milliseconds)
{
    LOG(latency_milliseconds);

    // We don't change this value immediately since it triggers an expensive
    // audio device reset every time. Instead we just store the changed value,
    // essentially compacting multiple events into one.
    m_audio_latency_milliseconds = latency_milliseconds;
    m_audio_latency_changed = true;
}

void age::qt_simulation_runner::set_audio_downsampler_quality(age::qt_downsampler_quality quality)
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

void age::qt_simulation_runner::timer_event()
{
    // check for changede audio settings
    if (m_audio_latency_changed)
    {
        m_audio_latency_changed = false;
        m_audio_output.set_latency(m_audio_latency_milliseconds);
        emit_audio_output_activated();
    }

    // run the simulation, if we have a simulator available
    bool buffer_audio_silence = true;

    if (m_simulator != nullptr)
    {
        std::shared_ptr<simulator> sim = m_simulator->get_simulator();

        // handle "button down" events even when paused
        sim->set_buttons_down(m_buttons_down);
        m_buttons_down = 0;

        // run simulation, if we're not paused
        if (!m_paused)
        {
            simulate(sim);
            buffer_audio_silence = false;
        }

        // handle "button up" events even when paused
        sim->set_buttons_up(m_buttons_up);
        m_buttons_up = 0;
    }

    // Keep the audio stream alive. If no new audio data is available
    // (e.g. because the simulation is paused), allow silent samples
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

void age::qt_simulation_runner::simulate(std::shared_ptr<simulator> sim)
{
    uint64 current_timer_ticks = m_timer.nsecsElapsed();
    uint64 timer_ticks_elapsed = current_timer_ticks - m_last_timer_ticks;
    uint64 timer_ticks_to_add = m_synchronize ? timer_ticks_elapsed : m_simulation_interval_ticks;

    // limit the ticks to simulate to not stall the system in case the CPU can't keep up
    // (use a multiple of m_simulation_interval_ticks to allow evening out load spikes)
    timer_ticks_to_add = std::min(timer_ticks_to_add, m_simulation_interval_ticks << 1);

    // simulate
    uint64 simulated_ticks = sim->get_simulated_ticks();
    uint64 simulation_timer_ticks = (m_simulation_timer_ticks += timer_ticks_to_add);
    uint64 simulated_ticks_to_go = simulation_timer_ticks * sim->get_ticks_per_second() / 1000000000;

    bool new_frame = false;
    if (simulated_ticks_to_go > simulated_ticks)
    {
        uint64 ticks_to_simulate = simulated_ticks_to_go - simulated_ticks;
        new_frame = sim->simulate(ticks_to_simulate);

        // calculate simulation speed & time
        uint ticks_simulated = static_cast<uint>(sim->get_simulated_ticks() - simulated_ticks);
        m_speed_calculator.add_value(ticks_simulated, timer_ticks_elapsed);
        m_speed_percent = m_speed_calculator.get_speed_percent(sim->get_ticks_per_second(), 1000000000);
    }

    // update video & audio
    if (new_frame)
    {
        m_renderer.add_video_frame(sim->get_video_front_buffer());
    }
    m_audio_output.buffer_samples(sim->get_audio_buffer());

    // store the current ticks as reference for future simulate() calls
    m_last_timer_ticks = current_timer_ticks;
}



void age::qt_simulation_runner::set_simulation_timer_interval()
{
    int millis = (!m_paused && !m_synchronize) ? 0 : m_simulation_interval_milliseconds;
    m_simulation_event_trigger->setInterval(millis);
}



void age::qt_simulation_runner::emit_audio_output_activated()
{
    QAudioDeviceInfo device_info = m_audio_output.get_device_info();
    QAudioFormat format = m_audio_output.get_format();
    int buffer_size = m_audio_output.get_buffer_size();
    int downsampler_fir_size = static_cast<int>(m_audio_output.get_downsampler_fir_size());

    LOG(device_info.deviceName() << ", " << format.sampleRate() << ", " << buffer_size << " bytes");
    emit audio_output_activated(device_info, format, buffer_size, downsampler_fir_size);
}
