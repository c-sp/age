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
    // emulation

    m_pause_emulator = new QCheckBox("pause emulator");
    m_synchronize_emulator = new QCheckBox("synchronize emulator to clock");

    QVBoxLayout *emulator_layout = new QVBoxLayout;
    emulator_layout->setAlignment(Qt::AlignCenter);
    emulator_layout->addWidget(m_pause_emulator);
    emulator_layout->addWidget(m_synchronize_emulator);

    QGroupBox *emulator_group = new QGroupBox("emulator");
    emulator_group->setLayout(emulator_layout);

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
    layout->addWidget(emulator_group, 0, 0);
    layout->addWidget(window_group, 1, 0);
    setLayout(layout);

    // connect signals to slots

    connect(m_pause_emulator, SIGNAL(stateChanged(int)), this, SLOT(pause_emulator_changed(int)));
    connect(m_synchronize_emulator, SIGNAL(stateChanged(int)), this, SLOT(synchronize_emulator_changed(int)));
    connect(m_show_menu_bar, SIGNAL(stateChanged(int)), this, SLOT(show_menu_bar_changed(int)));
    connect(m_show_status_bar, SIGNAL(stateChanged(int)), this, SLOT(show_status_bar_changed(int)));
    connect(m_show_menu_bar_fullscreen, SIGNAL(stateChanged(int)), this, SLOT(show_menu_bar_fullscreen_changed(int)));
    connect(m_show_status_bar_fullscreen, SIGNAL(stateChanged(int)), this, SLOT(show_status_bar_fullscreen_changed(int)));

    // set values from config

    m_pause_emulator->setChecked(m_user_value_store->get_value(qt_settings_misc_pause_emulator, false).toBool());
    m_synchronize_emulator->setChecked(m_user_value_store->get_value(qt_settings_misc_synchronize_emulator, true).toBool());
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



void age::qt_settings_miscellaneous::toggle_pause_emulator()
{
    m_pause_emulator->toggle();
}

void age::qt_settings_miscellaneous::toggle_synchronize_emulator()
{
    m_synchronize_emulator->toggle();
}

void age::qt_settings_miscellaneous::set_pause_emulator(bool pause_emulator)
{
    m_pause_emulator->setChecked(pause_emulator);
}

void age::qt_settings_miscellaneous::emit_settings_signals()
{
    LOG("emitting settings signals");

    emit pause_emulator_changed(m_pause_emulator->isChecked());
    emit synchronize_emulator_changed(m_synchronize_emulator->isChecked());
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

void age::qt_settings_miscellaneous::pause_emulator_changed(int state)
{
    bool checked = is_checked(state);
    m_user_value_store->set_value(qt_settings_misc_pause_emulator, checked);

    LOG(checked);
    emit pause_emulator_changed(checked);
}

void age::qt_settings_miscellaneous::synchronize_emulator_changed(int state)
{
    bool checked = is_checked(state);
    m_user_value_store->set_value(qt_settings_misc_synchronize_emulator, checked);

    LOG(checked);
    emit synchronize_emulator_changed(checked);
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
