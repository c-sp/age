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

#include <QDir>
#include <QFileInfo>

#include "age_test_gb.hpp"



namespace
{

int test_seconds(const QString &test_result)
{
    // see https://github.com/c-sp/gameboy-test-roms
    if (test_result == "cgb_cpu_instrs")
    {
        return 31;
    }
    if (test_result == "dmg_cpu_instrs")
    {
        return 55;
    }
    if (test_result == "cgb_instr_timing")
    {
        return 1;
    }
    if (test_result == "dmg_instr_timing")
    {
        return 1;
    }
    if (test_result == "cgb_mem_timing")
    {
        return 4;
    }
    if (test_result == "dmg_mem_timing")
    {
        return 4;
    }
    if (test_result == "cgb_sound")
    {
        return 37;
    }
    if (test_result == "dmg_sound")
    {
        return 36;
    }
    return 0;
}

age::test_method blargg_test(const QString &test_file_name, QString &result_file_name, bool for_dmg)
{
    // blargg test results are always checked by screenshot comparison

    QString prefix = for_dmg ? "dmg_" : "cgb_";

    QFileInfo test_file_info(test_file_name);

    QString test_file_base_name = test_file_info.completeBaseName();
    QString result_png_name = QString(test_file_base_name.startsWith(prefix, Qt::CaseInsensitive) ? "" : prefix)
            .append(test_file_base_name)
            .append(".png");
    QStringList result_file_filter(result_png_name);

    // we expect to find at most one result screenshot
    QFileInfoList file_infos = test_file_info.absoluteDir().entryInfoList(result_file_filter);
    if (file_infos.length() == 1)
    {
        QFileInfo result_file_info = file_infos[0];
        int seconds = test_seconds(result_file_info.completeBaseName());
        if (seconds > 0)
        {
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
