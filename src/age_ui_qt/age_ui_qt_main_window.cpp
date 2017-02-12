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

#include "age_ui_qt_main_window.hpp"

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

age::qt_main_window::qt_main_window(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    setWindowTitle((std::string(project_name) + " " + project_version).c_str());
    resize(500, 350);

    m_user_value_store = std::allocate_shared<qt_user_value_store>(std::allocator<qt_user_value_store>());

    m_renderer = new qt_gl_renderer(this);
    setCentralWidget(m_renderer);

    m_settings = new qt_settings_dialog(m_user_value_store, m_renderer->get_max_texture_size(), this, Qt::WindowTitleHint  | Qt::WindowCloseButtonHint);

    m_action_open = new QAction("open file", this);
    m_action_open_dmg = new QAction("open CGB file as GB", this);
    m_action_settings = new QAction("settings", this);
    m_action_fullscreen = new QAction("fullscreen", this);
    m_action_fullscreen->setCheckable(true);
    m_action_fullscreen->setChecked(false);
    m_action_exit = new QAction("exit", this);

    QMenu *menu = menuBar()->addMenu(project_name);
    fill_menu(menu);

    QStatusBar *status_bar = statusBar();
    m_simulated_time_label = new QLabel;
    m_speed_label = new QLabel;
    m_fps_label = new QLabel;
    m_simulated_time_label->setIndent(5);
    m_speed_label->setIndent(5);
    m_fps_label->setIndent(5);
    status_bar->addPermanentWidget(m_simulated_time_label);
    status_bar->addPermanentWidget(m_speed_label);
    status_bar->addPermanentWidget(m_fps_label);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update_status()));
    timer->start(250);

    m_simulation_runner = new qt_simulation_runner(*m_renderer, 8);
    m_simulation_runner->moveToThread(&m_simulation_runner_thread);

    // connect simulation runner (& thread) signals

    connect(&m_simulation_runner_thread, &QThread::started, m_simulation_runner, &qt_simulation_runner::initialize);
    connect(&m_simulation_runner_thread, &QThread::finished, m_simulation_runner, &QObject::deleteLater);

    connect(m_simulation_runner, SIGNAL(audio_output_activated(QAudioDeviceInfo,QAudioFormat,int,int)), m_settings, SLOT(audio_output_activated(QAudioDeviceInfo,QAudioFormat,int,int)));

    // connect settings signals

    connect(m_settings, SIGNAL(video_use_bilinear_filter_changed(bool)), this, SLOT(video_use_bilinear_filter_changed(bool)));
    connect(m_settings, SIGNAL(video_frames_to_blend_changed(uint)), this, SLOT(video_frames_to_blend_changed(uint)));
    connect(m_settings, SIGNAL(video_filter_chain_changed(qt_filter_vector)), this, SLOT(video_filter_chain_changed(qt_filter_vector)));

    connect(m_settings, SIGNAL(audio_output_changed(QAudioDeviceInfo,QAudioFormat)), m_simulation_runner, SLOT(set_audio_output(QAudioDeviceInfo,QAudioFormat)));
    connect(m_settings, SIGNAL(audio_volume_changed(int)), m_simulation_runner, SLOT(set_audio_volume(int)));
    connect(m_settings, SIGNAL(audio_latency_changed(int)), m_simulation_runner, SLOT(set_audio_latency(int)));
    connect(m_settings, SIGNAL(audio_downsampler_quality_changed(age::qt_downsampler_quality)), m_simulation_runner, SLOT(set_audio_downsampler_quality(age::qt_downsampler_quality)));

    connect(m_settings, SIGNAL(misc_pause_simulation_changed(bool)), m_simulation_runner, SLOT(set_simulation_paused(bool)));
    connect(m_settings, SIGNAL(misc_synchronize_simulation_changed(bool)), m_simulation_runner, SLOT(set_simulation_synchronize(bool)));
    connect(m_settings, SIGNAL(misc_show_menu_bar_changed(bool)), this, SLOT(misc_show_menu_bar_changed(bool)));
    connect(m_settings, SIGNAL(misc_show_status_bar_changed(bool)), this, SLOT(misc_show_status_bar_changed(bool)));
    connect(m_settings, SIGNAL(misc_show_menu_bar_fullscreen_changed(bool)), this, SLOT(misc_show_menu_bar_fullscreen_changed(bool)));
    connect(m_settings, SIGNAL(misc_show_status_bar_fullscreen_changed(bool)), this, SLOT(misc_show_status_bar_fullscreen_changed(bool)));

    // connect action signals

    connect(m_action_open, SIGNAL(triggered()), this, SLOT(menu_emulator_open()));
    connect(m_action_open_dmg, SIGNAL(triggered()), this, SLOT(menu_emulator_open_dmg()));
    connect(m_action_settings, SIGNAL(triggered()), this, SLOT(menu_emulator_settings()));
    connect(m_action_fullscreen, SIGNAL(triggered()), this, SLOT(menu_emulator_fullscreen()));
    connect(m_action_exit, SIGNAL(triggered()), this, SLOT(menu_emulator_exit()));

    // connect own signals

    connect(this, SIGNAL(simulator_loaded(std::shared_ptr<age::qt_simulator>)), m_simulation_runner, SLOT(set_simulator(std::shared_ptr<age::qt_simulator>)));
    connect(this, SIGNAL(simulator_button_down(uint)), m_simulation_runner, SLOT(set_simulator_buttons_down(uint)));
    connect(this, SIGNAL(simulator_button_up(uint)), m_simulation_runner, SLOT(set_simulator_buttons_up(uint)));

    // trigger initial setting signals after slots have been connected & start the simulation runner thread

    m_settings->emit_settings_signals();
    m_simulation_runner_thread.start();
}

age::qt_main_window::~qt_main_window()
{
    LOG("terminating simulation runner");
    m_simulation_runner_thread.quit();
    m_simulation_runner_thread.wait();

    LOG("destructor done");
}



void age::qt_main_window::start()
{
    m_renderer->start();
}





//---------------------------------------------------------
//
//   overriden widget methods
//
//---------------------------------------------------------

void age::qt_main_window::contextMenuEvent(QContextMenuEvent *event)
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



void age::qt_main_window::keyPressEvent(QKeyEvent *keyEvent)
{
    // if there is no event bound to that key, call the parent handler
    qt_key_event event = get_event_for_key(keyEvent->key());
    if (event == qt_key_event::none)
    {
        QMainWindow::keyReleaseEvent(keyEvent);
    }

    // check if this is a settings event
    else
    {
        m_settings->trigger_settings_event(event);

        if (!keyEvent->isAutoRepeat())
        {
            uint button = get_button(event);
            if (button != 0)
            {
                emit simulator_button_down(button);
            }
        }
    }
}

void age::qt_main_window::keyReleaseEvent(QKeyEvent *keyEvent)
{
    // if there is no event bound to that key, call the parent handler
    qt_key_event event = get_event_for_key(keyEvent->key());
    if (event == qt_key_event::none)
    {
        QMainWindow::keyReleaseEvent(keyEvent);
    }

    // notify the simulator
    else if (!keyEvent->isAutoRepeat())
    {
        uint button = get_button(event);
        if (button != 0)
        {
            emit simulator_button_up(button);
        }
    }
}





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

age::uint age::qt_main_window::get_button(qt_key_event key_event) const
{
    uint result;

    switch(key_event)
    {
    case qt_key_event::gb_up:     result = gb_up; break;
    case qt_key_event::gb_down:   result = gb_down; break;
    case qt_key_event::gb_left:   result = gb_left; break;
    case qt_key_event::gb_right:  result = gb_right; break;
    case qt_key_event::gb_a:      result = gb_a; break;
    case qt_key_event::gb_b:      result = gb_b; break;
    case qt_key_event::gb_start:  result = gb_start; break;
    case qt_key_event::gb_select: result = gb_select; break;

    default: result = 0; break;
    }

    return result;
}



bool age::qt_main_window::is_fullscreen() const
{
    return m_action_fullscreen->isChecked();
}



void age::qt_main_window::fill_menu(QMenu *menu)
{
    menu->addAction(m_action_open);
    menu->addAction(m_action_open_dmg);
    menu->addSeparator();
    menu->addAction(m_action_settings);
    menu->addAction(m_action_fullscreen);
    menu->addSeparator();
    menu->addAction(m_action_exit);
}



age::qt_key_event age::qt_main_window::get_event_for_key(int key)
{
    Qt::Key qt_key = static_cast<Qt::Key>(key);
    qt_key_event event = m_settings->get_event_for_key(qt_key);
    return event;
}



void age::qt_main_window::open_file(bool force_dmg)
{

    QString file_name;
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

    LOG("open-file-dialog selected " << std::quoted(file_name.toStdString()));
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

        // create simulator
        std::shared_ptr<qt_simulator> new_simulator;

        if (file_contents.size() > 0)
        {
            new_simulator = std::allocate_shared<qt_simulator>(std::allocator<qt_simulator>(), file_contents, force_dmg, m_user_value_store);
        }

        // propagate new simulator
        if (new_simulator != nullptr)
        {
            uint screen_width = new_simulator->get_simulator()->get_screen_width();
            uint screen_height = new_simulator->get_simulator()->get_screen_height();

            m_settings->set_simulator_screen_size(screen_width, screen_height);
            m_renderer->setMinimumWidth(screen_width);
            m_renderer->setMinimumHeight(screen_height);

            emit simulator_loaded(new_simulator);
        }
    }
}





//---------------------------------------------------------
//
//   private slots
//
//---------------------------------------------------------

void age::qt_main_window::video_use_bilinear_filter_changed(bool use)
{
    LOG("use bilinear filter " << use);
    m_renderer->set_bilinear_filter(use);
}

void age::qt_main_window::video_frames_to_blend_changed(uint frames_to_blend)
{
    LOG("blend frames " << frames_to_blend);
    m_renderer->set_blend_video_frames(frames_to_blend);
}

void age::qt_main_window::video_filter_chain_changed(qt_filter_vector filter_chain)
{
    LOG("filter chain changed, chain size " << filter_chain.size());
    m_renderer->set_filter_chain(filter_chain);
}



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



void age::qt_main_window::update_status()
{
    uint64 millis = m_simulation_runner->get_simulated_milliseconds();

    QString simulated_time = QString::number(millis / 1000) + "." + QString::number((millis % 1000) / 10) + " s";
    QString speed(QString::number(m_simulation_runner->get_speed_percent()).append(" %"));
    QString fps(QString::number(m_renderer->get_fps()).append(" fps"));

    m_simulated_time_label->setText(simulated_time);
    m_speed_label->setText(speed);
    m_fps_label->setText(fps);
}

void age::qt_main_window::menu_emulator_open()
{
    open_file(false);
    m_settings->set_pause_simulation(false);
}

void age::qt_main_window::menu_emulator_open_dmg()
{
    open_file(true);
    m_settings->set_pause_simulation(false);
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
