
QT += core
QT -= gui

TARGET = age_simulator_test
TEMPLATE = app
CONFIG -= app_bundle

DEPENDENCIES = age_simulator_gb age_common
include($$PWD/../age.pri)



SOURCES += \
    age_test_main.cpp \
    age_test_runner.cpp \
    age_test.cpp

HEADERS += \
    age_test.hpp \
    age_test_runner.hpp
