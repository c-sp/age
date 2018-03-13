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

#ifndef AGE_UI_HPP
#define AGE_UI_HPP

//!
//! \file
//!

#include <age_types.hpp>

// we skip the following includes for doxygen output since they would bloat the include graphs
//! \cond

#include <cmath> // sin, cos
#include <atomic>
#include <condition_variable>
#include <sstream> // std::stringstream

//! \endcond



namespace age
{

typedef std::atomic_size_t  atomic_uint;
typedef std::atomic<uint64> atomic_uint64;
typedef std::atomic_bool    atomic_bool;

} // namespace age



#endif // AGE_UI_HPP
