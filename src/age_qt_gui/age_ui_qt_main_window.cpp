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

#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QByteArray>
#include <QFile>
#include <QFileDialog>
#include <QMenuBar>
#include <QObject>
#include <QStatusBar>
#include <QString>
#include <QStringList>

#include <age_debug.hpp>
#include <emulator/age_gb_emulator.hpp> // gb buttons

#include "age_ui_qt_emulation_runner.hpp"
#include "age_ui_qt_main_window.hpp"
#include "age_ui_qt_video.hpp"

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif





//---------------------------------------------------------
//
//   object creation & destruction
//
//---------------------------------------------------------

age::qt_main_window::qt_main_window(QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    setWindowTitle(project_name);
    resize(500, 350);

    m_user_value_store = QSharedPointer<qt_user_value_store>(new qt_user_value_store());

    auto* video_output = new qt_video_output(this);
    setCentralWidget(video_output);

    m_settings = new qt_settings_dialog(m_user_value_store, this, Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    m_action_open       = new QAction("open file", this);
    m_action_open_dmg   = new QAction("open file as GB", this);
    m_action_open_cgb   = new QAction("open file as CGB", this);
    m_action_settings   = new QAction("settings", this);
    m_action_fullscreen = new QAction("fullscreen", this);
    m_action_fullscreen->setCheckable(true);
    m_action_fullscreen->setChecked(false);
    m_action_exit = new QAction("exit", this);

    QMenu* menu = menuBar()->addMenu(project_name);
    fill_menu(menu);

    QStatusBar* status_bar = statusBar();
    m_emulated_time_label  = new QLabel;
    m_speed_label          = new QLabel;
    m_fps_label            = new QLabel;
    m_emulated_time_label->setIndent(5);
    m_speed_label->setIndent(5);
    m_fps_label->setIndent(5);
    status_bar->addPermanentWidget(m_emulated_time_label);
    status_bar->addPermanentWidget(m_speed_label);
    status_bar->addPermanentWidget(m_fps_label);
    emulator_speed(0);
    emulator_milliseconds(0);
    fps(0);

    auto* emulation_runner = new qt_emulation_runner();
    emulation_runner->moveToThread(&m_emulation_runner_thread);

    // connect emulation runner (& thread) signals

    connect(&m_emulation_runner_thread, &QThread::started, emulation_runner, &qt_emulation_runner::initialize);
    connect(&m_emulation_runner_thread, &QThread::finished, emulation_runner, &QObject::deleteLater);

    connect(emulation_runner, &qt_emulation_runner::audio_output_activated, m_settings, &qt_settings_dialog::audio_output_activated);
    connect(emulation_runner, &qt_emulation_runner::emulator_screen_updated, video_output, &qt_video_output::new_frame);
    connect(emulation_runner, &qt_emulation_runner::emulator_speed, this, &qt_main_window::emulator_speed);
    connect(emulation_runner, &qt_emulation_runner::emulator_milliseconds, this, &qt_main_window::emulator_milliseconds);

    // connect video output signals

    connect(video_output, &qt_video_output::fps, this, &qt_main_window::fps);

    // connect settings signals

    connect(m_settings, &qt_settings_dialog::video_use_bilinear_filter_changed, video_output, &qt_video_output::set_bilinear_filter);
    connect(m_settings, &qt_settings_dialog::video_frames_to_blend_changed, video_output, &qt_video_output::set_blend_frames);
    connect(m_settings, &qt_settings_dialog::video_post_processing_filter_changed, video_output, &qt_video_output::set_post_processing_filter);

    connect(m_settings, &qt_settings_dialog::audio_output_changed, emulation_runner, &qt_emulation_runner::set_audio_output);
    connect(m_settings, &qt_settings_dialog::audio_volume_changed, emulation_runner, &qt_emulation_runner::set_audio_volume);
    connect(m_settings, &qt_settings_dialog::audio_latency_changed, emulation_runner, &qt_emulation_runner::set_audio_latency);
    connect(m_settings, &qt_settings_dialog::audio_downsampler_quality_changed, emulation_runner, &qt_emulation_runner::set_audio_downsampler_quality);

    connect(m_settings, &qt_settings_dialog::misc_pause_emulator_changed, emulation_runner, &qt_emulation_runner::set_emulator_paused);
    connect(m_settings, &qt_settings_dialog::misc_synchronize_emulator_changed, emulation_runner, &qt_emulation_runner::set_emulator_synchronize);
    connect(m_settings, &qt_settings_dialog::misc_show_menu_bar_changed, this, &qt_main_window::misc_show_menu_bar_changed);
    connect(m_settings, &qt_settings_dialog::misc_show_status_bar_changed, this, &qt_main_window::misc_show_status_bar_changed);
    connect(m_settings, &qt_settings_dialog::misc_show_menu_bar_fullscreen_changed, this, &qt_main_window::misc_show_menu_bar_fullscreen_changed);
    connect(m_settings, &qt_settings_dialog::misc_show_status_bar_fullscreen_changed, this, &qt_main_window::misc_show_status_bar_fullscreen_changed);

    // connect action signals

    connect(m_action_open, &QAction::triggered, this, &qt_main_window::menu_emulator_open);
    connect(m_action_open_dmg, &QAction::triggered, this, &qt_main_window::menu_emulator_open_dmg);
    connect(m_action_open_cgb, &QAction::triggered, this, &qt_main_window::menu_emulator_open_cgb);
    connect(m_action_settings, &QAction::triggered, this, &qt_main_window::menu_emulator_settings);
    connect(m_action_fullscreen, &QAction::triggered, this, &qt_main_window::menu_emulator_fullscreen);
    connect(m_action_exit, &QAction::triggered, this, &qt_main_window::menu_emulator_exit);

    // connect main window signals

    connect(this, &qt_main_window::emulator_loaded, emulation_runner, &qt_emulation_runner::set_emulator);
    connect(this, &qt_main_window::emulator_screen_resize, video_output, &qt_video_output::set_emulator_screen_size);
    connect(this, &qt_main_window::emulator_screen_resize, m_settings, &qt_settings_dialog::set_emulator_screen_size);
    connect(this, &qt_main_window::emulator_button_down, emulation_runner, &qt_emulation_runner::set_emulator_buttons_down);
    connect(this, &qt_main_window::emulator_button_up, emulation_runner, &qt_emulation_runner::set_emulator_buttons_up);

    // trigger initial setting signals after slots have been connected & start the emulation runner thread

    m_settings->emit_settings_signals();
    m_emulation_runner_thread.start();
}

age::qt_main_window::~qt_main_window()
{
    LOG("terminating emulation runner");
    m_emulation_runner_thread.quit();
    m_emulation_runner_thread.wait();

    LOG("destructor done");
}





//---------------------------------------------------------
//
//   overriden widget methods
//
//---------------------------------------------------------

void age::qt_main_window::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    fill_menu(&menu);
    menu.exec(event->globalPos());
}

void age::qt_main_window::mouseDoubleClickEvent(QMouseEvent*)
{
    m_action_fullscreen->setChecked(!m_action_fullscreen->isChecked());
    menu_emulator_fullscreen();
}



void age::qt_main_window::keyPressEvent(QKeyEvent* keyEvent)
{
    // if there is no event bound to that key, call the parent handler
    qt_key_event event = get_event_for_key(keyEvent->key());
    if (event == qt_key_event::none)
    {
        QMainWindow::keyPressEvent(keyEvent);
    }

    // check if this is a settings event
    else
    {
        m_settings->trigger_settings_event(event);

        if (!keyEvent->isAutoRepeat())
        {
            int button = get_button(event);
            if (button != 0)
            {
                emit emulator_button_down(button);
            }
        }
    }
}

void age::qt_main_window::keyReleaseEvent(QKeyEvent* keyEvent)
{
    // if there is no event bound to that key, call the parent handler
    qt_key_event event = get_event_for_key(keyEvent->key());
    if (event == qt_key_event::none)
    {
        QMainWindow::keyReleaseEvent(keyEvent);
    }

    // notify the emulator
    else if (!keyEvent->isAutoRepeat())
    {
        int button = get_button(event);
        if (button != 0)
        {
            emit emulator_button_up(button);
        }
    }
}





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

int age::qt_main_window::get_button(qt_key_event key_event) const
{
    switch (key_event)
    {
        case qt_key_event::gb_up: return gb_up;
        case qt_key_event::gb_down: return gb_down;
        case qt_key_event::gb_left: return gb_left;
        case qt_key_event::gb_right: return gb_right;
        case qt_key_event::gb_a: return gb_a;
        case qt_key_event::gb_b: return gb_b;
        case qt_key_event::gb_start: return gb_start;
        case qt_key_event::gb_select: return gb_select;

        default: return 0;
    }
}



bool age::qt_main_window::is_fullscreen() const
{
    return m_action_fullscreen->isChecked();
}



void age::qt_main_window::fill_menu(QMenu* menu)
{
    menu->addAction(m_action_open);
    menu->addAction(m_action_open_dmg);
    menu->addAction(m_action_open_cgb);
    menu->addSeparator();
    menu->addAction(m_action_settings);
    menu->addAction(m_action_fullscreen);
    menu->addSeparator();
    menu->addAction(m_action_exit);
}



age::qt_key_event age::qt_main_window::get_event_for_key(int key)
{
    auto         qt_key = static_cast<Qt::Key>(key);
    qt_key_event event  = m_settings->get_event_for_key(qt_key);
    return event;
}



void age::qt_main_window::open_file(gb_hardware hardware)
{
    QString     file_name;
    QFileDialog dialog(this, "Open file", m_settings->get_open_file_dialog_directory(), "Gameboy files (*.gb *.gbc)");
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (dialog.exec())
    {
        m_settings->set_open_file_dialog_directory(dialog.directory().absolutePath());
        QStringList files = dialog.selectedFiles();
        if (!files.empty())
        {
            file_name = files.at(0);
        }
    }

    LOG("open-file-dialog selected " << AGE_LOG_QUOTED(file_name.toStdString()));
    if (file_name.length() > 0)
    {
        // load file
        QByteArray file_contents;
        {
            QFile file = {file_name};
            if (file.open(QIODevice::ReadOnly) && (file.size() <= max_file_bytes))
            {
                file_contents = file.readAll();
            }
        }

        // create emulator
        QSharedPointer<qt_emulator> new_emulator;

        if (file_contents.size() > 0)
        {
            new_emulator = QSharedPointer<qt_emulator>(new qt_emulator(file_contents, hardware, m_user_value_store));
        }

        // propagate new emulator
        if (new_emulator != nullptr)
        {
            int16_t screen_width  = new_emulator->get_emulator()->get_screen_width();
            int16_t screen_height = new_emulator->get_emulator()->get_screen_height();

            emit emulator_screen_resize(screen_width, screen_height);
            emit emulator_loaded(new_emulator);
        }
    }
}





//---------------------------------------------------------
//
//   private slots
//
//---------------------------------------------------------

void age::qt_main_window::misc_show_menu_bar_changed(bool show_menu_bar)
{
    LOG("show_menu_bar changed to " << show_menu_bar);
    if (!is_fullscreen())
    {
        menuBar()->setVisible(show_menu_bar);
    }
}

void age::qt_main_window::misc_show_status_bar_changed(bool show_status_bar)
{
    LOG("show_status_bar changed to " << show_status_bar);
    if (!is_fullscreen())
    {
        statusBar()->setVisible(show_status_bar);
    }
}

void age::qt_main_window::misc_show_menu_bar_fullscreen_changed(bool show_menu_bar_fullscreen)
{
    LOG("show_menu_bar_fullscreen changed to " << show_menu_bar_fullscreen);
    if (is_fullscreen())
    {
        menuBar()->setVisible(show_menu_bar_fullscreen);
    }
}

void age::qt_main_window::misc_show_status_bar_fullscreen_changed(bool show_status_bar_fullscreen)
{
    LOG("show_status_bar_fullscreen changed to " << show_status_bar_fullscreen);
    if (is_fullscreen())
    {
        statusBar()->setVisible(show_status_bar_fullscreen);
    }
}



void age::qt_main_window::menu_emulator_open()
{
    open_file();
    m_settings->set_pause_emulator(false);
}

void age::qt_main_window::menu_emulator_open_dmg()
{
    open_file(gb_hardware::dmg);
    m_settings->set_pause_emulator(false);
}

void age::qt_main_window::menu_emulator_open_cgb()
{
    open_file(gb_hardware::cgb);
    m_settings->set_pause_emulator(false);
}

void age::qt_main_window::menu_emulator_settings()
{
    m_settings->show();
}

void age::qt_main_window::menu_emulator_fullscreen()
{
    bool fullscreen = is_fullscreen();

    menuBar()->setVisible(m_settings->show_menu_bar(fullscreen));
    statusBar()->setVisible(m_settings->show_status_bar(fullscreen));

    if (fullscreen)
    {
        showFullScreen();
    }
    else
    {
        showNormal();
    }
}

void age::qt_main_window::menu_emulator_exit()
{
    close();
}



void age::qt_main_window::emulator_speed(int speed_percent)
{
    QString speed(QString::number(speed_percent).append(" %"));
    m_speed_label->setText(speed);
}

void age::qt_main_window::emulator_milliseconds(qint64 emulated_milliseconds)
{
    QString emulated_time = QString::number(emulated_milliseconds / 1000) + "." + QString::number((emulated_milliseconds % 1000) / 10) + " s";
    m_emulated_time_label->setText(emulated_time);
}

void age::qt_main_window::fps(int fps)
{
    QString speed(QString::number(fps).append(" fps"));
    m_fps_label->setText(speed);
}
