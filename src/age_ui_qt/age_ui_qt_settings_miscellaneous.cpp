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

age::qt_settings_miscellaneous::qt_settings_miscellaneous(std::shared_ptr<qt_user_value_store> user_value_store, QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_user_value_store(user_value_store)
{
    // simulation

    m_pause_simulation = new QCheckBox("pause simulation");
    m_synchronize_simulation = new QCheckBox("synchronize simulation to clock");

    QVBoxLayout *simulation_layout = new QVBoxLayout;
    simulation_layout->setAlignment(Qt::AlignCenter);
    simulation_layout->addWidget(m_pause_simulation);
    simulation_layout->addWidget(m_synchronize_simulation);

    QGroupBox *simulation_group = new QGroupBox("simulation");
    simulation_group->setLayout(simulation_layout);

    // window elements

    m_show_menu_bar = new QCheckBox("show menu bar");
    m_show_status_bar = new QCheckBox("show status bar");
    m_show_menu_bar_fullscreen = new QCheckBox("show menu bar in fullscreen mode");
    m_show_status_bar_fullscreen = new QCheckBox("show status bar in fullscreen mode");

    QVBoxLayout *window_layout = new QVBoxLayout;
    window_layout->setAlignment(Qt::AlignCenter);
    window_layout->addWidget(m_show_menu_bar);
    window_layout->addWidget(m_show_status_bar);
    window_layout->addSpacing(qt_settings_element_spacing);
    window_layout->addWidget(m_show_menu_bar_fullscreen);
    window_layout->addWidget(m_show_status_bar_fullscreen);

    QGroupBox *window_group = new QGroupBox("window elements");
    window_group->setLayout(window_layout);

    // create final layout

    QGridLayout *layout = new QGridLayout;
    layout->setContentsMargins(qt_settings_layout_margin, qt_settings_layout_margin, qt_settings_layout_margin, qt_settings_layout_margin);
    layout->setSpacing(qt_settings_layout_spacing);
    layout->addWidget(simulation_group, 0, 0);
    layout->addWidget(window_group, 1, 0);
    setLayout(layout);

    // connect signals to slots

    connect(m_pause_simulation, SIGNAL(stateChanged(int)), this, SLOT(pause_simulation_changed(int)));
    connect(m_synchronize_simulation, SIGNAL(stateChanged(int)), this, SLOT(synchronize_simulation_changed(int)));
    connect(m_show_menu_bar, SIGNAL(stateChanged(int)), this, SLOT(show_menu_bar_changed(int)));
    connect(m_show_status_bar, SIGNAL(stateChanged(int)), this, SLOT(show_status_bar_changed(int)));
    connect(m_show_menu_bar_fullscreen, SIGNAL(stateChanged(int)), this, SLOT(show_menu_bar_fullscreen_changed(int)));
    connect(m_show_status_bar_fullscreen, SIGNAL(stateChanged(int)), this, SLOT(show_status_bar_fullscreen_changed(int)));

    // set values from config

    m_pause_simulation->setChecked(m_user_value_store->get_value(qt_settings_misc_pause_simulation, false).toBool());
    m_synchronize_simulation->setChecked(m_user_value_store->get_value(qt_settings_misc_synchronize_simulation, true).toBool());
    m_show_menu_bar->setChecked(m_user_value_store->get_value(qt_settings_misc_menu_bar, true).toBool());
    m_show_status_bar->setChecked(m_user_value_store->get_value(qt_settings_misc_status_bar, true).toBool());
    m_show_menu_bar_fullscreen->setChecked(m_user_value_store->get_value(qt_settings_misc_menu_bar_fullscreen, true).toBool());
    m_show_status_bar_fullscreen->setChecked(m_user_value_store->get_value(qt_settings_misc_status_bar_fullscreen, true).toBool());
}





//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

bool age::qt_settings_miscellaneous::show_menu_bar(bool fullscreen) const
{
    QCheckBox *check_box = fullscreen ? m_show_menu_bar_fullscreen : m_show_menu_bar;
    bool checked = is_checked(check_box->checkState());
    return checked;
}

bool age::qt_settings_miscellaneous::show_status_bar(bool fullscreen) const
{
    QCheckBox *check_box = fullscreen ? m_show_status_bar_fullscreen : m_show_status_bar;
    bool checked = is_checked(check_box->checkState());
    return checked;
}



void age::qt_settings_miscellaneous::toggle_pause_simulation()
{
    m_pause_simulation->toggle();
}

void age::qt_settings_miscellaneous::toggle_synchronize_simulation()
{
    m_synchronize_simulation->toggle();
}

void age::qt_settings_miscellaneous::set_pause_simulation(bool pause_simulation)
{
    m_pause_simulation->setChecked(pause_simulation);
}

void age::qt_settings_miscellaneous::emit_settings_signals()
{
    LOG("emitting settings signals");

    emit pause_simulation_changed(m_pause_simulation->isChecked());
    emit synchronize_simulation_changed(m_synchronize_simulation->isChecked());
    emit show_menu_bar_changed(m_show_menu_bar->isChecked());
    emit show_status_bar_changed(m_show_status_bar->isChecked());
    emit show_menu_bar_fullscreen_changed(m_show_menu_bar_fullscreen->isChecked());
    emit show_status_bar_fullscreen_changed(m_show_status_bar_fullscreen->isChecked());
}





//---------------------------------------------------------
//
//   private slots
//
//---------------------------------------------------------

void age::qt_settings_miscellaneous::pause_simulation_changed(int state)
{
    bool checked = is_checked(state);
    m_user_value_store->set_value(qt_settings_misc_pause_simulation, checked);

    LOG(checked);
    emit pause_simulation_changed(checked);
}

void age::qt_settings_miscellaneous::synchronize_simulation_changed(int state)
{
    bool checked = is_checked(state);
    m_user_value_store->set_value(qt_settings_misc_synchronize_simulation, checked);

    LOG(checked);
    emit synchronize_simulation_changed(checked);
}

void age::qt_settings_miscellaneous::show_menu_bar_changed(int state)
{
    bool checked = is_checked(state);
    m_user_value_store->set_value(qt_settings_misc_menu_bar, checked);

    LOG(checked);
    emit show_menu_bar_changed(checked);
}

void age::qt_settings_miscellaneous::show_status_bar_changed(int state)
{
    bool checked = is_checked(state);
    m_user_value_store->set_value(qt_settings_misc_status_bar, checked);

    LOG(checked);
    emit show_status_bar_changed(checked);
}

void age::qt_settings_miscellaneous::show_menu_bar_fullscreen_changed(int state)
{
    bool checked = is_checked(state);
    m_user_value_store->set_value(qt_settings_misc_menu_bar_fullscreen, checked);

    LOG(checked);
    emit show_menu_bar_fullscreen_changed(checked);
}

void age::qt_settings_miscellaneous::show_status_bar_fullscreen_changed(int state)
{
    bool checked = is_checked(state);
    m_user_value_store->set_value(qt_settings_misc_status_bar_fullscreen, checked);

    LOG(checked);
    emit show_status_bar_fullscreen_changed(checked);
}
