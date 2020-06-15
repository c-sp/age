//
// Copyright 2019 Christoph Sprenger
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

#include <QFile>
#include <QTextStream>

#include "age_test_app.hpp"



QString get_category_line(age::test_type type)
{
    switch (type)
    {
        case age::test_type::blargg_test:
            return QString(age::blargg) + ":";

        case age::test_type::gambatte_test:
            return QString(age::gambatte) + ":";

        case age::test_type::mooneye_test:
            return QString(age::mooneye_gb) + ":";
    }
    return "";
}


bool age::remove_ignored_files(const QString &ignore_list_path,
                               age::test_type category,
                               QSet<QString> &files)
{
    // nothing to read?
    if (ignore_list_path.isEmpty())
    {
        return true;
    }

    // try to open the file for reading
    QFile file(ignore_list_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    qInfo("reading ignore list from file %s", qPrintable(file.fileName()));

    // parse ignore list
    QStringList ignore_list;
    QTextStream stream(&file);
    QString category_line = get_category_line(category);
    bool found_category = false;

    while (!stream.atEnd())
    {
        QString line = stream.readLine();

        // ignore empty lines and lines beginning with a '#'
        line = line.trimmed();
        if (line.isEmpty() || line.startsWith('#'))
        {
            continue;
        }

        // trim end-of-line comments
        auto idx = line.indexOf('#');
        if (idx >= 0)
        {
            line = line.left(idx).trimmed();
        }

        // is this a category?
        if (line.endsWith(":"))
        {
            found_category = category_line == line;
        }
        // not a category => possible ignore list entry
        else if (found_category)
        {
            ignore_list.append(line);
        }
    }

    int entries = ignore_list.size();
    qInfo("found %d ignore-%s for category \"%s\"", entries, ((entries == 1) ? "entry" : "entries"), qPrintable(category_line.chopped(1)));

    // remove ignored files
    QStringList ignored_files;
    QSet<QString> used_entries;

    if (!ignore_list.isEmpty())
    {
        QMutableSetIterator<QString> itr(files);
        while (itr.hasNext())
        {
            QString file = itr.next();
            // check each ignore list entry for this file
            for (QString to_ignore : ignore_list)
            {
                if (file.contains(to_ignore))
                {
                    itr.remove();
                    ignored_files.append(file);
                    used_entries.insert(to_ignore);
                    break;
                }
            }
        }
    }

    ignored_files.sort();
    print_list("ignoring the following file(s):", ignored_files);

    // find unused ignore entries
    QStringList unused;

    for (QString to_ignore : ignore_list)
    {
        if (!used_entries.contains(to_ignore))
        {
            unused.append(to_ignore);
        }
    }

    if (!unused.isEmpty())
    {
        unused.sort();
        print_list("unused ignore entries:", unused);
    }

    // done
    return true;
}
