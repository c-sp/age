
QT -= core gui

TARGET = age_common
TEMPLATE = lib
CONFIG += staticlib

include($$PWD/../age.pri)



SOURCES += \
    age_graphics.cpp \
    age_debug.cpp \
    age_emulator.cpp

HEADERS += \
    age_graphics.hpp \
    age_types.hpp \
    age_debug.hpp \
    age_non_copyable.hpp \
    age_audio.hpp \
    age_emulator.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
