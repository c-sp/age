
QT -= core gui

TARGET = age_common
TEMPLATE = lib
CONFIG += staticlib

include($$PWD/../age.pri)



SOURCES += \
    age_debug.cpp \
    age_downsampler.cpp \
    age_emulator.cpp \
    age_pcm_ring_buffer.cpp \
    age_screen_buffer.cpp \
    age_utilities.cpp

HEADERS += \
    ../include/age_debug.hpp \
    ../include/age_types.hpp \
    ../include/age_utilities.hpp \
    ../include/emulator/age_emulator.hpp \
    ../include/gfx/age_pixel.hpp \
    ../include/gfx/age_screen_buffer.hpp \
    ../include/pcm/age_downsampler.hpp \
    ../include/pcm/age_pcm_ring_buffer.hpp \
    ../include/pcm/age_pcm_sample.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
