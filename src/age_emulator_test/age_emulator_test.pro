
QT += core
QT -= gui

TARGET = age_emulator_test
TEMPLATE = app
CONFIG -= app_bundle

DEPENDENCIES = age_simulator_gb age_common
include($$PWD/../age.pri)



SOURCES += \
    age_test_main.cpp \
    age_test.cpp \
    age_test_application.cpp \
    age_test_mooneye.cpp

HEADERS += \
    age_test.hpp \
    age_test_application.hpp \
    age_test_mooneye.hpp
