
QT -= core gui

TARGET = age_ui_common
TEMPLATE = lib
CONFIG += staticlib

DEPENDENCIES = age_common
include($$PWD/../age.pri)



SOURCES += \
    age_ui_downsampler.cpp \
    age_ui_pcm_ring_buffer.cpp

HEADERS += \
    age_ui_downsampler.hpp \
    age_ui_pcm_ring_buffer.hpp \
    age_ui_speed_calculator.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
