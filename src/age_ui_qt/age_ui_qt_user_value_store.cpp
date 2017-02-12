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

#include "age_ui_qt_user_value_store.hpp"

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

age::qt_user_value_store::qt_user_value_store()
    : m_user_value_directory(QDir::homePath() + QDir::separator() + user_value_directory + QDir::separator())
{
    // open settings.ini file within user value directory
    QString settings_file = m_user_value_directory + "settings.ini";
    LOG("using settings file " << std::quoted(settings_file.toStdString()));

    m_settings = std::allocate_shared<QSettings>(std::allocator<QSettings>(), settings_file, QSettings::IniFormat);
}



age::qt_user_value_store::~qt_user_value_store()
{
    // force writing the complete settings file by re-setting all current values
    for (auto key : m_settings->allKeys())
    {
        m_settings->setValue(key, m_settings->value(key));
    }

    sync();
}



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

QVariant age::qt_user_value_store::get_value(const QString &key, const QVariant &default_value) const
{
    QVariant result;

    // check the settings file for the specified key
    if (m_settings != nullptr)
    {
        LOG("looking in settings for key " << std::quoted(key.toStdString()));
        result = m_settings->value(key);
    }

    // if the value was not found in the settings file, try to find a value file
    if (!result.isValid())
    {
        QString file_path = m_user_value_directory + key;
        LOG("looking for user value file " << std::quoted(file_path.toStdString()));

        QFile file = {file_path};
        if (file.open(QIODevice::ReadOnly))
        {
            // do not load the file, if it exceeds the maximal number of bytes
            qint64 file_size = file.size();
            LOG("file has " << file_size << " bytes (max bytes allowed is " << max_file_bytes << ")");

            if (file_size <= max_file_bytes)
            {
                LOG("about to load user value file " << std::quoted(file.fileName().toStdString()));
                result = file.readAll();
            }
        }
    }

    // return what we found
    LOG((result.isValid() ? "" : "NO ") << "user value loaded for key " << std::quoted(key.toStdString()));
    return result.isValid() ? result : default_value;
}



bool age::qt_user_value_store::set_value(const QString &key, const QVariant &value)
{
    bool saved = false;

    // if this is a byte array, store it as extra file
    if (QMetaType::QByteArray == value.userType())
    {
        make_user_directory(); // make sure the user value directory exists

        QString file_path = m_user_value_directory + key;
        LOG("will try to write user value file " << std::quoted(file_path.toStdString()));

        QFile file = {file_path};
        if (file.open(QIODevice::WriteOnly))
        {
            LOG("writing user value file " << std::quoted(file_path.toStdString()));

            QByteArray bytes = value.toByteArray();
            file.write(bytes.data(), bytes.size());
            LOG("wrote " << bytes.size() << " bytes to " << std::quoted(file_path.toStdString()));

            // remove the key from the settings file to prevent any confusion
            LOG("removing key from settings file: " << std::quoted(key.toStdString()));
            m_settings->remove(key);
            saved = true;
        }
    }

    // store the value to the settings file, if it is valid
    else if (value.isValid())
    {
        LOG("writing user value to settings file, key " << std::quoted(key.toStdString()));
        m_settings->setValue(key, value);
        saved = true;
    }

    // return success indicator
    LOG((saved ? "" : "NOT ") << "saved user value for key " << std::quoted(key.toStdString()));
    return saved;
}



void age::qt_user_value_store::sync()
{
    if (m_settings != nullptr)
    {
        LOG("syncing settings file");
        m_settings->sync();
    }
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

void age::qt_user_value_store::make_user_directory()
{
    QDir home = QDir::home();

    if (!home.exists(user_value_directory))
    {
        LOG("making path " << std::quoted(m_user_value_directory.toStdString()));
        home.mkpath(user_value_directory);
    }
}
