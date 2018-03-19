//
// Copyright (c) 2010-2018 Christoph Sprenger
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

#ifndef AGE_NON_COPYABLE_HPP
#define AGE_NON_COPYABLE_HPP

//!
//! \file
//!



namespace age {

//!
//! \brief Base class for non-copyable instances.
//!
class non_copyable
{
public:

    non_copyable() {}

    non_copyable(const non_copyable&) = delete;
    non_copyable(non_copyable&&) = delete;
    non_copyable& operator=(const non_copyable&) = delete;
    non_copyable& operator=(non_copyable&&) = delete;
};

} // namespace age



#endif // AGE_NON_COPYABLE_HPP
