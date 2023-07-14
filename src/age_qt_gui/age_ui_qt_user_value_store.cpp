//
// © 2017 Christoph Sprenger <https://github.com/c-sp>
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

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QMetaType>

#include "age_ui_qt.hpp"
#include "age_ui_qt_user_value_store.hpp"

constexpr const char* user_value_directory = ".age_emulator";





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

    m_settings = QSharedPointer<QSettings>(new QSettings(settings_file, QSettings::IniFormat));
}



age::qt_user_value_store::~qt_user_value_store()
{
    // force writing the complete settings file by re-setting all current values
    for (const auto& key : m_settings->allKeys())
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

QVariant age::qt_user_value_store::get_value(const QString& key, const QVariant& default_value) const
{
    QVariant result;

    // check the settings file for the specified key
    if (m_settings != nullptr)
    {
        result = m_settings->value(key);
    }

    // if the value was not found in the settings file, try to find a value file
    if (!result.isValid())
    {
        QString file_path = m_user_value_directory + key;

        QFile file = {file_path};
        if (file.open(QIODevice::ReadOnly))
        {
            // do not load the file, if it exceeds the maximal number of bytes
            qint64 file_size = file.size();

            if (file_size <= max_file_bytes)
            {
                result = file.readAll();
            }
        }
    }

    // return what we found
    return result.isValid() ? result : default_value;
}



bool age::qt_user_value_store::set_value(const QString& key, const QVariant& value)
{
    bool saved = false;

    // if this is a byte array, store it as extra file
    if (QMetaType::QByteArray == value.userType())
    {
        make_user_directory(); // make sure the user value directory exists

        QString file_path = m_user_value_directory + key;

        QFile file = {file_path};
        if (file.open(QIODevice::WriteOnly))
        {

            QByteArray bytes = value.toByteArray();
            file.write(bytes.data(), bytes.size());

            // remove the key from the settings file to prevent any confusion
            m_settings->remove(key);
            saved = true;
        }
    }

    // store the value to the settings file, if it is valid
    else if (value.isValid())
    {
        m_settings->setValue(key, value);
        saved = true;
    }

    // return success indicator
    return saved;
}



void age::qt_user_value_store::sync()
{
    if (m_settings != nullptr)
    {
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
        home.mkpath(user_value_directory);
    }
}
