
QT -= core gui

TARGET = age_common
TEMPLATE = lib
CONFIG += staticlib

include($$PWD/../age.pri)



SOURCES += \
    age.cpp

HEADERS += \
    age.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
