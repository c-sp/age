
QT -= core gui

TARGET = age_emulator_gb
TEMPLATE = lib
CONFIG += staticlib

DEPENDENCIES = age_common
include($$PWD/../age.pri)



SOURCES += \
    age_gb_cpu_opcodes.cpp \
    age_gb_div.cpp \
    age_gb_lcd.cpp \
    age_gb_lcd_irqs.cpp \
    age_gb_lcd_palettes.cpp \
    age_gb_lcd_ports.cpp \
    age_gb_lcd_render.cpp \
    age_gb_lcd_scanline.cpp \
    age_gb_lcd_sprites.cpp \
    age_gb_timer.cpp \
    age_gb_timer_state.cpp \
    common/age_gb_clock.cpp \
    age_gb_bus.cpp \
    age_gb_cpu.cpp \
    age_gb_emulator.cpp \
    age_gb_emulator_impl.cpp \
    age_gb_joypad.cpp \
    age_gb_memory.cpp \
    age_gb_memory_init.cpp \
    age_gb_serial.cpp \
    age_gb_sound.cpp \
    age_gb_sound_io_ports.cpp \
    age_gb_sound_utilities.cpp \
    common/age_gb_device.cpp \
    common/age_gb_events.cpp \
    common/age_gb_interrupts.cpp

HEADERS += \
    age_gb_div.hpp \
    age_gb_lcd.hpp \
    age_gb_lcd_render.hpp \
    age_gb_timer.hpp \
    common/age_gb_clock.hpp \
    age_gb.hpp \
    age_gb_bus.hpp \
    age_gb_cpu.hpp \
    age_gb_emulator_impl.hpp \
    age_gb_joypad.hpp \
    age_gb_memory.hpp \
    age_gb_serial.hpp \
    age_gb_sound.hpp \
    age_gb_sound_utilities.hpp \
    common/age_gb_device.hpp \
    common/age_gb_events.hpp \
    common/age_gb_interrupts.hpp \
    ../include/emulator/age_gb_emulator.hpp \
    ../include/emulator/age_gb_types.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
