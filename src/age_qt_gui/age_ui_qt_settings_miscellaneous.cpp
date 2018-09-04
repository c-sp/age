//
// Copyright 2018 Christoph Sprenger
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

#include <QGroupBox>
#include <QVBoxLayout>

#include <age_debug.hpp>

#include "age_ui_qt_settings.hpp"

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif

constexpr const char *qt_settings_misc_pause_emulator = "miscellaneous/pause_emulator";
constexpr const char *qt_settings_misc_synchronize_emulator = "miscellaneous/synchronize_emulator";
constexpr const char *qt_settings_misc_menu_bar = "miscellaneous/menu_bar";
constexpr const char *qt_settings_misc_status_bar = "miscellaneous/status_bar";
constexpr const char *qt_settings_misc_menu_bar_fullscreen = "miscellaneous/menu_bar_fullscreen";
constexpr const char *qt_settings_misc_status_bar_fullscreen = "miscellaneous/status_bar_fullscreen";





//---------------------------------------------------------
//
//   Object creation/destruction
//
//---------------------------------------------------------

age::qt_settings_miscellaneous::qt_settings_miscellaneous(QSharedPointer<qt_user_value_store> user_value_store, QWidget *parent, Qt::WindowFlags flags)
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
