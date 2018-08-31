
QT -= core gui

TARGET = age_common
TEMPLATE = lib
CONFIG += staticlib

include($$PWD/../age.pri)



SOURCES += \
    ../../../src/age_common/age_debug.cpp \
    ../../../src/age_common/age_downsampler.cpp \
    ../../../src/age_common/age_emulator.cpp \
    ../../../src/age_common/age_pcm_ring_buffer.cpp \
    ../../../src/age_common/age_screen_buffer.cpp

HEADERS += \
    ../../../src/include/age_debug.hpp \
    ../../../src/include/age_speed_calculator.hpp \
    ../../../src/include/age_types.hpp \
    ../../../src/include/emulator/age_emulator.hpp \
    ../../../src/include/gfx/age_pixel.hpp \
    ../../../src/include/gfx/age_screen_buffer.hpp \
    ../../../src/include/pcm/age_downsampler.hpp \
    ../../../src/include/pcm/age_pcm_ring_buffer.hpp \
    ../../../src/include/pcm/age_pcm_sample.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
