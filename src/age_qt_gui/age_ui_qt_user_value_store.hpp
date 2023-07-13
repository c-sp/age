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

#ifndef AGE_UI_QT_USER_VALUE_STORE_HPP
#define AGE_UI_QT_USER_VALUE_STORE_HPP

//!
//! \file
//!

#include <QSettings>
#include <QSharedPointer>
#include <QString>
#include <QVariant>

#include <age_types.hpp>



namespace age
{

    //!
    //! \brief Access to user specific values is handled by this class.
    //!
    //! User specific values (e.g. settings, persistent ram) is accessed over this class.
    //! The values are stored separately for each user.
    //! To not lose all values when the program exists, a persistent value store is used.
    //!
    class qt_user_value_store
    {
        AGE_DISABLE_COPY(qt_user_value_store);

    public:
        qt_user_value_store();
        ~qt_user_value_store();

        //!
        //! \brief Get the specified user value.
        //!
        //! The value might be returned from memory instead of the persistent value store.
        //! Thus it might not be equal to the most recent value found in the persistent value store.
        //! See sync() for more information.
        //!
        //! \param key The unique identifier of the user value to get.
        //! \param default_value The default value to return if the user value has not yet been set.
        //! \return The requested user value.
        //!
        [[nodiscard]] QVariant get_value(const QString& key, const QVariant& default_value = QVariant()) const;

        //!
        //! \brief Set a user value.
        //!
        //! Instead of immediately updating the persistent value store, the value might be just stored to memory.
        //! To make sure all changed values are persisted, call sync().
        //!
        //! \param key The unique identifier of the user value to set.
        //! \param value The value to set.
        //! \return True if the value has been saved. False if the value could not be saved for some reason.
        //!
        bool set_value(const QString& key, const QVariant& value);

        //!
        //! \brief Synchronize the persistent value store and all in-memory copies of user values.
        //!
        void sync();

    private:
        void make_user_directory();

        const QString             m_user_value_directory;
        QSharedPointer<QSettings> m_settings = nullptr;
    };

} // namespace age



#endif // AGE_UI_QT_USER_VALUE_STORE_HPP
