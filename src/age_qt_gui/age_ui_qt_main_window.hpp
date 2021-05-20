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

#ifndef AGE_UI_QT_MAIN_WINDOW_HPP
#define AGE_UI_QT_MAIN_WINDOW_HPP

//!
//! \file
//!

#include <QAction>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMouseEvent>
#include <QSharedPointer>
#include <QThread>
#include <QWidget>

#include <age_types.hpp>
#include <emulator/age_gb_types.hpp>

#include "age_ui_qt_emulator.hpp"
#include "age_ui_qt_settings.hpp"
#include "age_ui_qt_user_value_store.hpp"



namespace age
{

    class qt_main_window : public QMainWindow
    {
        Q_OBJECT
        AGE_DISABLE_COPY(qt_main_window);

    public:
        explicit qt_main_window(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
        ~qt_main_window() override;

    signals:

        void emulator_loaded(QSharedPointer<age::qt_emulator> new_emulator);
        void emulator_screen_resize(age::int16_t width, age::int16_t height);
        void emulator_button_down(int button);
        void emulator_button_up(int button);

    private:
        void contextMenuEvent(QContextMenuEvent* event) override;
        void mouseDoubleClickEvent(QMouseEvent* event) override;

        void keyPressEvent(QKeyEvent* keyEvent) override;
        void keyReleaseEvent(QKeyEvent* keyEvent) override;

        [[nodiscard]] int  get_button(qt_key_event key_event) const;
        [[nodiscard]] bool is_fullscreen() const;
        void               fill_menu(QMenu* menu);
        qt_key_event       get_event_for_key(int key);
        void               open_file(gb_hardware hardware = gb_hardware::auto_detect);

        QSharedPointer<qt_user_value_store> m_user_value_store = nullptr;

        QAction* m_action_open       = nullptr;
        QAction* m_action_open_dmg   = nullptr;
        QAction* m_action_open_cgb   = nullptr;
        QAction* m_action_settings   = nullptr;
        QAction* m_action_fullscreen = nullptr;
        QAction* m_action_exit       = nullptr;

        qt_settings_dialog* m_settings = nullptr;

        QLabel* m_emulated_time_label = nullptr;
        QLabel* m_speed_label         = nullptr;
        QLabel* m_fps_label           = nullptr;

        QThread m_emulation_runner_thread;

    private slots:

        void misc_show_menu_bar_changed(bool show_menu_bar);
        void misc_show_status_bar_changed(bool show_status_bar);
        void misc_show_menu_bar_fullscreen_changed(bool show_menu_bar_fullscreen);
        void misc_show_status_bar_fullscreen_changed(bool show_status_bar_fullscreen);

        void menu_emulator_open();
        void menu_emulator_open_dmg();
        void menu_emulator_open_cgb();
        void menu_emulator_settings();
        void menu_emulator_fullscreen();
        void menu_emulator_exit();

        void emulator_speed(int speed_percent);
        void emulator_milliseconds(qint64 emulated_milliseconds);
        void fps(int fps);
    };

} // namespace age



#endif // AGE_UI_QT_MAIN_WINDOW_HPP
