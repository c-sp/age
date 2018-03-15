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

#include <age_debug.hpp>

#include "age_ui_qt.hpp"

#if 1
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif





//---------------------------------------------------------
//
//   operators
//
//---------------------------------------------------------

std::ostream& operator<<(std::ostream &stream, const QString &string)
{
    stream << string.toStdString();
    return stream;
}

std::string operator+(const std::string &std_string, const QString &q_string)
{
    std::string result = std_string + q_string.toStdString();
    return result;
}



//---------------------------------------------------------
//
//   utility methods
//
//---------------------------------------------------------

bool age::is_checked(int checked_state)
{
    return checked_state == Qt::Checked;
}



//---------------------------------------------------------
//
//   audio utility methods
//
//---------------------------------------------------------

QString age::get_name_for_qt_downsampler_quality(qt_downsampler_quality quality)
{
    QString result;

    switch (quality)
    {
        case qt_downsampler_quality::low: result = qt_downsampler_quality_low; break;
        case qt_downsampler_quality::high: result = qt_downsampler_quality_high; break;
        case qt_downsampler_quality::highest: result = qt_downsampler_quality_highest; break;
    }

    return result;
}



age::qt_downsampler_quality age::get_qt_downsampler_quality_for_name(const QString &name)
{
    qt_downsampler_quality result;

    if (0 == name.compare(qt_downsampler_quality_low, Qt::CaseInsensitive))
    {
        result = qt_downsampler_quality::low;
    }
    else if (0 == name.compare(qt_downsampler_quality_highest, Qt::CaseInsensitive))
    {
        result = qt_downsampler_quality::highest;
    }
    else
    {
        result = qt_downsampler_quality::high; // default
    }

    return result;
}



//---------------------------------------------------------
//
//   renderer utility methods
//
//---------------------------------------------------------

GLint age::get_qt_filter_factor(qt_filter filter)
{
    GLint result;

    switch(filter)
    {
    case qt_filter::scale2x:
    case qt_filter::age_scale2x:
        result = 2;
        break;

    case qt_filter::gauss3x3:
    case qt_filter::gauss5x5:
    case qt_filter::emboss3x3:
    case qt_filter::emboss5x5:
        result = 1;
        break;

    default:
        result = 0;
        break;
    }

    return result;
}
