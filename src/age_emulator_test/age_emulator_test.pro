
# we need Qt GUI for QImage
QT += core gui

TARGET = age_emulator_test
TEMPLATE = app
CONFIG -= app_bundle

DEPENDENCIES = age_emulator_gb age_common
include($$PWD/../age.pri)



SOURCES += \
    age_test_main.cpp \
    age_test_gb.cpp \
    age_test_gb_gambatte.cpp \
    age_test_app.cpp \
    age_test_app_utils.cpp

HEADERS += \
    age_test.hpp \
    age_test_gb.hpp \
    age_test_app.hpp
