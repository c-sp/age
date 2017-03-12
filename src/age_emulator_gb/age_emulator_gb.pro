
QT -= core gui

TARGET = age_emulator_gb
TEMPLATE = lib
CONFIG += staticlib

DEPENDENCIES = age_common
include($$PWD/../age.pri)



SOURCES += \
    age_gb_bus.cpp \
    age_gb_core.cpp \
    age_gb_cpu.cpp \
    age_gb_joypad.cpp \
    age_gb_lcd.cpp \
    age_gb_lcd_ly_lyc.cpp \
    age_gb_lcd_old_render.cpp \
    age_gb_lcd_ports.cpp \
    age_gb_lcd_ppu.cpp \
    age_gb_memory.cpp \
    age_gb_memory_init.cpp \
    age_gb_serial.cpp \
    age_gb_sound.cpp \
    age_gb_sound_channels.cpp \
    age_gb_sound_utilities.cpp \
    age_gb_timer.cpp \
    age_gb_emulator.cpp \
    age_gb_timer_utils.cpp

HEADERS += \
    age_gb.hpp \
    age_gb_bus.hpp \
    age_gb_core.hpp \
    age_gb_cpu.hpp \
    age_gb_joypad.hpp \
    age_gb_lcd.hpp \
    age_gb_memory.hpp \
    age_gb_serial.hpp \
    age_gb_sound.hpp \
    age_gb_sound_utilities.hpp \
    age_gb_timer.hpp \
    age_gb_emulator.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
