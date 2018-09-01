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

#include <QDir>
#include <QFileInfo>

#include "age_test_gb.hpp"



namespace
{

age::test_method blargg_test(const QString &test_file_name, QString &result_file_name, bool for_dmg)
{
    // blargg test results are always checked by screenshot comparison

    QString prefix = for_dmg ? "dmg_" : "cgb_";

    QFileInfo test_file_info(test_file_name);

    QString test_file_base_name = test_file_info.completeBaseName();
    QStringList result_file_filter(
                QString(test_file_base_name.startsWith(prefix, Qt::CaseInsensitive) ? "" : prefix)
                .append(test_file_base_name)
                .append("_?*s.png")
                );

    // we expect to find at most one result screenshot
    QFileInfoList file_infos = test_file_info.absoluteDir().entryInfoList(result_file_filter);
    if (file_infos.length() == 1)
    {
        QFileInfo result_file_info = file_infos[0];

        // parse the number of seconds to emulate
        QString base_name = result_file_info.completeBaseName();
        int last_underscore = base_name.lastIndexOf('_');
        QString seconds_string = base_name.mid(
                    last_underscore + 1,
                    base_name.length() - last_underscore - 2
                    );
        bool parsed = false;
        int seconds = seconds_string.toInt(&parsed);

        if (parsed)
        {
            seconds = qMax(1, seconds);
            result_file_name = result_file_info.absoluteFilePath();
            return age::screenshot_test_png(for_dmg, false, seconds * 1000);
        }
    }

    // result file not found or emulation duration not parseable
    return age::test_method{};
}

}



age::test_method age::blargg_dmg_test(const QString &test_file_name, QString &result_file_name)
{
    return blargg_test(test_file_name, result_file_name, true);
}

age::test_method age::blargg_cgb_test(const QString &test_file_name, QString &result_file_name)
{
    return blargg_test(test_file_name, result_file_name, false);
}
