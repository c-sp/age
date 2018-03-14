
QT -= core gui

TARGET = age_common
TEMPLATE = lib
CONFIG += staticlib

include($$PWD/../age.pri)



SOURCES += \
    age_debug.cpp \
    age_emulator.cpp \
    age_screen_buffer.cpp

HEADERS += \
    age_types.hpp \
    age_debug.hpp \
    age_non_copyable.hpp \
    age_emulator.hpp \
    age_pcm_sample.hpp \
    age_screen_buffer.hpp \
    age_pixel.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
