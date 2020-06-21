
QT -= core gui

TARGET = age_emulator_gb
TEMPLATE = lib
CONFIG += staticlib

DEPENDENCIES = age_common
include($$PWD/../age.pri)



SOURCES += \
    ../../../src/age_emulator_gb/age_gb_cpu_opcodes.cpp \
    ../../../src/age_emulator_gb/common/age_gb_clock.cpp \
    ../../../src/age_emulator_gb/age_gb_bus.cpp \
    ../../../src/age_emulator_gb/age_gb_cpu.cpp \
    ../../../src/age_emulator_gb/age_gb_emulator.cpp \
    ../../../src/age_emulator_gb/age_gb_emulator_impl.cpp \
    ../../../src/age_emulator_gb/age_gb_joypad.cpp \
    ../../../src/age_emulator_gb/age_gb_lcd.cpp \
    ../../../src/age_emulator_gb/age_gb_lcd_ly_lyc.cpp \
    ../../../src/age_emulator_gb/age_gb_lcd_old_render.cpp \
    ../../../src/age_emulator_gb/age_gb_lcd_ports.cpp \
    ../../../src/age_emulator_gb/age_gb_lcd_ppu.cpp \
    ../../../src/age_emulator_gb/age_gb_memory.cpp \
    ../../../src/age_emulator_gb/age_gb_memory_init.cpp \
    ../../../src/age_emulator_gb/age_gb_serial.cpp \
    ../../../src/age_emulator_gb/age_gb_sound.cpp \
    ../../../src/age_emulator_gb/age_gb_sound_io_ports.cpp \
    ../../../src/age_emulator_gb/age_gb_sound_utilities.cpp \
    ../../../src/age_emulator_gb/age_gb_timer.cpp \
    ../../../src/age_emulator_gb/age_gb_timer_utils.cpp \
    ../../../src/age_emulator_gb/common/age_gb_device.cpp \
    ../../../src/age_emulator_gb/common/age_gb_events.cpp \
    ../../../src/age_emulator_gb/common/age_gb_interrupts.cpp

HEADERS += \
    ../../../src/age_emulator_gb/common/age_gb_clock.hpp \
    ../../../src/age_emulator_gb/age_gb.hpp \
    ../../../src/age_emulator_gb/age_gb_bus.hpp \
    ../../../src/age_emulator_gb/age_gb_cpu.hpp \
    ../../../src/age_emulator_gb/age_gb_emulator_impl.hpp \
    ../../../src/age_emulator_gb/age_gb_joypad.hpp \
    ../../../src/age_emulator_gb/age_gb_lcd.hpp \
    ../../../src/age_emulator_gb/age_gb_memory.hpp \
    ../../../src/age_emulator_gb/age_gb_serial.hpp \
    ../../../src/age_emulator_gb/age_gb_sound.hpp \
    ../../../src/age_emulator_gb/age_gb_sound_utilities.hpp \
    ../../../src/age_emulator_gb/age_gb_timer.hpp \
    ../../../src/age_emulator_gb/common/age_gb_device.hpp \
    ../../../src/age_emulator_gb/common/age_gb_events.hpp \
    ../../../src/age_emulator_gb/common/age_gb_interrupts.hpp \
    ../../../src/include/emulator/age_gb_emulator.hpp \
    ../../../src/include/emulator/age_gb_types.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
