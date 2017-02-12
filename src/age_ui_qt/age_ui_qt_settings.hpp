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

#ifndef AGE_UI_QT_SETTINGS_HPP
#define AGE_UI_QT_SETTINGS_HPP

//!
//! \file
//!

#include "age_ui_qt_user_value_store.hpp"



namespace age
{





//---------------------------------------------------------
//
//   video settings widget
//
//---------------------------------------------------------

class qt_filter_widget : public QFrame
{
    Q_OBJECT
public:

    qt_filter_widget(uint widget_index, QWidget* parent = 0, Qt::WindowFlags flags = 0);

    void set_filter(qt_filter filter, GLint width, GLint height);

signals:

    void remove(uint index);
    void drag_start(uint drag_index);
    void drag_clear();
    void drag_update(uint insert_at_index);
    void drag_finish(bool keep);

protected:

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:

    void remove_clicked();

private:

    bool handle_drag_event(QDropEvent *event);

    const uint m_widget_index;
    QLabel *m_text = nullptr;
    QPushButton *m_remove = nullptr;
    bool m_has_content = false;
};



class qt_settings_video : public QWidget, non_copyable
{
    Q_OBJECT
public:

    qt_settings_video(std::shared_ptr<qt_user_value_store> user_value_store, GLint max_texture_size, QWidget* parent = 0, Qt::WindowFlags flags = 0);

    void set_simulator_screen_size(GLint width, GLint height);

    void toggle_filter_chain();
    void toggle_bilinear_filter();
    void cycle_frames_to_blend();

    void emit_settings_signals();

    static QString get_name_for_qt_filter(qt_filter filter);

signals:

    void use_bilinear_filter_changed(bool use);
    void frames_to_blend_changed(uint frames_to_blend);
    void filter_chain_changed(qt_filter_vector filter_chain);

private slots:

    void filter_chain_state_changed(int state);
    void bilinear_filter_state_changed(int state);
    void frames_to_blend_toggled(bool checked);
    void add_filter_clicked();

    void remove(uint index);
    void drag_start(uint drag_index);
    void drag_clear();
    void drag_update(uint insert_at_index);
    void drag_finish(bool keep);

private:

    static qt_filter get_qt_filter_for_name(const QString &filter);

    void update_filter_chain_controls(bool only_controls);
    void emit_filter_chain_changed();

    std::shared_ptr<qt_user_value_store> m_user_value_store;

    GLint m_simulator_screen_width = 0;
    GLint m_simulator_screen_height = 0;
    const GLint m_max_texture_size;

    QCheckBox *m_use_filter_chain = nullptr;
    QCheckBox *m_use_bilinear_filter = nullptr;
    std::array<QRadioButton*, qt_video_frame_history_size> m_blend_frames;
    std::array<qt_filter_widget*, 8> m_filter_widgets;

    int m_frames_to_blend = 0;
    qt_filter_vector m_filter_chain;

    qt_filter_vector m_dnd_original_filter_chain;
    uint m_dnd_index = 0;
};





//---------------------------------------------------------
//
//   audio settings widget
//
//---------------------------------------------------------

class qt_settings_audio : public QWidget, non_copyable
{
    Q_OBJECT
public:

    qt_settings_audio(std::shared_ptr<qt_user_value_store> user_value_store, QWidget* parent = 0, Qt::WindowFlags flags = 0);

    void set_active_audio_output(const QAudioDeviceInfo &device, const QAudioFormat &format, int buffer_size, int downsampler_fir_size);

    void toggle_mute();
    void increase_volume();
    void decrease_volume();

    void emit_settings_signals();

signals:

    void output_changed(QAudioDeviceInfo device, QAudioFormat format);
    void volume_changed(int volume_percent);
    void latency_changed(int latency_milliseconds);
    void downsampler_quality_changed(age::qt_downsampler_quality quality);

private slots:

    void devices_index_changed(const QString &text);
    void mute_state_changed(int state);
    void volume_value_changed(int value);
    void latency_value_changed(int value);
    void downsampler_quality_index_changed(const QString &text);

private:

    void populate_devices_box(const QString &device_to_select);
    void update_volume(int volume);

    QAudioDeviceInfo get_device_info(const QString &device_name) const;
    static QAudioFormat find_suitable_format(const QAudioDeviceInfo &device_info);

    std::shared_ptr<qt_user_value_store> m_user_value_store;

    QComboBox *m_combo_devices = nullptr;
    QLabel *m_label_device = nullptr;
    QLabel *m_label_format = nullptr;
    QLabel *m_label_buffer = nullptr;
    QLabel *m_label_fir_entries = nullptr;

    QComboBox *m_combo_downsampler = nullptr;

    QSlider *m_slider_volume = nullptr;
    QLabel *m_label_volume = nullptr;
    QCheckBox *m_check_mute = nullptr;

    QSlider *m_slider_latency = nullptr;
    QLabel *m_label_latency = nullptr;

    QAudioDeviceInfo m_selected_device;
    QAudioFormat m_selected_device_format;
    int m_selected_volume = 0;
    int m_selected_latency = 0;
    qt_downsampler_quality m_selected_downsampler_quality = qt_downsampler_quality::high; // default value
};





//---------------------------------------------------------
//
//   keys settings widget
//
//---------------------------------------------------------

class qt_settings_keys : public QWidget, non_copyable
{
    Q_OBJECT
public:

    qt_settings_keys(std::shared_ptr<qt_user_value_store> user_value_store, QWidget* parent = 0, Qt::WindowFlags flags = 0);

    qt_key_event get_event_for_key(Qt::Key key) const;

private slots:

    void button_clicked(bool checked);

private:

    QString get_event_string(qt_key_event event) const;
    QString get_key_string(Qt::Key key) const;

    void add_key_widgets(QGridLayout *layout, int row, qt_key_event event, Qt::Key default_key);
    void set_key_binding(qt_key_event event, Qt::Key key, bool update_settings);
    void save_key_binding(qt_key_event event);

    std::shared_ptr<qt_user_value_store> m_user_value_store;

    const QMap<qt_key_event, const char*> m_category_strings;
    const QMap<qt_key_event, const char*> m_event_strings;
    const QMap<qt_key_event, const char*> m_event_setting_strings;
    const QSet<Qt::Key> m_allowed_keys;

    QMap<QObject*, qt_key_event> m_event_for_object;
    QMap<qt_key_event, QPushButton*> m_button_for_event;

    QMap<Qt::Key, qt_key_event> m_event_for_key;
    QMap<qt_key_event, Qt::Key> m_key_for_event;
};



class qt_key_dialog : public QDialog
{
    Q_OBJECT
public:

    qt_key_dialog(qt_settings_keys* parent, const QString &event, const QString &key);

    Qt::Key get_key_pressed() const;

protected:

    void keyPressEvent(QKeyEvent *key_event) override;

private:

    qt_settings_keys *const m_parent;
    Qt::Key m_key_pressed = Qt::Key_unknown;
};





//---------------------------------------------------------
//
//   miscellaneous settings widget
//
//---------------------------------------------------------

class qt_settings_miscellaneous : public QWidget, non_copyable
{
    Q_OBJECT
public:

    qt_settings_miscellaneous(std::shared_ptr<qt_user_value_store> user_value_store, QWidget* parent = 0, Qt::WindowFlags flags = 0);

    bool show_menu_bar(bool fullscreen) const;
    bool show_status_bar(bool fullscreen) const;

    void toggle_pause_simulation();
    void toggle_synchronize_simulation();
    void set_pause_simulation(bool pause_simulation);
    void emit_settings_signals();

signals:

    void pause_simulation_changed(bool pause_simulation);
    void synchronize_simulation_changed(bool synchronize_simulation);

    void show_menu_bar_changed(bool show_menu_bar);
    void show_status_bar_changed(bool show_status_bar);
    void show_menu_bar_fullscreen_changed(bool show_menu_bar_fullscreen);
    void show_status_bar_fullscreen_changed(bool show_status_bar_fullscreen);

private slots:

    void pause_simulation_changed(int state);
    void synchronize_simulation_changed(int state);

    void show_menu_bar_changed(int state);
    void show_status_bar_changed(int state);
    void show_menu_bar_fullscreen_changed(int state);
    void show_status_bar_fullscreen_changed(int state);

private:

    std::shared_ptr<qt_user_value_store> m_user_value_store;

    QCheckBox *m_pause_simulation = nullptr;
    QCheckBox *m_synchronize_simulation = nullptr;

    QCheckBox *m_show_menu_bar = nullptr;
    QCheckBox *m_show_status_bar = nullptr;
    QCheckBox *m_show_menu_bar_fullscreen = nullptr;
    QCheckBox *m_show_status_bar_fullscreen = nullptr;
};





//---------------------------------------------------------
//
//   settings dialog
//
//---------------------------------------------------------

class qt_settings_dialog : public QDialog, non_copyable
{
    Q_OBJECT
public:

    qt_settings_dialog(std::shared_ptr<qt_user_value_store> user_value_store, GLint max_texture_size, QWidget *parent, Qt::WindowFlags flags = 0);

    qt_key_event get_event_for_key(Qt::Key key) const;
    QString get_open_file_dialog_directory() const;
    bool show_menu_bar(bool fullscreen) const;
    bool show_status_bar(bool fullscreen) const;

    void set_open_file_dialog_directory(const QString &directory);
    void set_simulator_screen_size(GLint width, GLint height);
    void set_pause_simulation(bool pause_simulation);

    bool trigger_settings_event(qt_key_event event);
    void emit_settings_signals();

signals:

    void video_use_bilinear_filter_changed(bool use);
    void video_frames_to_blend_changed(uint frames_to_blend);
    void video_filter_chain_changed(qt_filter_vector filter_chain);

    void audio_output_changed(QAudioDeviceInfo device, QAudioFormat format);
    void audio_volume_changed(int volume_percent);
    void audio_latency_changed(int latency_milliseconds);
    void audio_downsampler_quality_changed(age::qt_downsampler_quality quality);

    void misc_pause_simulation_changed(bool pause_simulation);
    void misc_synchronize_simulation_changed(bool synchronize_simulation);
    void misc_show_menu_bar_changed(bool show_menu_bar);
    void misc_show_status_bar_changed(bool show_status_bar);
    void misc_show_menu_bar_fullscreen_changed(bool show_menu_bar_fullscreen);
    void misc_show_status_bar_fullscreen_changed(bool show_status_bar_fullscreen);

public slots:

    void audio_output_activated(QAudioDeviceInfo device, QAudioFormat format, int buffer_size, int downsampler_fir_size);



protected:

    void keyPressEvent(QKeyEvent *key_event) override;
    void closeEvent(QCloseEvent *close_event) override;

private slots:

    void emit_video_use_bilinear_filter_changed(bool use);
    void emit_video_frames_to_blend_changed(uint frames_to_blend);
    void emit_video_filter_chain_changed(qt_filter_vector filter_chain);

    void emit_audio_output_changed(QAudioDeviceInfo device, QAudioFormat format);
    void emit_audio_volume_changed(int volume_percent);
    void emit_audio_latency_changed(int latency_milliseconds);
    void emit_audio_downsampler_quality_changed(age::qt_downsampler_quality quality);

    void emit_misc_pause_simulation_changed(bool pause_simulation);
    void emit_misc_synchronize_simulation_changed(bool synchronize_simulation);
    void emit_misc_show_menu_bar_changed(bool show_menu_bar);
    void emit_misc_show_status_bar_changed(bool show_status_bar);
    void emit_misc_show_menu_bar_fullscreen_changed(bool show_menu_bar_fullscreen);
    void emit_misc_show_status_bar_fullscreen_changed(bool show_status_bar_fullscreen);

private:

    qt_settings_video *m_settings_video = nullptr;
    qt_settings_audio *m_settings_audio = nullptr;
    qt_settings_keys *m_settings_keys = nullptr;
    qt_settings_miscellaneous *m_settings_miscellaneous = nullptr;

    std::shared_ptr<qt_user_value_store> m_user_value_store = nullptr;
};

} // namespace age



#endif // AGE_UI_QT_SETTINGS_HPP
