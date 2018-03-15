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

#ifndef AGE_UI_QT_AUDIO_HPP
#define AGE_UI_QT_AUDIO_HPP

//!
//! \file
//!

#include <memory> // std::unique_ptr
#include <vector>

#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QIODevice>

#include <age_non_copyable.hpp>
#include <age_types.hpp>
#include <pcm/age_downsampler.hpp>
#include <pcm/age_pcm_ring_buffer.hpp>
#include <pcm/age_pcm_sample.hpp>

#include "age_ui_qt.hpp"



namespace age
{

//!
//! \brief The qt_audio_output class can be used for easily configurable audio streaming.
//!
//! This class is based on Qt's QAudioOutput.
//! It can be configured in several ways (for details see the respective methods):
//! - audio volume: set_volume()
//! - audio data sampling rate: set_input_sampling_rate().
//! - audio data resampling quality: set_resampling_quality()
//! - audio latency: set_latency()
//! - audio device: set_output()
//! - audio output format: set_output()
//!
//! Before any audio streaming is possible, set_output() must be called at least once.
//! The audio stream must be kept alive by regular stream_audio_data() calls.
//! The underlying QAudioOutput object is used in "push mode" (by not specifying any
//! QIODevice when calling QAudioOutput::start()).
//!
//! Hints regarding QAudioOutput:
//! - When trying "pull mode" (by using a custom QIODevice for buffering audio data),
//! there were frequent noticable audio lags caused by buffer underruns.
//! Apparently some of the timer events used for streaming audio data in "pull mode"
//! were massively delayed.
//! - QAudioOutpu::setVolume() did not work very well on Windows. Calling this method
//! sometimes did not work at all and at other times caused CPU load spikes and audio
//! lags. That's why we use our own volume control: downsampler::set_volume().
//!
class qt_audio_output : public non_copyable
{
public:

    //!
    //! If an audio device is still in use, close it.
    //!
    ~qt_audio_output();



    //!
    //! \brief Get information about the currently used audio output device.
    //! \return A QAudioDeviceInfo for the currently used audio output device.
    //!
    QAudioDeviceInfo get_device_info() const;

    //!
    //! \brief Get the audio format used by the current audio output device.
    //! \return A QAudioFormat for the currently used audio output format.
    //!
    QAudioFormat get_format() const;

    //!
    //! \brief Get the buffer size in bytes configured on the current audio output device.
    //! \return The current audio output device's buffer size.
    //!
    int get_buffer_size() const;

    //!
    //! \brief Get the last audio latency value set by set_latency().
    //! \return The last value set by set_latency().
    //!
    int get_latency_milliseconds() const;

    //!
    //! \brief Get the current downsampler's FIR size.
    //! \return The current downsampler's FIR size or zero, if the downsampler does not
    //! make use of any FIR.
    //!
    uint get_downsampler_fir_size() const;



    //!
    //! \brief Set the audio output volume as percent value.
    //!
    //! \param volume_percent The audio output volume as percent value.
    //!
    void set_volume(int volume_percent);

    //!
    //! \brief Set the sampling rate of audio data added with buffer_samples().
    //! \param sampling_rate The sampling rate of audio data added with buffer_samples().
    //! This value is expected to be greater than 1. The behaviour for a value lower
    //! than 2 is undefined.
    //!
    void set_input_sampling_rate(uint sampling_rate);

    //!
    //! \brief Set the quality of audio data resampling.
    //!
    //! Depending on the quality, resampling may be cheap or CPU intensive.
    //!
    //! \param quality The requested resampling quality.
    //!
    void set_downsampler_quality(qt_downsampler_quality quality);

    //!
    //! \brief Set the audio latency as millisecond value.
    //!
    //! The specified millisecond value will be clamped to the range
    //! [age::qt_audio_latency_milliseconds_min; age::qt_audio_latency_milliseconds_max].
    //! This defines the audio buffer size, i.e. the delay between adding new
    //! audio data to be played and the device actually playing that audio data.
    //!
    //! \param latency_milliseconds The audio latency.
    //!
    void set_latency(int latency_milliseconds);

    //!
    //! \brief Use the specified audio device and audio format for audio streaming.
    //!
    //! This method expects, that:
    //! - the specified format is supported by the specified device
    //! - the specified format's channel count is 2
    //! - the specified format's sample size is 16 bits
    //! - the specified format's codec is "audio/pcm"
    //! - the specified format's sample type is QAudioFormat::SignedInt
    //! - the specified format's sample rate is greater than 1
    //!
    //! The root cause of these expectations is that we are streaming audio data
    //! in form of {@link pcm_sample}s.
    //! The behaviour when specifying parameters that do not meet these expectations
    //! is undefined.
    //!
    //! Note that the specified format's sample rate can be varied since
    //! qt_audio_output is capable of resampling the audio data to be
    //! streamed.
    //!
    //! \param device_info A QAudioDeviceInfo for audio output device to use.
    //! \param format A QAudioFormat for the audio output format to use.
    //!
    void set_output(QAudioDeviceInfo device_info, QAudioFormat format);



    //!
    //! \brief Copy the audio data specified as {@link pcm_sample}s
    //! to the intermediate buffer.
    //!
    //! To stream audio data from the intermediate buffer to the audio output
    //! device, call stream_audio_data().
    //!
    //! \param samples The {@link pcm_sample}s to be buffered.
    //!
    void buffer_samples(const pcm_vector &samples);

    //!
    //! \brief Copy a specific amount of silent {@link pcm_sample}s
    //! to the intermediate buffer.
    //!
    //! The amount of silent samples copied is calculated automatically, so that
    //! afterwards the intermediate buffer contains enough samples to completely
    //! fill the audio output device's buffer.
    //!
    //! To stream audio data from the intermediate buffer to the audio output
    //! device, call stream_audio_data().
    //!
    void buffer_silence();

    //!
    //! \brief Stream buffered audio data to the audio device.
    //!
    //! This method must be called regularly to keep the audio stream running.
    //! If a buffer underrun occurs on the audio device's buffer (which may produce
    //! some audible crack noise), streaming will be paused until the intermediate
    //! buffer is filled completely.
    //! By enforcing the audio latency in that way the probability of multiple
    //! concurrent buffer underruns is reduced.
    //!
    //! To add new audio data to the intermediate buffer, call buffer_samples()
    //! or buffer_silence().
    //!
    void stream_audio_data();



private:

    void reset();
    void create_downsampler();
    uint write_samples();

    uint m_input_sampling_rate = 200000; // some (arbitrary) big value
    float m_volume = 1;
    int m_latency_milliseconds = qt_audio_latency_milliseconds_min;
    qt_downsampler_quality m_downsampler_quality = qt_downsampler_quality::low;
    QAudioDeviceInfo m_device_info;
    QAudioFormat m_format;

    std::unique_ptr<downsampler> m_downsampler;
    uint m_downsampler_fir_size = 0;
    pcm_ring_buffer m_buffer = {1};
    std::unique_ptr<QAudioOutput> m_output;
    QIODevice *m_device = nullptr;
    std::vector<pcm_sample> m_silence;
    bool m_pause_streaming = false;
};

} // namespace age



#endif // AGE_UI_QT_AUDIO_HPP
