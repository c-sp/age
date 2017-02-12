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

age::qt_settings_keys::qt_settings_keys(std::shared_ptr<qt_user_value_store> user_value_store, QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_user_value_store(user_value_store),

      m_category_strings({
                         std::pair<qt_key_event, const char*>(qt_key_event::gb_up, "Gameboy buttons"),
                         std::pair<qt_key_event, const char*>(qt_key_event::gb_down, "Gameboy buttons"),
                         std::pair<qt_key_event, const char*>(qt_key_event::gb_left, "Gameboy buttons"),
                         std::pair<qt_key_event, const char*>(qt_key_event::gb_right, "Gameboy buttons"),
                         std::pair<qt_key_event, const char*>(qt_key_event::gb_a, "Gameboy buttons"),
                         std::pair<qt_key_event, const char*>(qt_key_event::gb_b, "Gameboy buttons"),
                         std::pair<qt_key_event, const char*>(qt_key_event::gb_start, "Gameboy buttons"),
                         std::pair<qt_key_event, const char*>(qt_key_event::gb_select, "Gameboy buttons"),

                         std::pair<qt_key_event, const char*>(qt_key_event::video_toggle_filter_chain, "video settings"),
                         std::pair<qt_key_event, const char*>(qt_key_event::video_toggle_bilinear_filter, "video settings"),
                         std::pair<qt_key_event, const char*>(qt_key_event::video_cycle_frames_to_blend, "video settings"),

                         std::pair<qt_key_event, const char*>(qt_key_event::audio_toggle_mute, "audio settings"),
                         std::pair<qt_key_event, const char*>(qt_key_event::audio_increase_volume, "audio settings"),
                         std::pair<qt_key_event, const char*>(qt_key_event::audio_decrease_volume, "audio settings"),

                         std::pair<qt_key_event, const char*>(qt_key_event::misc_toggle_pause_simulation, "miscellaneous settings"),
                         std::pair<qt_key_event, const char*>(qt_key_event::misc_toggle_synchronize_simulation, "miscellaneous settings")
                         }),

      m_event_strings({
                      std::pair<qt_key_event, const char*>(qt_key_event::gb_up, "up"),
                      std::pair<qt_key_event, const char*>(qt_key_event::gb_down, "down"),
                      std::pair<qt_key_event, const char*>(qt_key_event::gb_left, "left"),
                      std::pair<qt_key_event, const char*>(qt_key_event::gb_right, "right"),
                      std::pair<qt_key_event, const char*>(qt_key_event::gb_a, "a"),
                      std::pair<qt_key_event, const char*>(qt_key_event::gb_b, "b"),
                      std::pair<qt_key_event, const char*>(qt_key_event::gb_start, "start"),
                      std::pair<qt_key_event, const char*>(qt_key_event::gb_select, "select"),

                      std::pair<qt_key_event, const char*>(qt_key_event::video_toggle_filter_chain, "toggle filter chain"),
                      std::pair<qt_key_event, const char*>(qt_key_event::video_toggle_bilinear_filter, "toggle bilinear filter"),
                      std::pair<qt_key_event, const char*>(qt_key_event::video_cycle_frames_to_blend, "cycle frames to blend"),

                      std::pair<qt_key_event, const char*>(qt_key_event::audio_toggle_mute, "toggle mute"),
                      std::pair<qt_key_event, const char*>(qt_key_event::audio_increase_volume, "increase volume"),
                      std::pair<qt_key_event, const char*>(qt_key_event::audio_decrease_volume, "decrease volume"),

                      std::pair<qt_key_event, const char*>(qt_key_event::misc_toggle_pause_simulation, "toggle pause simulation"),
                      std::pair<qt_key_event, const char*>(qt_key_event::misc_toggle_synchronize_simulation, "toggle synchronize simulation clock")
                      }),

      m_event_setting_strings({
                              std::pair<qt_key_event, const char*>(qt_key_event::gb_up, qt_settings_keys_gameboy_up),
                              std::pair<qt_key_event, const char*>(qt_key_event::gb_down, qt_settings_keys_gameboy_down),
                              std::pair<qt_key_event, const char*>(qt_key_event::gb_left, qt_settings_keys_gameboy_left),
                              std::pair<qt_key_event, const char*>(qt_key_event::gb_right, qt_settings_keys_gameboy_right),
                              std::pair<qt_key_event, const char*>(qt_key_event::gb_a, qt_settings_keys_gameboy_a),
                              std::pair<qt_key_event, const char*>(qt_key_event::gb_b, qt_settings_keys_gameboy_b),
                              std::pair<qt_key_event, const char*>(qt_key_event::gb_start, qt_settings_keys_gameboy_start),
                              std::pair<qt_key_event, const char*>(qt_key_event::gb_select, qt_settings_keys_gameboy_select),

                              std::pair<qt_key_event, const char*>(qt_key_event::video_toggle_filter_chain, qt_settings_keys_video_toggle_filter_chain),
                              std::pair<qt_key_event, const char*>(qt_key_event::video_toggle_bilinear_filter, qt_settings_keys_video_toggle_bilinear_filter),
                              std::pair<qt_key_event, const char*>(qt_key_event::video_cycle_frames_to_blend, qt_settings_keys_video_cycle_frames_to_blend),

                              std::pair<qt_key_event, const char*>(qt_key_event::audio_toggle_mute, qt_settings_keys_audio_toggle_mute),
                              std::pair<qt_key_event, const char*>(qt_key_event::audio_increase_volume, qt_settings_keys_audio_increase_volume),
                              std::pair<qt_key_event, const char*>(qt_key_event::audio_decrease_volume, qt_settings_keys_audio_decrease_volume),

                              std::pair<qt_key_event, const char*>(qt_key_event::misc_toggle_pause_simulation, qt_settings_keys_misc_toggle_pause_simulation),
                              std::pair<qt_key_event, const char*>(qt_key_event::misc_toggle_synchronize_simulation, qt_settings_keys_misc_toggle_synchronize_simulation)
                              }),

      m_allowed_keys({
                     Qt::Key_Tab,
                     Qt::Key_Backspace,
                     Qt::Key_Return,
                     Qt::Key_Insert,
                     Qt::Key_Delete,
                     Qt::Key_Home,
                     Qt::Key_End,
                     Qt::Key_Left,
                     Qt::Key_Up,
                     Qt::Key_Right,
                     Qt::Key_Down,
                     Qt::Key_PageUp,
                     Qt::Key_PageDown,
                     Qt::Key_Shift,
                     Qt::Key_Control,
                     Qt::Key_Alt,

                     Qt::Key_F1,
                     Qt::Key_F2,
                     Qt::Key_F3,
                     Qt::Key_F4,
                     Qt::Key_F5,
                     Qt::Key_F6,
                     Qt::Key_F7,
                     Qt::Key_F8,
                     Qt::Key_F9,
                     Qt::Key_F10,
                     Qt::Key_F11,
                     Qt::Key_F12,

                     Qt::Key_Space,
                     Qt::Key_0,
                     Qt::Key_1,
                     Qt::Key_2,
                     Qt::Key_3,
                     Qt::Key_4,
                     Qt::Key_5,
                     Qt::Key_6,
                     Qt::Key_7,
                     Qt::Key_8,
                     Qt::Key_9,
                     Qt::Key_A,
                     Qt::Key_B,
                     Qt::Key_C,
                     Qt::Key_D,
                     Qt::Key_E,
                     Qt::Key_F,
                     Qt::Key_G,
                     Qt::Key_H,
                     Qt::Key_I,
                     Qt::Key_J,
                     Qt::Key_K,
                     Qt::Key_L,
                     Qt::Key_M,
                     Qt::Key_N,
                     Qt::Key_O,
                     Qt::Key_P,
                     Qt::Key_Q,
                     Qt::Key_R,
                     Qt::Key_S,
                     Qt::Key_T,
                     Qt::Key_U,
                     Qt::Key_V,
                     Qt::Key_W,
                     Qt::Key_X,
                     Qt::Key_Y,
                     Qt::Key_Z,
                     })
{
    // Gameboy keys

    QGridLayout *gameboy_keys_layout = new QGridLayout;
    add_key_widgets(gameboy_keys_layout, 0, qt_key_event::gb_up, Qt::Key_Up);
    add_key_widgets(gameboy_keys_layout, 1, qt_key_event::gb_down, Qt::Key_Down);
    add_key_widgets(gameboy_keys_layout, 2, qt_key_event::gb_left, Qt::Key_Left);
    add_key_widgets(gameboy_keys_layout, 3, qt_key_event::gb_right, Qt::Key_Right);
    gameboy_keys_layout->addItem(new QSpacerItem(1, qt_settings_element_spacing), 4, 0);
    add_key_widgets(gameboy_keys_layout, 5, qt_key_event::gb_a, Qt::Key_A);
    add_key_widgets(gameboy_keys_layout, 6, qt_key_event::gb_b, Qt::Key_S);
    gameboy_keys_layout->addItem(new QSpacerItem(1, qt_settings_element_spacing), 7, 0);
    add_key_widgets(gameboy_keys_layout, 8, qt_key_event::gb_start, Qt::Key_Space);
    add_key_widgets(gameboy_keys_layout, 9, qt_key_event::gb_select, Qt::Key_Return);

    const char *gameboy_cagegory = m_category_strings.value(qt_key_event::gb_up);
    QGroupBox *gameboy_keys_group = new QGroupBox(gameboy_cagegory);
    gameboy_keys_group->setLayout(gameboy_keys_layout);

    // video keys

    QGridLayout *video_keys_layout = new QGridLayout;
    add_key_widgets(video_keys_layout, 0, qt_key_event::video_toggle_filter_chain, Qt::Key_F2);
    add_key_widgets(video_keys_layout, 1, qt_key_event::video_toggle_bilinear_filter, Qt::Key_F3);
    add_key_widgets(video_keys_layout, 2, qt_key_event::video_cycle_frames_to_blend, Qt::Key_F4);

    const char *video_cagegory = m_category_strings.value(qt_key_event::video_toggle_filter_chain);
    QGroupBox *video_keys_group = new QGroupBox(video_cagegory);
    video_keys_group->setLayout(video_keys_layout);

    // audio keys

    QGridLayout *audio_keys_layout = new QGridLayout;
    add_key_widgets(audio_keys_layout, 0, qt_key_event::audio_toggle_mute, Qt::Key_F5);
    add_key_widgets(audio_keys_layout, 1, qt_key_event::audio_increase_volume, Qt::Key_PageUp);
    add_key_widgets(audio_keys_layout, 2, qt_key_event::audio_decrease_volume, Qt::Key_PageDown);

    const char *audio_cagegory = m_category_strings.value(qt_key_event::audio_toggle_mute);
    QGroupBox *audio_keys_group = new QGroupBox(audio_cagegory);
    audio_keys_group->setLayout(audio_keys_layout);

    // miscellaneous keys

    QGridLayout *misc_keys_layout = new QGridLayout;
    add_key_widgets(misc_keys_layout, 0, qt_key_event::misc_toggle_pause_simulation, Qt::Key_F9);
    add_key_widgets(misc_keys_layout, 1, qt_key_event::misc_toggle_synchronize_simulation, Qt::Key_F10);

    const char *misc_cagegory = m_category_strings.value(qt_key_event::misc_toggle_pause_simulation);
    QGroupBox *misc_keys_group = new QGroupBox(misc_cagegory);
    misc_keys_group->setLayout(misc_keys_layout);

    // create main layout

    QGridLayout *layout = new QGridLayout;
    layout->setContentsMargins(qt_settings_layout_margin, qt_settings_layout_margin, qt_settings_layout_margin, qt_settings_layout_margin);
    layout->setSpacing(qt_settings_layout_spacing);
    layout->addWidget(gameboy_keys_group, 0, 0, 3, 1);
    layout->addWidget(video_keys_group, 0, 1);
    layout->addWidget(audio_keys_group, 1, 1);
    layout->addWidget(misc_keys_group, 2, 1);

    // activate main layout

    setLayout(layout);
}





//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

age::qt_key_event age::qt_settings_keys::get_event_for_key(Qt::Key key) const
{
    return m_event_for_key.value(key, qt_key_event::none);
}





//---------------------------------------------------------
//
//   private slots
//
//---------------------------------------------------------

void age::qt_settings_keys::button_clicked(bool)
{
    QObject *object = sender();
    if (object != nullptr)
    {
        qt_key_event event = m_event_for_object.value(object, qt_key_event::none);
        if (event != qt_key_event::none) // should never happen
        {
            Qt::Key key = m_key_for_event.value(event, Qt::Key_unknown);

            QString event_string = get_event_string(event);
            QString key_string = get_key_string(key);

            qt_key_dialog dialog = {this, event_string, key_string};
            dialog.exec();

            Qt::Key new_key = dialog.get_key_pressed();
            LOG("new key binding for " << event_string << ": " << get_key_string(new_key));

            set_key_binding(event, new_key, true);
        }
    }
}





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

QString age::qt_settings_keys::get_event_string(qt_key_event event) const
{
    constexpr const char *qt_settings_key_none = "<none>";

    QString result;

    const char *category_string = m_category_strings.value(event, nullptr);
    if (category_string == nullptr)
    {
        result = qt_settings_key_none;
    }
    else
    {
        const char *event_string = m_event_strings.value(event, qt_settings_key_none);

        result = category_string;
        result += ", ";
        result += event_string;
    }

    return result;
}

QString age::qt_settings_keys::get_key_string(Qt::Key key) const
{
    QString result = QKeySequence(key).toString(QKeySequence::PortableText);
    return result;
}



void age::qt_settings_keys::add_key_widgets(QGridLayout *layout, int row, qt_key_event event, Qt::Key default_key)
{
    const char *event_string = m_event_strings.value(event);

    // create widgets
    QPushButton *button = new QPushButton;

    layout->addWidget(new QLabel(event_string), row, 0);
    layout->addWidget(button, row, 1);

    m_event_for_object.insert(button, event);
    m_button_for_event.insert(event, button);

    // connect to button's clicked() signal
    connect(button, SIGNAL(clicked(bool)), this, SLOT(button_clicked(bool)));

    // load key binding from settings
    const char *settings_key = m_event_setting_strings.value(event);
    QString key_string = m_user_value_store->get_value(settings_key).toString();
    QKeySequence key_sequence(key_string, QKeySequence::PortableText);
    int key_int = key_sequence[0];
    Qt::Key key = static_cast<Qt::Key>(key_int);

    if (!m_allowed_keys.contains(key))
    {
        key = default_key;
    }

    // save key binding
    set_key_binding(event, key, false);
}



void age::qt_settings_keys::set_key_binding(qt_key_event event, Qt::Key key, bool update_settings)
{
    LOG("binding key " << get_key_string(key) << " to event " << get_event_string(event));

    // set binding only, if both parameters are valid
    if (m_event_strings.contains(event) && m_allowed_keys.contains(key))
    {
        // remove all current mappings for the specified event and key
        qt_key_event old_event = get_event_for_key(key);
        Qt::Key old_key = m_key_for_event.value(event, Qt::Key_unknown);

        LOG("old event for key " << get_key_string(key) << ": " << get_event_string(old_event));
        LOG("old key for event " << get_event_string(event) << ": " << get_key_string(old_key));

        m_key_for_event.remove(old_event);
        m_event_for_key.remove(old_key);

        // save the new mapping
        m_key_for_event.insert(event, key);
        m_event_for_key.insert(key, event);

        // update widgets
        if (old_event != qt_key_event::none)
        {
            QPushButton *button = m_button_for_event.value(old_event);
            button->setText("?");
        }
        QPushButton *button = m_button_for_event.value(event);
        button->setText(get_key_string(key));

        // update settings
        if (update_settings)
        {
            save_key_binding(event);
            save_key_binding(old_event);
        }
    }
}



void age::qt_settings_keys::save_key_binding(qt_key_event event)
{
    if (event != qt_key_event::none)
    {
        Qt::Key key = m_key_for_event.value(event, Qt::Key_unknown);

        QString settings_value;
        if (key == Qt::Key_unknown)
        {
            settings_value = "";
        }
        else
        {
            QKeySequence key_sequence(key);
            settings_value = key_sequence.toString(QKeySequence::PortableText);
        }

        const char *settings_key = m_event_setting_strings.value(event);

        LOG("saving to settings: " << settings_key << "=" << settings_value);
        m_user_value_store->set_value(settings_key, settings_value);
    }
}





//---------------------------------------------------------
//
//   key dialog
//
//---------------------------------------------------------

age::qt_key_dialog::qt_key_dialog(qt_settings_keys *parent, const QString &event, const QString &key)
    : QDialog(parent),
      m_parent(parent)
{
    QGridLayout *m_key_layout = new QGridLayout;
    m_key_layout->addItem(new QSpacerItem(1, qt_settings_element_spacing), 0, 0);
    m_key_layout->addWidget(new QLabel("press new key to use for:"), 1, 0, Qt::AlignHCenter);
    m_key_layout->addWidget(new QLabel(event), 2, 0, Qt::AlignHCenter);
    m_key_layout->addItem(new QSpacerItem(1, qt_settings_element_spacing), 3, 0);
    m_key_layout->addWidget(new QLabel("current key is:"), 4, 0, Qt::AlignHCenter);
    m_key_layout->addWidget(new QLabel(key), 5, 0, Qt::AlignHCenter);
    m_key_layout->addItem(new QSpacerItem(1, qt_settings_element_spacing), 6, 0);
    m_key_layout->addWidget(new QLabel("(press ESC to cancel)"), 7, 0, Qt::AlignHCenter);
    m_key_layout->addItem(new QSpacerItem(1, qt_settings_element_spacing), 8, 0);

    setLayout(m_key_layout);
    setWindowTitle(QString(project_name) + " key binding");
}

Qt::Key age::qt_key_dialog::get_key_pressed() const
{
    return m_key_pressed;
}

void age::qt_key_dialog::keyPressEvent(QKeyEvent *key_event)
{
    m_key_pressed = static_cast<Qt::Key>(key_event->key());
    LOG("closing qt_key_dialog with m_key_pressed == " << m_key_pressed);
    close();
}
