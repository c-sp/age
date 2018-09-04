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

#include <age_debug.hpp>

#include "age_ui_qt.hpp"





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
