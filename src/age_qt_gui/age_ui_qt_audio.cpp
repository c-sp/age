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

#include "age_ui_qt_audio.hpp"

#include <cassert>

constexpr int sizeof_pcm_frame = sizeof(age::pcm_frame);



//---------------------------------------------------------
//
//   object creation/destruction
//
//---------------------------------------------------------

age::qt_audio_output::~qt_audio_output()
{
    if (m_sink != nullptr)
    {
        m_sink->stop();
    }
}



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

QAudioDevice age::qt_audio_output::get_device() const
{
    return m_device;
}

QAudioFormat age::qt_audio_output::get_format() const
{
    return (m_sink == nullptr) ? m_format : m_sink->format();
}

int age::qt_audio_output::get_buffer_size() const
{
    return (m_sink == nullptr) ? 0 : m_sink->bufferSize();
}

age::size_t age::qt_audio_output::get_downsampler_fir_size() const
{
    return m_downsampler_fir_size;
}



void age::qt_audio_output::set_volume(int volume_percent)
{
    m_volume = volume_percent / 100.F;

    if (m_downsampler != nullptr)
    {
        m_downsampler->set_volume(m_volume);
    }
}

void age::qt_audio_output::set_input_sampling_rate(int sampling_rate)
{
    assert(sampling_rate > 1);
    m_input_sampling_rate = sampling_rate;

    create_downsampler();
}

void age::qt_audio_output::set_downsampler_quality(qt_downsampler_quality quality)
{
    m_downsampler_quality = quality;

    create_downsampler();
}

void age::qt_audio_output::set_latency(int latency_milliseconds)
{
    latency_milliseconds   = qMax(qt_audio_latency_milliseconds_min, latency_milliseconds);
    m_latency_milliseconds = qMin(qt_audio_latency_milliseconds_max, latency_milliseconds);

    reset(); // during reset() the thread's event loop may be called
}

void age::qt_audio_output::set_device(QAudioDevice device, QAudioFormat format)
{
    assert(format.sampleRate() > 1);

    m_device = device;
    m_format = format;

    reset(); // during reset() the thread's event loop may be called
}



void age::qt_audio_output::buffer_samples(const pcm_vector& samples)
{
    if (m_sink != nullptr)
    {
        assert(m_downsampler != nullptr);

        m_downsampler->add_input_samples(samples);
        m_buffer.add_samples(m_downsampler->get_output_samples());
        m_downsampler->clear_output_samples();
    }
}

void age::qt_audio_output::buffer_silence()
{
    if (m_sink != nullptr)
    {
        int bytes_free = m_sink->bytesFree();

        if (bytes_free > 0)
        {
            int samples_free = bytes_free / sizeof_pcm_frame;

            if (m_buffer.get_buffered_samples() < samples_free)
            {
                int samples_to_add = samples_free - m_buffer.get_buffered_samples();
                m_buffer.add_samples(m_silence, samples_to_add);
            }
        }
    }
}

void age::qt_audio_output::stream_audio_data()
{
    if (m_sink != nullptr)
    {
        // wait for ring buffer to fill up on buffer underflow
        if (m_sink->bytesFree() == m_sink->bufferSize())
        {
            if (m_buffer.get_buffered_samples() == m_buffer.get_max_buffered_samples())
            {
                m_pause_streaming = false;
            }
            else if (!m_pause_streaming)
            {
                m_pause_streaming = true;
            }
        }

        // stream to audio output device
        // (call write_samples twice in case of a ring buffer wrap around)
        if (!m_pause_streaming)
        {
            int samples_written = write_samples();
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
    if (m_sink != nullptr)
    {
        m_sink->stop();

        m_downsampler = nullptr;
        m_sink        = nullptr;
        m_io_device   = nullptr;
    }

    // create a new audio output device
    if (!m_device.isNull() && m_format.isValid())
    {
        m_sink = QSharedPointer<QAudioSink>(new QAudioSink(m_device, m_format));

        // set the audio output buffer size
        int sample_rate = m_sink->format().sampleRate();

        int buffered_samples = m_latency_milliseconds * sample_rate / 1000;
        int buffered_bytes   = buffered_samples * sizeof_pcm_frame;

        m_sink->setBufferSize(buffered_bytes);

        // adjust the ring buffer
        if (buffered_samples != m_buffer.get_max_buffered_samples())
        {
            m_buffer = pcm_ring_buffer(buffered_samples);
        }
        m_buffer.set_to(pcm_frame());

        // create a new downsampler
        create_downsampler();

        // Start the new audio output device.
        m_io_device = m_sink->start();

        // create the silence buffer
        assert(buffered_samples >= 0);
        m_silence = pcm_vector(static_cast<unsigned>(buffered_samples), pcm_frame());
    }
}



void age::qt_audio_output::create_downsampler()
{
    if (m_sink != nullptr)
    {
        int output_sample_rate = m_sink->format().sampleRate();

        downsampler* d = nullptr;
        switch (m_downsampler_quality)
        {
            case qt_downsampler_quality::low:
                d                      = new downsampler_linear(m_input_sampling_rate, output_sample_rate);
                m_downsampler_fir_size = 0;
                break;

            case qt_downsampler_quality::high: {
                auto* ds               = new downsampler_kaiser_low_pass(m_input_sampling_rate, output_sample_rate, 0.1);
                m_downsampler_fir_size = ds->get_fir_size();
                d                      = ds;
                break;
            }

            case qt_downsampler_quality::highest: {
                auto* ds               = new downsampler_kaiser_low_pass(m_input_sampling_rate, output_sample_rate, 0.01);
                m_downsampler_fir_size = ds->get_fir_size();
                d                      = ds;
                break;
            }
        }

        m_downsampler = QSharedPointer<downsampler>(d);
        m_downsampler->set_volume(m_volume);
    }
}

int age::qt_audio_output::write_samples()
{
    assert(m_sink != nullptr);

    int samples_written = 0;

    // don't try to write samples, if no audio device has been opened
    // (may happen, if there is no audio device available)
    if (m_io_device != nullptr)
    {
        // check how many samples we can stream to the audio output device
        int bytes_free = m_sink->bytesFree();
        if (bytes_free > 0)
        {
            int samples_free = bytes_free / sizeof_pcm_frame;

            // check how many samples we have available for streaming
            int              samples_available = 0;
            const pcm_frame* buffer            = m_buffer.get_buffered_samples_ptr(samples_available);

            // write samples to audio output device
            int         samples_to_write = qMin(samples_available, samples_free);
            const char* char_buffer      = reinterpret_cast<const char*>(buffer);

            qint64 bytes_written = m_io_device->write(char_buffer, samples_to_write * sizeof_pcm_frame);

            // calculate the number of samples that were written
            qint64 tmp = bytes_written / sizeof_pcm_frame;
            assert((tmp >= 0) && (tmp <= samples_to_write));
            samples_written = static_cast<int>(tmp);

            m_buffer.discard_buffered_samples(samples_written);
        }
    }

    return samples_written;
}
