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

#ifndef AGE_UI_QT_MAIN_WINDOW_HPP
#define AGE_UI_QT_MAIN_WINDOW_HPP

//!
//! \file
//!

#include "age_ui_qt_settings.hpp"
#include "age_ui_qt_emulation_runner.hpp"



namespace age
{

class qt_main_window : public QMainWindow, non_copyable
{
    Q_OBJECT
public:

    qt_main_window(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~qt_main_window();

    void start();

signals:

    void emulator_loaded(std::shared_ptr<age::qt_emulator> new_emulator);
    void emulator_button_down(uint button);
    void emulator_button_up(uint button);

private:

    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *keyEvent) override;
    void keyReleaseEvent(QKeyEvent *keyEvent) override;

    uint get_button(qt_key_event key_event) const;
    bool is_fullscreen() const;
    void fill_menu(QMenu *menu);
    qt_key_event get_event_for_key(int key);
    void open_file(bool force_dmg);

    std::shared_ptr<qt_user_value_store> m_user_value_store = nullptr;

    QAction *m_action_open = nullptr;
    QAction *m_action_open_dmg = nullptr;
    QAction *m_action_settings = nullptr;
    QAction *m_action_fullscreen = nullptr;
    QAction *m_action_exit = nullptr;

    qt_settings_dialog *m_settings = nullptr;
    qt_gl_renderer *m_renderer = nullptr;

    QLabel *m_emulated_time_label = nullptr;
    QLabel *m_speed_label = nullptr;
    QLabel *m_fps_label = nullptr;

    qt_emulation_runner *m_emulation_runner = nullptr;
    QThread m_emulation_runner_thread;

private slots:

    void video_use_bilinear_filter_changed(bool use);
    void video_frames_to_blend_changed(uint frames_to_blend);
    void video_filter_chain_changed(qt_filter_vector filter_chain);

    void misc_show_menu_bar_changed(bool show_menu_bar);
    void misc_show_status_bar_changed(bool show_status_bar);
    void misc_show_menu_bar_fullscreen_changed(bool show_menu_bar_fullscreen);
    void misc_show_status_bar_fullscreen_changed(bool show_status_bar_fullscreen);

    void update_status();
    void menu_emulator_open();
    void menu_emulator_open_dmg();
    void menu_emulator_settings();
    void menu_emulator_fullscreen();
    void menu_emulator_exit();
};

} // namespace age



#endif // AGE_UI_QT_MAIN_WINDOW_HPP
