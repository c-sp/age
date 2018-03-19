//
// Copyright (c) 2010-2018 Christoph Sprenger
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

#include <algorithm>

#include <age_debug.hpp>

#include "age_ui_qt_audio.hpp"

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif

#if 0
#define LOG_STREAM(x) LOG(x)
#else
#define LOG_STREAM(x)
#endif



//---------------------------------------------------------
//
//   object destruction
//
//---------------------------------------------------------

age::qt_audio_output::~qt_audio_output()
{
    if (m_output != nullptr)
    {
        LOG("closing audio output");
        m_output->stop();
    }
}



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

QAudioDeviceInfo age::qt_audio_output::get_device_info() const
{
    return m_device_info;
}

QAudioFormat age::qt_audio_output::get_format() const
{
    return (m_output == nullptr) ? m_format : m_output->format();
}

int age::qt_audio_output::get_buffer_size() const
{
    return (m_output == nullptr) ? 0 : m_output->bufferSize();
}

int age::qt_audio_output::get_latency_milliseconds() const
{
    return m_latency_milliseconds;
}

age::uint age::qt_audio_output::get_downsampler_fir_size() const
{
    return m_downsampler_fir_size;
}



void age::qt_audio_output::set_volume(int volume_percent)
{
    m_volume = static_cast<float>(volume_percent / 100.0);
    LOG(volume_percent << " (" << m_volume << ")");

    if (m_downsampler != nullptr)
    {
        m_downsampler->set_volume(m_volume);
    }
}

void age::qt_audio_output::set_input_sampling_rate(uint sampling_rate)
{
    LOG(sampling_rate);
    AGE_ASSERT(sampling_rate > 1);
    m_input_sampling_rate = sampling_rate;

    create_downsampler();
}

void age::qt_audio_output::set_downsampler_quality(qt_downsampler_quality quality)
{
    LOG("");
    m_downsampler_quality = quality;

    create_downsampler();
}

void age::qt_audio_output::set_latency(int latency_milliseconds)
{
    LOG(latency_milliseconds);
    m_latency_milliseconds = std::min(qt_audio_latency_milliseconds_max, std::max(qt_audio_latency_milliseconds_min, latency_milliseconds));

    reset(); // during reset() the thread's event loop may be called
}

void age::qt_audio_output::set_output(QAudioDeviceInfo device_info, QAudioFormat format)
{
    LOG(device_info.deviceName() << " @ " << format.sampleRate() << "hz");
    AGE_ASSERT(format.sampleRate() > 1);

    m_device_info = device_info;
    m_format = format;

    reset(); // during reset() the thread's event loop may be called
}



void age::qt_audio_output::buffer_samples(const pcm_vector &samples)
{
    if (m_output != nullptr)
    {
        AGE_ASSERT(m_downsampler != nullptr);

        m_downsampler->add_input_samples(samples);
        m_buffer.add_samples(m_downsampler->get_output_samples());
        m_downsampler->clear_output_samples();
    }
}

void age::qt_audio_output::buffer_silence()
{
    if (m_output != nullptr)
    {
        int bytes_free = m_output->bytesFree();

        if (bytes_free > 0)
        {
            uint samples_free = static_cast<uint>(bytes_free / sizeof_pcm_sample);

            if (m_buffer.get_buffered_samples() < samples_free)
            {
                uint samples_to_add = samples_free - m_buffer.get_buffered_samples();
                m_buffer.add_samples(m_silence, samples_to_add);
            }
        }
    }
}

void age::qt_audio_output::stream_audio_data()
{
    if (m_output != nullptr)
    {
        LOG_STREAM("free bytes: " << m_output->bytesFree() << ", buffered bytes "
                   << m_buffer.get_buffered_samples() * sizeof_pcm_sample);

        // wait for ring buffer to fill up on buffer underrun
        if (m_output->bytesFree() == m_output->bufferSize())
        {
            if (m_buffer.get_buffered_samples() == m_buffer.get_max_buffered_samples())
            {
                m_pause_streaming = false;
                LOG("ring buffer is filled completely after buffer underrun, will resume streaming");
            }
            else if (!m_pause_streaming)
            {
                m_pause_streaming = true;
                LOG("buffer underun detected, streaming paused until ring buffer is filled completely");
            }
        }

        // stream to audio output device
        // (call write_samples twice in case of a ring buffer wrap around)
        if (!m_pause_streaming)
        {
            uint samples_written = write_samples();
            if (samples_written > 0)
            {
                write_samples();
            }
        }
    }
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

void age::qt_audio_output::reset()
{
    // stop the current audio output device and clean up
    if (m_output != nullptr)
    {
        LOG("cleaning up");
        m_output->stop();

        m_downsampler = nullptr;
        m_output = nullptr;
        m_device = nullptr;
    }

    // create a new audio output device
    LOG("device info null: " << m_device_info.isNull() << ", format valid: " << m_format.isValid());
    if (!m_device_info.isNull() && m_format.isValid())
    {
        m_output = std::unique_ptr<QAudioOutput>(new QAudioOutput(m_device_info, m_format));

        // set the audio output buffer size
        int sample_rate = m_output->format().sampleRate();
        uint output_sampling_rate = static_cast<uint>(sample_rate);

        LOG("current latency is " << m_latency_milliseconds);
        uint buffered_samples = m_latency_milliseconds * output_sampling_rate / 1000;
        uint buffered_bytes = buffered_samples * sizeof_pcm_sample;

        m_output->setBufferSize(buffered_bytes);

        // adjust the ring buffer
        if (buffered_samples != m_buffer.get_max_buffered_samples())
        {
            LOG("changing ring buffer size from " << m_buffer.get_max_buffered_samples() << " to " << buffered_samples);
            m_buffer = pcm_ring_buffer(buffered_samples);
        }
        m_buffer.set_to(pcm_sample());

        // create a new downsampler
        create_downsampler();

        // Start the new audio output device.
        m_device = m_output->start();
        LOG("started audio output, error code is " << m_output->error());
        LOG("output.bufferSize     " << m_output->bufferSize() << " (expected " << buffered_bytes << ")");
        LOG("output.category       " << m_output->category());
        LOG("output.notifyInterval " << m_output->notifyInterval());
        LOG("output.periodSize     " << m_output->periodSize());

        // create the silence buffer
        m_silence = std::vector<pcm_sample>(buffered_samples, pcm_sample());
    }
}



void age::qt_audio_output::create_downsampler()
{
    if (m_output != nullptr)
    {
        int sample_rate = m_output->format().sampleRate();
        uint output_sampling_rate = static_cast<uint>(sample_rate);

        downsampler *d = nullptr;
        switch (m_downsampler_quality)
        {
            case qt_downsampler_quality::low:
                LOG("creating downsampler_linear");
                d = new downsampler_linear(m_input_sampling_rate, output_sampling_rate);
                m_downsampler_fir_size = 0;
                break;

            case qt_downsampler_quality::high:
            {
                LOG("creating downsampler_kaiser with ripple 0.1");
                downsampler_kaiser_low_pass *ds = new downsampler_kaiser_low_pass(m_input_sampling_rate, output_sampling_rate, 0.1);
                m_downsampler_fir_size = ds->get_fir_size();
                d = ds;
                break;
            }

            case qt_downsampler_quality::highest:
            {
                LOG("creating downsampler_kaiser with ripple 0.01");
                downsampler_kaiser_low_pass *ds = new downsampler_kaiser_low_pass(m_input_sampling_rate, output_sampling_rate, 0.01);
                m_downsampler_fir_size = ds->get_fir_size();
                d = ds;
                break;
            }
        }

        m_downsampler = std::unique_ptr<downsampler>(d);
        m_downsampler->set_volume(m_volume);
    }
}

age::uint age::qt_audio_output::write_samples()
{
    AGE_ASSERT(m_output != nullptr);

    uint samples_written = 0;

    // don't try to write samples, if no audio device has been opened
    // (may happen, if there is no audio device available)
    if (m_device != nullptr)
    {
        // check how many samples we can stream to the audio output device
        int bytes_free_int = m_output->bytesFree();
        if (bytes_free_int > 0)
        {
            uint bytes_free = static_cast<uint>(bytes_free_int);
            uint samples_free = bytes_free / sizeof_pcm_sample;

            // check how many samples we have available for streaming
            uint samples_available;
            const pcm_sample* buffer = m_buffer.get_buffered_samples_ptr(samples_available);

            // write samples to audio output device
            uint samples_to_write = std::min(samples_available, samples_free);
            const char *char_buffer = reinterpret_cast<const char*>(buffer);

            qint64 bytes_written = m_device->write(char_buffer, samples_to_write * sizeof_pcm_sample);

            // calculate the number of samples that were written
            samples_written = bytes_written / sizeof_pcm_sample;
            m_buffer.discard_buffered_samples(samples_written);
        }
    }

    return samples_written;
}
