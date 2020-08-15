
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
    age_test_app.cpp \
    age_test_app_ignore.cpp \
    age_test_app_utils.cpp \
    age_test_gb.cpp \
    age_test_gb_blargg.cpp \
    age_test_gb_gambatte.cpp \
    age_test_gb_mooneye.cpp \
    age_test_main.cpp

HEADERS += \
    age_test.hpp \
    age_test_app.hpp \
    age_test_gb.hpp
