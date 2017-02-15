//
// Copyright (c) 2010-2017 Christoph Sprenger
//
// This file is part of AGE ("Another Gameboy Emulator").
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

#ifndef AGE_TEST_HPP
#define AGE_TEST_HPP

//!
//! \file
//!

// we skip the following includes for doxygen output since they would bloat the include graphs
//! \cond

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QRunnable>
#include <QSet>
#include <QTextStream>
#include <QThreadPool>
#include <QTimer>

//! \endcond

#include <age_gb_simulator.hpp>



namespace age
{

class emulator_test : public QObject, public QRunnable
{
    Q_OBJECT
public:

    emulator_test(const QString &test_file);
    virtual ~emulator_test() = default;

    void run() override;

signals:

    void test_passed(QString test_file);
    void test_failed(QString test_file, QString reason);

protected:

    //virtual QString run_test(gb_simulator &emulator) = 0;

    const QString m_test_file;
};

} // namespace age



#endif // AGE_TEST_HPP
