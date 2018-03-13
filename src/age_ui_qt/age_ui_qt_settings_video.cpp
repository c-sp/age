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

#include <QDrag>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QGroupBox>
#include <QMimeData>

#include "age_ui_qt_settings.hpp"

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif

#if 0
#define LOG_DRAG(x) LOG(x)
#else
#define LOG_DRAG(x)
#endif





//---------------------------------------------------------
//
//   Object creation/destruction
//
//---------------------------------------------------------

age::qt_settings_video::qt_settings_video(std::shared_ptr<qt_user_value_store> user_value_store, GLint max_texture_size, QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_user_value_store(user_value_store),
      m_max_texture_size(max_texture_size)
{
    // frame filtering

    m_use_filter_chain = new QCheckBox("use filter chain");
    m_use_bilinear_filter = new QCheckBox("use bilinear filter");

    QVBoxLayout *frame_filter_layout = new QVBoxLayout;
    frame_filter_layout->setAlignment(Qt::AlignCenter);
    frame_filter_layout->addWidget(m_use_filter_chain);
    frame_filter_layout->addWidget(m_use_bilinear_filter);

    QGroupBox *frame_filter_group = new QGroupBox("frame filter");
    frame_filter_group->setLayout(frame_filter_layout);

    // blend frames

    QVBoxLayout *frame_blend_layout = new QVBoxLayout;
    frame_blend_layout->setAlignment(Qt::AlignCenter);

    for (int i = 0, blend_frames_size = m_blend_frames.size(); i < blend_frames_size; ++i)
    {
        QString text = (i == 0) ? "do not blend frames" : QString("blend ") + QString::number(i + 1) + " frames";
        m_blend_frames[i] = new QRadioButton(text);
        m_blend_frames[i]->setProperty(qt_settings_property_frames_to_blend, i + 1);
        frame_blend_layout->addWidget(m_blend_frames[i]);
    }

    QGroupBox *blend_frames_group = new QGroupBox("blend frames");
    blend_frames_group->setLayout(frame_blend_layout);

    // filter chain buttons

    QString max_tex_size = QString::number(max_texture_size);
    QLabel *max_frame_size = new QLabel(QString("maximal frame size:\n") + max_tex_size + " x " + max_tex_size);
    max_frame_size->setAlignment(Qt::AlignCenter);

    QPushButton *add_scale2x = new QPushButton(QString("add ") + get_name_for_qt_filter(qt_filter::scale2x));
    QPushButton *add_age_scale2x = new QPushButton(QString("add ") + get_name_for_qt_filter(qt_filter::age_scale2x));
    QPushButton *add_weak_blur = new QPushButton(QString("add ") + get_name_for_qt_filter(qt_filter::gauss3x3));
    QPushButton *add_strong_blur = new QPushButton(QString("add ") + get_name_for_qt_filter(qt_filter::gauss5x5));
    QPushButton *add_weak_emboss = new QPushButton(QString("add ") + get_name_for_qt_filter(qt_filter::emboss3x3));
    QPushButton *add_strong_emboss = new QPushButton(QString("add ") + get_name_for_qt_filter(qt_filter::emboss5x5));

    add_scale2x->setProperty(qt_settings_property_filter, get_name_for_qt_filter(qt_filter::scale2x));
    add_age_scale2x->setProperty(qt_settings_property_filter, get_name_for_qt_filter(qt_filter::age_scale2x));
    add_weak_blur->setProperty(qt_settings_property_filter, get_name_for_qt_filter(qt_filter::gauss3x3));
    add_strong_blur->setProperty(qt_settings_property_filter, get_name_for_qt_filter(qt_filter::gauss5x5));
    add_weak_emboss->setProperty(qt_settings_property_filter, get_name_for_qt_filter(qt_filter::emboss3x3));
    add_strong_emboss->setProperty(qt_settings_property_filter, get_name_for_qt_filter(qt_filter::emboss5x5));

    QVBoxLayout *filter_chain_buttons_layout = new QVBoxLayout;
    filter_chain_buttons_layout->addWidget(add_scale2x);
    filter_chain_buttons_layout->addWidget(add_age_scale2x);
    filter_chain_buttons_layout->addSpacing(qt_settings_element_spacing);
    filter_chain_buttons_layout->addWidget(add_weak_blur);
    filter_chain_buttons_layout->addWidget(add_strong_blur);
    filter_chain_buttons_layout->addSpacing(qt_settings_element_spacing);
    filter_chain_buttons_layout->addWidget(add_weak_emboss);
    filter_chain_buttons_layout->addWidget(add_strong_emboss);
    filter_chain_buttons_layout->addSpacing(qt_settings_element_spacing);
    filter_chain_buttons_layout->addWidget(max_frame_size);
    filter_chain_buttons_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

    // filter chain

    QVBoxLayout *filter_chain_layout = new QVBoxLayout;
    filter_chain_layout->setSpacing(0);

    for (uint i = 0; i < m_filter_widgets.size(); ++i)
    {
        m_filter_widgets[i] = new qt_filter_widget(i);
        filter_chain_layout->addWidget(m_filter_widgets[i]);
    }

    filter_chain_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QHBoxLayout *filter_layout = new QHBoxLayout;
    filter_layout->addLayout(filter_chain_buttons_layout);
    filter_layout->addSpacing(qt_settings_element_spacing);
    filter_layout->addLayout(filter_chain_layout);

    QGroupBox *filter_group = new QGroupBox("filter chain");
    filter_group->setLayout(filter_layout);

    // create final layout

    QGridLayout *layout = new QGridLayout;
    layout->setContentsMargins(qt_settings_layout_margin, qt_settings_layout_margin, qt_settings_layout_margin, qt_settings_layout_margin);
    layout->setSpacing(qt_settings_layout_spacing);
    layout->addWidget(frame_filter_group, 0, 0);
    layout->addWidget(blend_frames_group, 1, 0);
    layout->addWidget(filter_group, 0, 1, 2, 1);
    setLayout(layout);

    // connect signals to slots

    connect(m_use_filter_chain, SIGNAL(stateChanged(int)), this, SLOT(filter_chain_state_changed(int)));
    connect(m_use_bilinear_filter, SIGNAL(stateChanged(int)), this, SLOT(bilinear_filter_state_changed(int)));

    for (QRadioButton *button : m_blend_frames)
    {
        connect(button, SIGNAL(toggled(bool)), this, SLOT(frames_to_blend_toggled(bool)));
    }

    connect(add_scale2x, SIGNAL(clicked(bool)), this, SLOT(add_filter_clicked()));
    connect(add_age_scale2x, SIGNAL(clicked(bool)), this, SLOT(add_filter_clicked()));
    connect(add_weak_blur, SIGNAL(clicked(bool)), this, SLOT(add_filter_clicked()));
    connect(add_strong_blur, SIGNAL(clicked(bool)), this, SLOT(add_filter_clicked()));
    connect(add_weak_emboss, SIGNAL(clicked(bool)), this, SLOT(add_filter_clicked()));
    connect(add_strong_emboss, SIGNAL(clicked(bool)), this, SLOT(add_filter_clicked()));

    for (uint i = 0; i < m_filter_widgets.size(); ++i)
    {
        connect(m_filter_widgets[i], SIGNAL(remove(uint)), this, SLOT(remove(uint)));
        connect(m_filter_widgets[i], SIGNAL(drag_start(uint)), this, SLOT(drag_start(uint)));
        connect(m_filter_widgets[i], SIGNAL(drag_clear()), this, SLOT(drag_clear()));
        connect(m_filter_widgets[i], SIGNAL(drag_update(uint)), this, SLOT(drag_update(uint)));
        connect(m_filter_widgets[i], SIGNAL(drag_finish(bool)), this, SLOT(drag_finish(bool)));
    }

    // set values from config

    m_use_filter_chain->setChecked(m_user_value_store->get_value(qt_settings_video_use_filter_chain, true).toBool());
    m_use_bilinear_filter->setChecked(m_user_value_store->get_value(qt_settings_video_bilinear_filter, true).toBool());

    m_frames_to_blend = m_user_value_store->get_value(qt_settings_video_frames_to_blend, 2).toInt();
    m_frames_to_blend = std::max(1, std::min(static_cast<int>(m_blend_frames.size()), m_frames_to_blend));
    m_blend_frames[m_frames_to_blend - 1]->setChecked(true);

    QString default_filter_chain =
            get_name_for_qt_filter(qt_filter::age_scale2x)
            + ","
            + get_name_for_qt_filter(qt_filter::age_scale2x)
            + ","
            + get_name_for_qt_filter(qt_filter::age_scale2x)
            + ","
            + get_name_for_qt_filter(qt_filter::age_scale2x)
            + ","
            + get_name_for_qt_filter(qt_filter::gauss5x5)
            ;
    QString filter_chain = m_user_value_store->get_value(qt_settings_video_filter_chain, default_filter_chain).toString();
    QStringList filters = filter_chain.split(",");

    for (int i = 0; (i < filters.size()) && (m_filter_chain.size() < m_filter_widgets.size()); ++i)
    {
        QString filter_name = filters.at(i).trimmed();
        qt_filter filter = get_qt_filter_for_name(filter_name);

        if (filter != qt_filter::none)
        {
            m_filter_chain.push_back(filter);
        }
    }

    update_filter_chain_controls(true);
}





//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

void age::qt_settings_video::set_emulator_screen_size(GLint width, GLint height)
{
    m_emulator_screen_width = width;
    m_emulator_screen_height = height;

    update_filter_chain_controls(true);
}



void age::qt_settings_video::toggle_filter_chain()
{
    m_use_filter_chain->toggle();
}

void age::qt_settings_video::toggle_bilinear_filter()
{
    m_use_bilinear_filter->toggle();
}

void age::qt_settings_video::cycle_frames_to_blend()
{
    m_frames_to_blend = (m_frames_to_blend % m_blend_frames.size()) + 1;
    m_blend_frames[m_frames_to_blend - 1]->setChecked(true);
}



void age::qt_settings_video::emit_settings_signals()
{
    LOG("emitting video settings signals");

    emit use_bilinear_filter_changed(m_use_bilinear_filter->isChecked());
    emit frames_to_blend_changed(m_frames_to_blend);
    emit_filter_chain_changed();
}



QString age::qt_settings_video::get_name_for_qt_filter(qt_filter filter)
{
    QString result;

    switch (filter)
    {
    case qt_filter::scale2x: result = qt_filter_name_scale2x; break;
    case qt_filter::age_scale2x: result = qt_filter_name_age_scale2x; break;
    case qt_filter::gauss3x3: result = qt_filter_name_gauss3x3; break;
    case qt_filter::gauss5x5: result = qt_filter_name_gauss5x5; break;
    case qt_filter::emboss3x3: result = qt_filter_name_emboss3x3; break;
    case qt_filter::emboss5x5: result = qt_filter_name_emboss5x5; break;
    default: result = qt_filter_name_none; break;
    }

    return result;
}





//---------------------------------------------------------
//
//   private slots
//
//---------------------------------------------------------

void age::qt_settings_video::filter_chain_state_changed(int state)
{
    bool checked = is_checked(state);
    m_user_value_store->set_value(qt_settings_video_use_filter_chain, checked);

    LOG(checked);
    emit_filter_chain_changed();
}

void age::qt_settings_video::bilinear_filter_state_changed(int state)
{
    bool checked = is_checked(state);
    m_user_value_store->set_value(qt_settings_video_bilinear_filter, checked);

    LOG(checked);
    emit use_bilinear_filter_changed(checked);
}

void age::qt_settings_video::frames_to_blend_toggled(bool checked)
{
    if (checked)
    {
        QObject *obj = sender();
        QVariant frames_to_blend_var = obj->property(qt_settings_property_frames_to_blend);
        m_frames_to_blend = frames_to_blend_var.toInt();

        LOG(m_frames_to_blend);
        m_user_value_store->set_value(qt_settings_video_frames_to_blend, m_frames_to_blend);
        emit frames_to_blend_changed(m_frames_to_blend);
    }
}

void age::qt_settings_video::add_filter_clicked()
{
    if (m_filter_chain.size() < m_filter_widgets.size())
    {
        QObject *obj = sender();
        QVariant filter_name_var = obj->property(qt_settings_property_filter);
        QString filter_name = filter_name_var.toString();
        LOG(filter_name);

        qt_filter filter = get_qt_filter_for_name(filter_name);

        if (filter != qt_filter::none)
        {
            m_filter_chain.push_back(filter);
            update_filter_chain_controls(false);
        }
    }
}



void age::qt_settings_video::remove(uint index)
{
    LOG(index);
    m_filter_chain.erase(m_filter_chain.begin() + index);
    update_filter_chain_controls(false);
}

void age::qt_settings_video::drag_start(uint drag_index)
{
    LOG("drag_start " << drag_index);
    m_dnd_original_filter_chain = m_filter_chain;
    m_dnd_index = drag_index;
}

void age::qt_settings_video::drag_clear()
{
    LOG("drag_clear");
    m_filter_chain = m_dnd_original_filter_chain;
    update_filter_chain_controls(false);
}

void age::qt_settings_video::drag_update(uint insert_at_index)
{
    LOG("drag_update " << insert_at_index);

    m_filter_chain = m_dnd_original_filter_chain;
    qt_filter filter = m_filter_chain[m_dnd_index];
    m_filter_chain.erase(m_filter_chain.begin() + m_dnd_index);
    m_filter_chain.insert(m_filter_chain.begin() + insert_at_index, filter);

    update_filter_chain_controls(false);
}

void age::qt_settings_video::drag_finish(bool keep)
{
    LOG("drag_finish " << keep);

    if (!keep)
    {
        m_filter_chain = m_dnd_original_filter_chain;
        update_filter_chain_controls(false);
    }
}





//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

age::qt_filter age::qt_settings_video::get_qt_filter_for_name(const QString &name)
{
    qt_filter result;

    if (0 == name.compare(qt_filter_name_scale2x, Qt::CaseInsensitive))
    {
        result = qt_filter::scale2x;
    }
    else if (0 == name.compare(qt_filter_name_age_scale2x, Qt::CaseInsensitive))
    {
        result = qt_filter::age_scale2x;
    }
    else if (0 == name.compare(qt_filter_name_gauss3x3, Qt::CaseInsensitive))
    {
        result = qt_filter::gauss3x3;
    }
    else if (0 == name.compare(qt_filter_name_gauss5x5, Qt::CaseInsensitive))
    {
        result = qt_filter::gauss5x5;
    }
    else if (0 == name.compare(qt_filter_name_emboss3x3, Qt::CaseInsensitive))
    {
        result = qt_filter::emboss3x3;
    }
    else if (0 == name.compare(qt_filter_name_emboss5x5, Qt::CaseInsensitive))
    {
        result = qt_filter::emboss5x5;
    }
    else
    {
        result = qt_filter::none;
    }

    return result;
}



void age::qt_settings_video::update_filter_chain_controls(bool only_controls)
{
    uint i = 0;
    QString filter_string;

    GLint width = m_emulator_screen_width;
    GLint height = m_emulator_screen_height;

    // adjust filter chain widgets & create settings value
    for (qt_filter filter : m_filter_chain)
    {
        if (i > 0)
        {
            filter_string += ",";
        }
        filter_string += get_name_for_qt_filter(filter);

        GLint factor = get_qt_filter_factor(filter);
        width *= factor;
        height *= factor;

        m_filter_widgets[i]->set_filter(filter, width, height);
        ++i;
    }

    // clear unused widgets
    for (; i < m_filter_widgets.size(); ++i)
    {
        m_filter_widgets[i]->set_filter(qt_filter::none, 0, 0);
    }

    // emit filter chain signal & store settings value
    if (!only_controls)
    {
        m_user_value_store->set_value(qt_settings_video_filter_chain, filter_string);
        emit_filter_chain_changed();
    }
}



void age::qt_settings_video::emit_filter_chain_changed()
{
    emit filter_chain_changed(m_use_filter_chain->isChecked() ? m_filter_chain : qt_filter_vector());
}





//---------------------------------------------------------
//
//   filter frame
//
//---------------------------------------------------------

age::qt_filter_widget::qt_filter_widget(uint widget_index, QWidget *parent, Qt::WindowFlags flags)
    : QFrame(parent, flags),
      m_widget_index(widget_index)
{
    m_text = new QLabel("text text text");
    m_remove = new QPushButton("X");
    m_remove->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_text);
    layout->addWidget(m_remove);

    setLayout(layout);
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    setAcceptDrops(true);

    connect(m_remove, SIGNAL(clicked(bool)), this, SLOT(remove_clicked()));

    set_filter(qt_filter::none, 0, 0);
}



void age::qt_filter_widget::set_filter(qt_filter filter, GLint width, GLint height)
{
    QString text;

    if (filter != qt_filter::none)
    {
        text = QString::number(m_widget_index + 1);
        text += ". ";
        text += qt_settings_video::get_name_for_qt_filter(filter);
        if ((width > 0) && (height > 0))
        {
            text += "  (";
            text += QString::number(width) + " x " + QString::number(height);
            text += ")";
        }
    }
    else
    {
        // if we leave this empty ("") instead, the widget size may change when setting a filter
        text = " ";
    }

    m_has_content = filter != qt_filter::none;
    m_text->setText(text);
    m_remove->setEnabled(m_has_content);
}



void age::qt_filter_widget::dragEnterEvent(QDragEnterEvent *event)
{
    LOG_DRAG("dragEnterEvent " << m_widget_index);
    bool handled = handle_drag_event(event);
    if (handled)
    {
        emit drag_update(m_widget_index);
    }
}

void age::qt_filter_widget::dragLeaveEvent(QDragLeaveEvent*)
{
    LOG_DRAG("dragLeaveEvent " << m_widget_index);
    emit drag_clear();
}

void age::qt_filter_widget::dropEvent(QDropEvent *event)
{
    LOG_DRAG("dropEvent " << m_widget_index);
    handle_drag_event(event);
}

void age::qt_filter_widget::mousePressEvent(QMouseEvent*)
{
    LOG_DRAG("mousePressEvent " << m_widget_index);

    if (m_has_content)
    {
        // get the current widget as pixmap
        QPixmap pixmap = grab();

        // serialize the pixmap
        QByteArray itemData;
        QDataStream dataStream(&itemData, QIODevice::WriteOnly);
        dataStream << pixmap;

        // create mime data containing the pixmap
        QMimeData *mimeData = new QMimeData;
        mimeData->setData(qt_settings_dnd_mime_type, itemData);

        // start drag and drop action
        emit drag_start(m_widget_index);

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(pixmap);

        Qt::DropAction action = drag->exec();

        // finish
        LOG_DRAG("action " << action);
        emit drag_finish(action > 0);
    }
}



void age::qt_filter_widget::remove_clicked()
{
    emit remove(m_widget_index);
}



bool age::qt_filter_widget::handle_drag_event(QDropEvent *event)
{
    bool handled;

    // accept event?
    bool known_mime_type = event->mimeData()->hasFormat(qt_settings_dnd_mime_type);

    if (known_mime_type && m_has_content)
    {
        event->acceptProposedAction();
        handled = true;
    }

    // ignore events
    else
    {
        event->ignore();
        handled = false;
    }

    return handled;
}
