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

#ifndef AGE_UI_QT_SETTINGS_HPP
#define AGE_UI_QT_SETTINGS_HPP

//!
//! \file
//!

#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialog>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QFrame>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QList>
#include <QMap>
#include <QMouseEvent>
#include <QObject>
#include <QPushButton>
#include <QRadioButton>
#include <QSet>
#include <QSharedPointer>
#include <QSlider>
#include <QString>
#include <QWidget>
#include <QtGui/qopengl.h> // GLint

#include <age_types.hpp>

#include "age_ui_qt_user_value_store.hpp"



namespace age
{

    constexpr int qt_settings_layout_margin   = 30;
    constexpr int qt_settings_layout_spacing  = 30;
    constexpr int qt_settings_element_spacing = 15;

    //!
    //! The enum contains all key events known by the AGE Qt GUI.
    //! Examples of key events are setting adjustments (e.g. increasing the audio volume)
    //! or pressing a emulator specific button (e.g. pressing the Game Boy's "start" button).
    //!
    //! The value qt_key_event::none does not represent any actual key event.
    //! It is used for example when parsing a key event from a string fails because the
    //! string is not parsable.
    //!
    enum class qt_key_event
    {
        none,

        gb_up,
        gb_down,
        gb_left,
        gb_right,
        gb_a,
        gb_b,
        gb_start,
        gb_select,

        video_toggle_filter_chain,
        video_toggle_bilinear_filter,
        video_cycle_frames_to_blend,

        audio_toggle_mute,
        audio_increase_volume,
        audio_decrease_volume,

        misc_toggle_pause_emulator,
        misc_toggle_synchronize_emulator
    };





    //---------------------------------------------------------
    //
    //   video settings widget
    //
    //---------------------------------------------------------

    class qt_filter_widget : public QFrame
    {
        Q_OBJECT
    public:
        explicit qt_filter_widget(int widget_index, QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);

        void set_filter(qt_filter filter, GLint width, GLint height);

    signals:

        void remove(int index);
        void drag_start(int drag_index);
        void drag_clear();
        void drag_update(int insert_at_index);
        void drag_finish(bool keep);

    protected:
        void dragEnterEvent(QDragEnterEvent* event) override;
        void dragLeaveEvent(QDragLeaveEvent* event) override;
        void dropEvent(QDropEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;

    private slots:

        void remove_clicked();

    private:
        bool handle_drag_event(QDropEvent* event) const;

        const int    m_widget_index;
        QLabel*      m_text        = nullptr;
        QPushButton* m_remove      = nullptr;
        bool         m_has_content = false;
    };



    class qt_settings_video : public QWidget
    {
        Q_OBJECT
        AGE_DISABLE_COPY(qt_settings_video);

    public:
        explicit qt_settings_video(QSharedPointer<qt_user_value_store> user_value_store, QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);

        void set_emulator_screen_size(GLint width, GLint height);

        void toggle_filter_chain();
        void toggle_bilinear_filter();
        void cycle_frames_to_blend();

        void emit_settings_signals();

        static QString get_name_for_qt_filter(qt_filter filter);

    signals:

        void use_bilinear_filter_changed(bool use);
        void frames_to_blend_changed(int frames_to_blend);
        void filter_chain_changed(age::qt_filter_list filter_list);

    private slots:

        void filter_chain_state_changed(int state);
        void bilinear_filter_state_changed(int state);
        void frames_to_blend_toggled(bool checked);
        void add_filter_clicked();

        void remove(int index);
        void drag_start(int drag_index);
        void drag_clear();
        void drag_update(int insert_at_index);
        void drag_finish(bool keep);

    private:
        static qt_filter get_qt_filter_for_name(const QString& filter);

        void update_filter_chain_controls(bool only_controls);
        void emit_filter_chain_changed();

        QSharedPointer<qt_user_value_store> m_user_value_store;

        GLint m_emulator_screen_width  = 0;
        GLint m_emulator_screen_height = 0;

        QCheckBox*               m_use_filter_chain    = nullptr;
        QCheckBox*               m_use_bilinear_filter = nullptr;
        QList<QRadioButton*>     m_blend_frames;
        QList<qt_filter_widget*> m_filter_widgets;

        int            m_frames_to_blend = 0;
        qt_filter_list m_filter_list;

        qt_filter_list m_dnd_original_filter_chain;
        int            m_dnd_index = 0;
    };





    //---------------------------------------------------------
    //
    //   audio settings widget
    //
    //---------------------------------------------------------

    class qt_settings_audio : public QWidget
    {
        Q_OBJECT
        AGE_DISABLE_COPY(qt_settings_audio);

    public:
        explicit qt_settings_audio(QSharedPointer<qt_user_value_store> user_value_store, QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);

        void set_active_audio_output(const QAudioDeviceInfo& device, const QAudioFormat& format, int buffer_size, int downsampler_fir_size);

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

        void devices_index_changed(const QString& text);
        void mute_state_changed(int state);
        void volume_value_changed(int value);
        void latency_value_changed(int value);
        void downsampler_quality_index_changed(const QString& text);

    private:
        void populate_devices_box(const QString& device_to_select);
        void update_volume(int volume);

        [[nodiscard]] QAudioDeviceInfo get_device_info(const QString& device_name) const;
        static QAudioFormat            find_suitable_format(const QAudioDeviceInfo& device_info);

        QSharedPointer<qt_user_value_store> m_user_value_store;

        QComboBox* m_combo_devices     = nullptr;
        QLabel*    m_label_device      = nullptr;
        QLabel*    m_label_format      = nullptr;
        QLabel*    m_label_buffer      = nullptr;
        QLabel*    m_label_fir_entries = nullptr;

        QComboBox* m_combo_downsampler = nullptr;

        QSlider*   m_slider_volume = nullptr;
        QLabel*    m_label_volume  = nullptr;
        QCheckBox* m_check_mute    = nullptr;

        QSlider* m_slider_latency = nullptr;
        QLabel*  m_label_latency  = nullptr;

        QAudioDeviceInfo       m_selected_device;
        QAudioFormat           m_selected_device_format;
        int                    m_selected_volume              = 0;
        int                    m_selected_latency             = 0;
        qt_downsampler_quality m_selected_downsampler_quality = qt_downsampler_quality::high; // default value
    };





    //---------------------------------------------------------
    //
    //   keys settings widget
    //
    //---------------------------------------------------------

    class qt_settings_keys : public QWidget
    {
        Q_OBJECT
        AGE_DISABLE_COPY(qt_settings_keys);

    public:
        explicit qt_settings_keys(QSharedPointer<qt_user_value_store> user_value_store, QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);

        [[nodiscard]] qt_key_event get_event_for_key(Qt::Key key) const;

    private slots:

        void button_clicked(bool checked);

    private:
        [[nodiscard]] QString get_event_string(qt_key_event event) const;

        void add_key_widgets(QGridLayout* layout, int row, qt_key_event event, Qt::Key default_key);
        void set_key_binding(qt_key_event event, Qt::Key key, bool update_settings);
        void save_key_binding(qt_key_event event);

        QSharedPointer<qt_user_value_store> m_user_value_store;

        const QMap<qt_key_event, const char*> m_category_strings;
        const QMap<qt_key_event, const char*> m_event_strings;
        const QMap<qt_key_event, const char*> m_event_setting_strings;
        const QSet<Qt::Key>                   m_allowed_keys;

        QMap<QObject*, qt_key_event>     m_event_for_object;
        QMap<qt_key_event, QPushButton*> m_button_for_event;

        QMap<Qt::Key, qt_key_event> m_event_for_key;
        QMap<qt_key_event, Qt::Key> m_key_for_event;
    };



    class qt_key_dialog : public QDialog
    {
        Q_OBJECT
    public:
        qt_key_dialog(qt_settings_keys* parent, const QString& event, const QString& key);

        [[nodiscard]] Qt::Key get_key_pressed() const;

    protected:
        void keyPressEvent(QKeyEvent* key_event) override;

    private:
        Qt::Key m_key_pressed = Qt::Key_unknown;
    };





    //---------------------------------------------------------
    //
    //   miscellaneous settings widget
    //
    //---------------------------------------------------------

    class qt_settings_miscellaneous : public QWidget
    {
        Q_OBJECT
        AGE_DISABLE_COPY(qt_settings_miscellaneous);

    public:
        explicit qt_settings_miscellaneous(QSharedPointer<qt_user_value_store> user_value_store, QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);

        [[nodiscard]] bool show_menu_bar(bool fullscreen) const;
        [[nodiscard]] bool show_status_bar(bool fullscreen) const;

        void toggle_pause_emulator();
        void toggle_synchronize_emulator();
        void set_pause_emulator(bool pause_emulator);
        void emit_settings_signals();

    signals:

        void pause_emulator_changed(bool pause_emulator);
        void synchronize_emulator_changed(bool synchronize_emulator);

        void show_menu_bar_changed(bool show_menu_bar);
        void show_status_bar_changed(bool show_status_bar);
        void show_menu_bar_fullscreen_changed(bool show_menu_bar_fullscreen);
        void show_status_bar_fullscreen_changed(bool show_status_bar_fullscreen);

    private slots:

        void on_pause_emulator_change(int state);
        void on_synchronize_emulator_change(int state);

        void on_show_menu_bar_change(int state);
        void on_show_status_bar_change(int state);
        void on_show_menu_bar_fullscreen_change(int state);
        void on_show_status_bar_fullscreen_change(int state);

    private:
        QSharedPointer<qt_user_value_store> m_user_value_store;

        QCheckBox* m_pause_emulator       = nullptr;
        QCheckBox* m_synchronize_emulator = nullptr;

        QCheckBox* m_show_menu_bar              = nullptr;
        QCheckBox* m_show_status_bar            = nullptr;
        QCheckBox* m_show_menu_bar_fullscreen   = nullptr;
        QCheckBox* m_show_status_bar_fullscreen = nullptr;
    };





    //---------------------------------------------------------
    //
    //   settings dialog
    //
    //---------------------------------------------------------

    class qt_settings_dialog : public QDialog
    {
        Q_OBJECT
        AGE_DISABLE_COPY(qt_settings_dialog);

    public:
        qt_settings_dialog(QSharedPointer<qt_user_value_store> user_value_store, QWidget* parent, Qt::WindowFlags flags = nullptr);

        [[nodiscard]] qt_key_event get_event_for_key(Qt::Key key) const;
        [[nodiscard]] QString      get_open_file_dialog_directory() const;
        [[nodiscard]] bool         show_menu_bar(bool fullscreen) const;
        [[nodiscard]] bool         show_status_bar(bool fullscreen) const;

        void set_open_file_dialog_directory(const QString& directory);
        void set_pause_emulator(bool pause_emulator);

        bool trigger_settings_event(qt_key_event event);
        void emit_settings_signals();

    signals:

        void video_use_bilinear_filter_changed(bool use);
        void video_frames_to_blend_changed(int frames_to_blend);
        void video_post_processing_filter_changed(age::qt_filter_list filter_list);

        void audio_output_changed(QAudioDeviceInfo device, QAudioFormat format);
        void audio_volume_changed(int volume_percent);
        void audio_latency_changed(int latency_milliseconds);
        void audio_downsampler_quality_changed(age::qt_downsampler_quality quality);

        void misc_pause_emulator_changed(bool pause_emulator);
        void misc_synchronize_emulator_changed(bool synchronize_emulator);
        void misc_show_menu_bar_changed(bool show_menu_bar);
        void misc_show_status_bar_changed(bool show_status_bar);
        void misc_show_menu_bar_fullscreen_changed(bool show_menu_bar_fullscreen);
        void misc_show_status_bar_fullscreen_changed(bool show_status_bar_fullscreen);

    public slots:

        void audio_output_activated(QAudioDeviceInfo device, QAudioFormat format, int buffer_size, int downsampler_fir_size);
        void set_emulator_screen_size(age::int16_t width, age::int16_t height);



    protected:
        void keyPressEvent(QKeyEvent* key_event) override;
        void closeEvent(QCloseEvent* close_event) override;

    private slots:

        void emit_video_use_bilinear_filter_changed(bool use);
        void emit_video_frames_to_blend_changed(int frames_to_blend);
        void emit_video_post_processing_filter_changed(age::qt_filter_list filter_list);

        void emit_audio_output_changed(QAudioDeviceInfo device, QAudioFormat format);
        void emit_audio_volume_changed(int volume_percent);
        void emit_audio_latency_changed(int latency_milliseconds);
        void emit_audio_downsampler_quality_changed(age::qt_downsampler_quality quality);

        void emit_misc_pause_emulator_changed(bool pause_emulator);
        void emit_misc_synchronize_emulator_changed(bool synchronize_emulator);
        void emit_misc_show_menu_bar_changed(bool show_menu_bar);
        void emit_misc_show_status_bar_changed(bool show_status_bar);
        void emit_misc_show_menu_bar_fullscreen_changed(bool show_menu_bar_fullscreen);
        void emit_misc_show_status_bar_fullscreen_changed(bool show_status_bar_fullscreen);

    private:
        qt_settings_video*         m_settings_video         = nullptr;
        qt_settings_audio*         m_settings_audio         = nullptr;
        qt_settings_keys*          m_settings_keys          = nullptr;
        qt_settings_miscellaneous* m_settings_miscellaneous = nullptr;

        QSharedPointer<qt_user_value_store> m_user_value_store = nullptr;
    };

} // namespace age



#endif // AGE_UI_QT_SETTINGS_HPP
