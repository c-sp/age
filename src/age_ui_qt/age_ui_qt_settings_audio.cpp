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

#include <algorithm>

#include <QChar> // QLatin1Char
#include <QGroupBox>
#include <QList>
#include <QSizePolicy>
#include <QVBoxLayout>

#include <age_audio.hpp>
#include <age_debug.hpp>

#include "age_ui_qt_settings.hpp"

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif





//---------------------------------------------------------
//
//   Object creation/destruction
//
//---------------------------------------------------------

age::qt_settings_audio::qt_settings_audio(std::shared_ptr<qt_user_value_store> user_value_store, QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_user_value_store(user_value_store)
{
    // audio device

    m_combo_devices = new QComboBox;
    m_combo_devices->setEditable(false);
    QLabel *current = new QLabel("current device:");
    m_label_device = new QLabel("n/a");
    m_label_format = new QLabel("n/a");
    m_label_buffer = new QLabel("n/a");
    m_label_fir_entries = new QLabel("n/a");

    // since audio device names may be quite long, we let the respective widgets expand or shrink
    // (and don't use audio device names as minimum size, like it was before)
    m_combo_devices->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::QSizePolicy::Maximum);
    m_label_device->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::QSizePolicy::Maximum);

    QVBoxLayout *audio_device_layout = new QVBoxLayout;
    audio_device_layout->addWidget(m_combo_devices);
    audio_device_layout->addSpacing(qt_settings_element_spacing);
    audio_device_layout->addWidget(current);
    audio_device_layout->addWidget(m_label_device);
    audio_device_layout->addWidget(m_label_format);
    audio_device_layout->addWidget(m_label_buffer);
    audio_device_layout->addWidget(m_label_fir_entries);

    QGroupBox *audio_device_group = new QGroupBox("audio device");
    audio_device_group->setLayout(audio_device_layout);

    // audio volume

    m_slider_volume = new QSlider(Qt::Horizontal);
    m_slider_volume->setRange(qt_audio_volume_percent_min, qt_audio_volume_percent_max);
    m_label_volume = new QLabel;
    m_check_mute = new QCheckBox("mute");

    QGridLayout *audio_volume_layout = new QGridLayout;
    audio_volume_layout->addWidget(m_label_volume, 0, 0);
    audio_volume_layout->addWidget(m_slider_volume, 0, 1);
    audio_volume_layout->addWidget(m_check_mute, 1, 1);

    QGroupBox *audio_volume_group = new QGroupBox("audio volume");
    audio_volume_group->setLayout(audio_volume_layout);

    // audio latency

    m_slider_latency = new QSlider(Qt::Horizontal);
    m_slider_latency->setRange(qt_audio_latency_milliseconds_min / qt_audio_latency_milliseconds_step,
                               qt_audio_latency_milliseconds_max / qt_audio_latency_milliseconds_step);
    m_label_latency = new QLabel;

    QHBoxLayout *audio_latency_layout = new QHBoxLayout;
    audio_latency_layout->addWidget(m_label_latency);
    audio_latency_layout->addWidget(m_slider_latency);

    QGroupBox *audio_latency_group = new QGroupBox("audio latency");
    audio_latency_group->setLayout(audio_latency_layout);

    // audio downsampler

    m_combo_downsampler = new QComboBox;
    m_combo_downsampler->setEditable(false);
    m_combo_downsampler->addItem(get_name_for_qt_downsampler_quality(qt_downsampler_quality::low));
    m_combo_downsampler->addItem(get_name_for_qt_downsampler_quality(qt_downsampler_quality::high));
    m_combo_downsampler->addItem(get_name_for_qt_downsampler_quality(qt_downsampler_quality::highest));
    m_combo_downsampler->setCurrentText(qt_downsampler_quality_high); // default value
    QLabel *quality = new QLabel("resampling quality:");

    QVBoxLayout *audio_downsampler_layout = new QVBoxLayout;
    audio_downsampler_layout->addWidget(quality);
    audio_downsampler_layout->addWidget(m_combo_downsampler);

    QGroupBox *audio_downsampler_group = new QGroupBox("audio quality");
    audio_downsampler_group->setLayout(audio_downsampler_layout);

    // create final layout

    QGridLayout *layout = new QGridLayout;
    layout->setContentsMargins(qt_settings_layout_margin, qt_settings_layout_margin, qt_settings_layout_margin, qt_settings_layout_margin);
    layout->setSpacing(qt_settings_layout_spacing);
    layout->addWidget(audio_device_group, 0, 0, 1, 2);
    layout->addWidget(audio_volume_group, 1, 0, 1, 2);
    layout->addWidget(audio_latency_group, 2, 0);
    layout->addWidget(audio_downsampler_group, 2, 1);
    setLayout(layout);

    // connect signals to slots

    connect(m_combo_devices, SIGNAL(currentIndexChanged(QString)), this, SLOT(devices_index_changed(QString)));
    connect(m_check_mute, SIGNAL(stateChanged(int)), this, SLOT(mute_state_changed(int)));
    connect(m_slider_volume, SIGNAL(valueChanged(int)), this, SLOT(volume_value_changed(int)));
    connect(m_slider_latency, SIGNAL(valueChanged(int)), this, SLOT(latency_value_changed(int)));
    connect(m_combo_downsampler, SIGNAL(currentIndexChanged(QString)), this, SLOT(downsampler_quality_index_changed(QString)));

    // set values from config

    QString device = m_user_value_store->get_value(qt_settings_audio_device, "default").toString();
    populate_devices_box(device);

    m_check_mute->setChecked(m_user_value_store->get_value(qt_settings_audio_mute, false).toBool());
    m_slider_volume->setValue(m_user_value_store->get_value(qt_settings_audio_volume, 75).toInt());
    m_slider_latency->setValue(m_user_value_store->get_value(qt_settings_audio_latency, 8).toInt());
    m_combo_downsampler->setCurrentText(m_user_value_store->get_value(qt_settings_audio_downsampler_quality, qt_downsampler_quality_high).toString());

    // update labels

    volume_value_changed(m_slider_volume->value());
    latency_value_changed(m_slider_latency->value());
}





//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

void age::qt_settings_audio::set_active_audio_output(const QAudioDeviceInfo &device, const QAudioFormat &format, int buffer_size, int downsampler_fir_size)
{
    QString device_name = device.deviceName();
    m_label_device->setText(device_name);
    m_label_device->setToolTip(device_name);

    m_label_format->setText(QString("stereo, 16 bit, ") + QString::number(format.sampleRate()) + " hz");

    int samples = buffer_size / sizeof(lpcm_stereo_sample);
    int millis = samples * 1000 / format.sampleRate();
    m_label_buffer->setText(QString::number(buffer_size) + " bytes buffer ("
                            + QString::number(samples) + " samples, "
                            + QString::number(millis) + " milliseconds)");

    m_label_fir_entries->setText(QString::number(downsampler_fir_size) + " FIR entries");
}



void age::qt_settings_audio::toggle_mute()
{
    m_check_mute->toggle();
}

void age::qt_settings_audio::increase_volume()
{
    int value = m_slider_volume->value();
    int delta = std::max(1, value / 20);
    m_slider_volume->setValue(value + delta);
}

void age::qt_settings_audio::decrease_volume()
{
    int value = m_slider_volume->value();
    int delta = std::max(1, value / 20);
    m_slider_volume->setValue(value - delta);
}



void age::qt_settings_audio::emit_settings_signals()
{
    emit output_changed(m_selected_device, m_selected_device_format);
    emit volume_changed(m_selected_volume);
    emit latency_changed(m_selected_latency);
    emit downsampler_quality_changed(m_selected_downsampler_quality);
}





//---------------------------------------------------------
//
//   private slots
//
//---------------------------------------------------------

void age::qt_settings_audio::devices_index_changed(const QString &device_name)
{
    // ignore this signal, if nothing is selected
    if (!device_name.isEmpty())
    {
        // check,if the audio device info is available
        QAudioDeviceInfo device_info = get_device_info(device_name);
        LOG("audio output device set to " << device_name << " (device is null: " << device_info.isNull() << ")");
        if (!device_info.isNull())
        {
            // store the selected device name before we change it
            m_user_value_store->set_value(qt_settings_audio_device, device_name);

            // find a suitable audio format
            LOG("looking for suitable audio format using device " << device_name);
            QAudioFormat format = find_suitable_format(device_info);
            LOG("format valid: " << format.isValid());

            // allocate and use the specified audio output, if the format is valid
            if (format.isValid())
            {
                m_selected_device = device_info;
                m_selected_device_format = format;

                LOG("emitting output_changed signal with " << device_info.deviceName());
                emit output_changed(m_selected_device, m_selected_device_format);
            }
        }
    }
}

void age::qt_settings_audio::mute_state_changed(int state)
{
    bool checked = is_checked(state);
    m_user_value_store->set_value(qt_settings_audio_mute, checked);

    update_volume(m_slider_volume->value());
}

void age::qt_settings_audio::volume_value_changed(int value)
{
    // we convert the number to a fixed-width string to prevent layout
    // changes caused by numbers of different lengths
    m_label_volume->setText(QString("volume:\n%1%").arg(value, 3, 10, QLatin1Char('0')));

    m_user_value_store->set_value(qt_settings_audio_volume, value);
    update_volume(value);
}

void age::qt_settings_audio::latency_value_changed(int value)
{
    m_user_value_store->set_value(qt_settings_audio_latency, value);

    m_selected_latency = value * qt_audio_latency_milliseconds_step;

    // we convert the number to a fixed-width string to prevent layout
    // changes caused by numbers of different lengths
    m_label_latency->setText(QString("latency:\n%1 ms").arg(m_selected_latency, 3, 10, QLatin1Char('0')));

    emit latency_changed(m_selected_latency);
}

void age::qt_settings_audio::downsampler_quality_index_changed(const QString &text)
{
    m_user_value_store->set_value(qt_settings_audio_downsampler_quality, text);

    m_selected_downsampler_quality = get_qt_downsampler_quality_for_name(text);
    emit downsampler_quality_changed(m_selected_downsampler_quality);
}





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

void age::qt_settings_audio::populate_devices_box(const QString &device_to_select)
{
    // clear combo box and set "default" item
    m_combo_devices->clear();
    m_combo_devices->addItem(qt_settings_default_audio_device_name);

    // add currently available audio output device names
    int current_device_index = 0; // index of the "default" item
    QList<QAudioDeviceInfo> output_devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    for (int i = 0; i < output_devices.size(); ++i)
    {
        QAudioDeviceInfo device_info = output_devices.at(i);
        LOG("available audio output device: " << device_info.deviceName());
        m_combo_devices->addItem(device_info.deviceName());

        if (device_to_select == device_info.deviceName())
        {
            LOG("found device_to_select at index " << i << ": " << device_to_select);
            current_device_index = i + 1;
        }
    }

    // select the requested audio output device, if available
    // (select "default" otherwise)
    LOG("setting device combo box currentIndex to " << current_device_index);
    m_combo_devices->setCurrentIndex(current_device_index);
}



void age::qt_settings_audio::update_volume(int volume)
{
    int selected_volume = (m_check_mute->checkState() == Qt::Checked) ? 0 : volume;

    if (selected_volume != m_selected_volume)
    {
        m_selected_volume = selected_volume;
        emit volume_changed(m_selected_volume);
    }
}



QAudioDeviceInfo age::qt_settings_audio::get_device_info(const QString &device_name) const
{
    QAudioDeviceInfo result;

    // get the default audio output device info
    if (device_name == qt_settings_default_audio_device_name)
    {
        result = QAudioDeviceInfo::defaultOutputDevice();
    }

    // get the specified audio output device info
    else
    {
        QList<QAudioDeviceInfo> output_devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
        for (int i = 0; i < output_devices.size(); ++i)
        {
            QAudioDeviceInfo device_info = output_devices.at(i);
            if (device_name == device_info.deviceName())
            {
                result = device_info;
                break;
            }
        }
    }

    return result;
}



QAudioFormat age::qt_settings_audio::find_suitable_format(const QAudioDeviceInfo &device_info)
{
    // create the audio format we want to use
    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setSampleType(QAudioFormat::SignedInt);
    //format.setByteOrder(); // initialized to system's byte order

    // find the best matching audio format available on the specified device
    QAudioFormat nearest_format = device_info.nearestFormat(format);

    // check the fixed parameters and invalidate the format, if we cannot use it
    if (nearest_format.sampleRate() < 11025)
    {
        nearest_format.setSampleRate(-1);
    }
    if (nearest_format.channelCount() != format.channelCount())
    {
        nearest_format.setChannelCount(-1);
    }
    if (nearest_format.sampleSize() != format.sampleSize())
    {
        nearest_format.setSampleSize(-1);
    }
    if (nearest_format.codec() != format.codec())
    {
        nearest_format.setCodec("");
    }
    if (nearest_format.sampleType() != format.sampleType())
    {
        nearest_format.setSampleType(QAudioFormat::Unknown);
    }

    // return the (potentially invalid) format
    return nearest_format;
}
