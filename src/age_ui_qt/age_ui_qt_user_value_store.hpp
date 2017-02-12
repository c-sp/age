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

#ifndef AGE_UI_QT_USER_VALUES_HPP
#define AGE_UI_QT_USER_VALUES_HPP

//!
//! \file
//!

#include "age_ui_qt.hpp"



namespace age
{

//!
//! \brief Access to user specific values is handled by this class.
//!
//! User specific values (e.g. settings, persistent ram) is accessed over this class.
//! The values are stored separately for each user.
//! To not lose all values when the program exists, a persistent value store is used.
//!
class qt_user_value_store : public non_copyable
{
public:

    qt_user_value_store();
    ~qt_user_value_store();

    //!
    //! \brief Get the speficied user value.
    //!
    //! The value might be returned from memory instead of the persistent value store.
    //! Thus it might not be equal to the most recent value found in the persistent value store.
    //! See sync() for more information.
    //!
    //! \param key The unique identifier of the user value to get.
    //! \param default_value The default value to return if the user value has not yet been set.
    //! \return The requested user value.
    //!
    QVariant get_value(const QString &key, const QVariant &default_value = QVariant()) const;

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
    bool set_value(const QString &key, const QVariant &value);

    //!
    //! \brief Synchronize the persistent value store and all in-memory copies of user values.
    //!
    void sync();

private:

    void make_user_directory();

    const QString m_user_value_directory;
    std::shared_ptr<QSettings> m_settings = nullptr;
};

} // namespace age



#endif // AGE_UI_QT_USER_VALUES_HPP
