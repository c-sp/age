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

#include <QAudioDevice>
#include <QChar> // QLatin1Char
#include <QGroupBox>
#include <QList>
#include <QMediaDevices>
#include <QSizePolicy>
#include <QtDebug>
#include <QVBoxLayout>

#include <pcm/age_pcm_frame.hpp>

#include "age_ui_qt_settings.hpp"

#include <cassert>
#include <utility> // std::move

constexpr const char* qt_settings_audio_device              = "audio/device";
constexpr const char* qt_settings_audio_volume              = "audio/volume";
constexpr const char* qt_settings_audio_mute                = "audio/mute";
constexpr const char* qt_settings_audio_latency             = "audio/latency";
constexpr const char* qt_settings_audio_downsampler_quality = "audio/downsampler_quality";

constexpr int qt_audio_volume_percent_min = 0;
constexpr int qt_audio_volume_percent_max = 100;





//---------------------------------------------------------
//
//   Object creation/destruction
//
//---------------------------------------------------------

age::qt_settings_audio::qt_settings_audio(QSharedPointer<qt_user_value_store> user_value_store, QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_user_value_store(std::move(user_value_store))
{
    // audio device

    m_combo_devices = new QComboBox;
    m_combo_devices->setEditable(false);
    auto* current       = new QLabel("current device:");
    m_label_device      = new QLabel("n/a");
    m_label_format      = new QLabel("n/a");
    m_label_buffer      = new QLabel("n/a");
    m_label_fir_entries = new QLabel("n/a");

    // since audio device names may be quite long, we let the respective widgets expand or shrink
    // (and don't use audio device names as minimum size, like it was before)
    m_combo_devices->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::QSizePolicy::Maximum);
    m_label_device->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::QSizePolicy::Maximum);

    auto* audio_device_layout = new QVBoxLayout;
    audio_device_layout->addWidget(m_combo_devices);
    audio_device_layout->addSpacing(qt_settings_element_spacing);
    audio_device_layout->addWidget(current);
    audio_device_layout->addWidget(m_label_device);
    audio_device_layout->addWidget(m_label_format);
    audio_device_layout->addWidget(m_label_buffer);
    audio_device_layout->addWidget(m_label_fir_entries);

    auto* audio_device_group = new QGroupBox("audio device");
    audio_device_group->setLayout(audio_device_layout);

    // audio volume

    m_slider_volume = new QSlider(Qt::Horizontal);
    m_slider_volume->setRange(qt_audio_volume_percent_min, qt_audio_volume_percent_max);
    m_label_volume = new QLabel();
    m_check_mute   = new QCheckBox("mute");

    auto* audio_volume_layout = new QGridLayout;
    audio_volume_layout->addWidget(m_label_volume, 0, 0);
    audio_volume_layout->addWidget(m_slider_volume, 0, 1);
    audio_volume_layout->addWidget(m_check_mute, 1, 1);

    auto* audio_volume_group = new QGroupBox("audio volume");
    audio_volume_group->setLayout(audio_volume_layout);

    // audio latency

    m_slider_latency = new QSlider(Qt::Horizontal);
    m_slider_latency->setRange(qt_audio_latency_milliseconds_min / qt_audio_latency_milliseconds_step,
                               qt_audio_latency_milliseconds_max / qt_audio_latency_milliseconds_step);
    m_label_latency = new QLabel();

    auto* audio_latency_layout = new QHBoxLayout;
    audio_latency_layout->addWidget(m_label_latency);
    audio_latency_layout->addWidget(m_slider_latency);

    auto* audio_latency_group = new QGroupBox("audio latency");
    audio_latency_group->setLayout(audio_latency_layout);

    // audio downsampler

    m_combo_downsampler = new QComboBox();
    m_combo_downsampler->setEditable(false);
    m_combo_downsampler->addItem(get_name_for_qt_downsampler_quality(qt_downsampler_quality::low));
    m_combo_downsampler->addItem(get_name_for_qt_downsampler_quality(qt_downsampler_quality::high));
    m_combo_downsampler->addItem(get_name_for_qt_downsampler_quality(qt_downsampler_quality::highest));
    m_combo_downsampler->setCurrentText(qt_downsampler_quality_high); // default value
    auto* quality = new QLabel("resampling quality:");

    auto* audio_downsampler_layout = new QVBoxLayout;
    audio_downsampler_layout->addWidget(quality);
    audio_downsampler_layout->addWidget(m_combo_downsampler);

    auto* audio_downsampler_group = new QGroupBox("audio quality");
    audio_downsampler_group->setLayout(audio_downsampler_layout);

    // create final layout

    auto* layout = new QGridLayout;
    layout->setContentsMargins(qt_settings_layout_margin, qt_settings_layout_margin, qt_settings_layout_margin, qt_settings_layout_margin);
    layout->setSpacing(qt_settings_layout_spacing);
    layout->addWidget(audio_device_group, 0, 0, 1, 2);
    layout->addWidget(audio_volume_group, 1, 0, 1, 2);
    layout->addWidget(audio_latency_group, 2, 0);
    layout->addWidget(audio_downsampler_group, 2, 1);
    setLayout(layout);

    // connect signals to slots

    connect(m_combo_devices, &QComboBox::currentIndexChanged, this, &qt_settings_audio::devices_index_changed);
    connect(m_check_mute, &QCheckBox::stateChanged, this, &qt_settings_audio::mute_state_changed);
    connect(m_slider_volume, &QSlider::valueChanged, this, &qt_settings_audio::volume_value_changed);
    connect(m_slider_latency, &QSlider::valueChanged, this, &qt_settings_audio::latency_value_changed);
    connect(m_combo_downsampler, &QComboBox::currentTextChanged, this, &qt_settings_audio::downsampler_quality_index_changed);

    // set values from config

    QString device_id = m_user_value_store->get_value(qt_settings_audio_device, "no_device_selected").toString();
    populate_devices_box(device_id);

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

//void age::qt_settings_audio::set_active_audio_output(const QAudioDeviceInfo& device, const QAudioFormat& format, int buffer_size, int downsampler_fir_size)
//{
//    QString device_name = device.deviceName();
//    m_label_device->setText(device_name);
//    m_label_device->setToolTip(device_name);

//    m_label_format->setText(QString("stereo, 16 bit, ") + QString::number(format.sampleRate()) + " hz");

//    assert(sizeof(pcm_frame) <= int_max);
//    int samples = buffer_size / static_cast<int>(sizeof(pcm_frame));
//    int millis  = samples * 1000 / format.sampleRate();
//    m_label_buffer->setText(QString::number(buffer_size) + " bytes buffer ("
//                            + QString::number(samples) + " samples, "
//                            + QString::number(millis) + " milliseconds)");

//    m_label_fir_entries->setText(QString::number(downsampler_fir_size) + " FIR entries");
//}



void age::qt_settings_audio::toggle_mute()
{
    m_check_mute->toggle();
}

void age::qt_settings_audio::increase_volume()
{
    int value = m_slider_volume->value();
    int delta = qMax(1, value / 20);
    m_slider_volume->setValue(value + delta);
}

void age::qt_settings_audio::decrease_volume()
{
    int value = m_slider_volume->value();
    int delta = qMax(1, value / 20);
    m_slider_volume->setValue(value - delta);
}



void age::qt_settings_audio::emit_settings_signals()
{
    devices_index_changed(m_combo_devices->currentIndex());
    emit volume_changed(m_selected_volume);
    emit latency_changed(m_selected_latency);
    emit downsampler_quality_changed(m_selected_downsampler_quality);
}





//---------------------------------------------------------
//
//   private slots
//
//---------------------------------------------------------

void age::qt_settings_audio::devices_index_changed(int device_index)
{
    auto device_id = m_combo_devices->itemData(device_index);
    qDebug() << "select audio device" << device_index << ", id" << device_id;

    // ignore this signal, if the selection is not valid
    if (!device_id.isValid())
    {
        return;
    }

    // find the device
    const QAudioDevice* device  = nullptr;
    auto                devices = QMediaDevices::audioOutputs();

    for (auto d = devices.begin(); d != devices.end(); ++d)
    {
        if (d->id() == device_id)
        {
            device = &*d;
            break;
        }
    }
    if (device == nullptr)
    {
        return;
    }

    // store the selected device id
    m_user_value_store->set_value(qt_settings_audio_device, device_id.toString());

    // find a suitable audio format
    auto format = find_suitable_format(*device);

    qDebug() << "emit device_changed" << device->id() << "," << format;
    emit device_changed(*device, format);
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

void age::qt_settings_audio::downsampler_quality_index_changed(const QString& text)
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

void age::qt_settings_audio::populate_devices_box(const QString& device_id_to_select)
{
    // clear combo box and set "default" item
    m_combo_devices->clear();

    // add currently available audio output device names
    int  current_device_index = -1;
    int  default_device_index = -1;
    auto devices              = QMediaDevices::audioOutputs();
    for (int i = 0; i < devices.size(); ++i)
    {
        const auto& device = devices.at(i);

        m_combo_devices->addItem(
            device.description() + (device.isDefault() ? " (default)" : ""),
            device.id());

        if (device.isDefault())
        {
            default_device_index = i;
        }
        if (device_id_to_select == device.id())
        {
            current_device_index = i;
        }
    }

    // select the requested audio output device, if available
    // (otherwise select the default device)
    m_combo_devices->setCurrentIndex(current_device_index >= 0 ? current_device_index : default_device_index);
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



QAudioFormat age::qt_settings_audio::find_suitable_format(const QAudioDevice& device)
{
    QAudioFormat format;
    format.setChannelConfig(QAudioFormat::ChannelConfigStereo);
    format.setSampleFormat(QAudioFormat::Int16);

    QList<int> sample_rates({48000, 44100, 22050, 11025});
    for (auto sample_rate : sample_rates)
    {
        format.setSampleRate(sample_rate);
        if (device.isFormatSupported(format))
        {
            return format;
        }
    }

    // no suitable QAudioFormat found, return invalid QAudioFormat
    QAudioFormat invalid;
    invalid.setSampleRate(-1);
    return invalid;
}
