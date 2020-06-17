
# we need Qt GUI for QImage
QT += core gui

TARGET = age_qt_emu_test
TEMPLATE = app
CONFIG -= app_bundle

# The dependency order is important for the linker:
# in order to resolve all age_common symbols (including
# the ones used in age_emulator_gb),
# the age_common library must be linked last and thus
# be the last dependency.
DEPENDENCIES = age_emulator_gb age_common
include($$PWD/../age.pri)



SOURCES += \
    ../../../src/age_qt_emu_test/age_test_app.cpp \
    ../../../src/age_qt_emu_test/age_test_app_ignore.cpp \
    ../../../src/age_qt_emu_test/age_test_app_utils.cpp \
    ../../../src/age_qt_emu_test/age_test_gb.cpp \
    ../../../src/age_qt_emu_test/age_test_gb_blargg.cpp \
    ../../../src/age_qt_emu_test/age_test_gb_gambatte.cpp \
    ../../../src/age_qt_emu_test/age_test_gb_mooneye.cpp \
    ../../../src/age_qt_emu_test/age_test_main.cpp

HEADERS += \
    ../../../src/age_qt_emu_test/age_test.hpp \
    ../../../src/age_qt_emu_test/age_test_app.hpp \
    ../../../src/age_qt_emu_test/age_test_gb.hpp
