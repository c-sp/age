TARGET = age
TEMPLATE = app
QT += core gui widgets opengl multimedia



# enable c14 features
CONFIG -= c++11
CONFIG *= c++17

# define DEBUG (or maybe don't)
CONFIG(debug, debug|release) {
    DEFINES *= DEBUG
} else {
    DEFINES -= DEBUG
}

# configure GCC optimizations
# (details at https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html)
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE *= -O3
QMAKE_LFLAGS_RELEASE *= -O3

# enable link time optimization only for applications (not for libraries)
equals(TEMPLATE, app) {
    QMAKE_CXXFLAGS_RELEASE *= -flto
    QMAKE_LFLAGS_RELEASE *= -flto
}

# add shared headers to include path
INCLUDEPATH += $$PWD/age_common/api
INCLUDEPATH += $$PWD/age_emulator_gb/api



HEADERS += \
    age_common/api/age_debug.hpp \
    age_common/api/age_types.hpp \
    age_common/api/age_utilities.hpp \
    age_common/api/gfx/age_pixel.hpp \
    age_common/api/gfx/age_screen_buffer.hpp \
    age_common/api/pcm/age_downsampler.hpp \
    age_common/api/pcm/age_pcm_frame.hpp \
    age_common/api/pcm/age_pcm_ring_buffer.hpp \
    age_emulator_gb/age_gb_bus.hpp \
    age_emulator_gb/age_gb_cpu.hpp \
    age_emulator_gb/age_gb_emulator_impl.hpp \
    age_emulator_gb/age_gb_joypad.hpp \
    age_emulator_gb/lcd/common/age_gb_lcd_common.hpp \
    age_emulator_gb/lcd/palettes/age_gb_lcd_palettes.hpp \
    age_emulator_gb/lcd/render/age_gb_lcd_fifo_fetcher.hpp \
    age_emulator_gb/lcd/render/age_gb_lcd_fifo_renderer.hpp \
    age_emulator_gb/lcd/render/age_gb_lcd_line_renderer.hpp \
    age_emulator_gb/lcd/render/age_gb_lcd_renderer.hpp \
    age_emulator_gb/lcd/render/age_gb_lcd_renderer_common.hpp \
    age_emulator_gb/lcd/render/age_gb_lcd_sprites.hpp \
    age_emulator_gb/lcd/render/age_gb_lcd_window_check.hpp \
    age_emulator_gb/lcd/age_gb_lcd.hpp \
    age_emulator_gb/age_gb_memory.hpp \
    age_emulator_gb/age_gb_oam_dma.hpp \
    age_emulator_gb/age_gb_serial.hpp \
    age_emulator_gb/age_gb_timer.hpp \
    age_emulator_gb/api/emulator/age_gb_emulator.hpp \
    age_emulator_gb/api/emulator/age_gb_types.hpp \
    age_emulator_gb/common/age_gb_clock.hpp \
    age_emulator_gb/common/age_gb_device.hpp \
    age_emulator_gb/common/age_gb_events.hpp \
    age_emulator_gb/common/age_gb_interrupts.hpp \
    age_emulator_gb/sound/age_gb_sound.hpp \
    age_emulator_gb/sound/age_gb_sound_channel.hpp \
    age_emulator_gb/sound/age_gb_sound_generate.hpp \
    age_emulator_gb/sound/age_gb_sound_generate_duty.hpp \
    age_emulator_gb/sound/age_gb_sound_generate_noise.hpp \
    age_emulator_gb/sound/age_gb_sound_generate_wave.hpp \
    age_emulator_gb/sound/age_gb_sound_length_counter.hpp \
    age_emulator_gb/sound/age_gb_sound_sweep.hpp \
    age_emulator_gb/sound/age_gb_sound_volume.hpp \
    age_qt_gui/age_ui_qt.hpp \
    age_qt_gui/age_ui_qt_audio.hpp \
    age_qt_gui/age_ui_qt_emulation_runner.hpp \
    age_qt_gui/age_ui_qt_emulator.hpp \
    age_qt_gui/age_ui_qt_main_window.hpp \
    age_qt_gui/age_ui_qt_settings.hpp \
    age_qt_gui/age_ui_qt_user_value_store.hpp \
    age_qt_gui/age_ui_qt_video.hpp

SOURCES += \
    age_common/age_debug.cpp \
    age_common/age_downsampler.cpp \
    age_common/age_pcm_ring_buffer.cpp \
    age_common/age_screen_buffer.cpp \
    age_common/age_utilities.cpp \
    age_emulator_gb/age_gb_bus.cpp \
    age_emulator_gb/age_gb_cpu.cpp \
    age_emulator_gb/age_gb_cpu_opcodes.cpp \
    age_emulator_gb/age_gb_emulator.cpp \
    age_emulator_gb/age_gb_emulator_impl.cpp \
    age_emulator_gb/age_gb_joypad.cpp \
    age_emulator_gb/lcd/palettes/age_gb_lcd_palettes.cpp \
    age_emulator_gb/lcd/palettes/age_gb_lcd_palettes_cgb.cpp \
    age_emulator_gb/lcd/palettes/age_gb_lcd_palettes_compat.cpp \
    age_emulator_gb/lcd/render/age_gb_lcd_fifo_renderer.cpp \
    age_emulator_gb/lcd/render/age_gb_lcd_line_renderer.cpp \
    age_emulator_gb/lcd/render/age_gb_lcd_renderer.cpp \
    age_emulator_gb/lcd/render/age_gb_lcd_sprites.cpp \
    age_emulator_gb/lcd/age_gb_lcd.cpp \
    age_emulator_gb/lcd/age_gb_lcd_irqs.cpp \
    age_emulator_gb/lcd/age_gb_lcd_line.cpp \
    age_emulator_gb/lcd/age_gb_lcd_oam_access.cpp \
    age_emulator_gb/lcd/age_gb_lcd_registers.cpp \
    age_emulator_gb/age_gb_memory.cpp \
    age_emulator_gb/age_gb_memory_init.cpp \
    age_emulator_gb/age_gb_oam_dma.cpp \
    age_emulator_gb/age_gb_timer.cpp \
    age_emulator_gb/age_gb_timer_state.cpp \
    age_emulator_gb/age_gb_serial.cpp \
    age_emulator_gb/common/age_gb_clock.cpp \
    age_emulator_gb/common/age_gb_device.cpp \
    age_emulator_gb/common/age_gb_events.cpp \
    age_emulator_gb/common/age_gb_interrupts.cpp \
    age_emulator_gb/sound/age_gb_sound.cpp \
    age_emulator_gb/sound/age_gb_sound_registers.cpp \
    age_qt_gui/age_ui_qt.cpp \
    age_qt_gui/age_ui_qt_audio.cpp \
    age_qt_gui/age_ui_qt_emulation_runner.cpp \
    age_qt_gui/age_ui_qt_emulator.cpp \
    age_qt_gui/age_ui_qt_main.cpp \
    age_qt_gui/age_ui_qt_main_window.cpp \
    age_qt_gui/age_ui_qt_settings_audio.cpp \
    age_qt_gui/age_ui_qt_settings_dialog.cpp \
    age_qt_gui/age_ui_qt_settings_keys.cpp \
    age_qt_gui/age_ui_qt_settings_miscellaneous.cpp \
    age_qt_gui/age_ui_qt_settings_video.cpp \
    age_qt_gui/age_ui_qt_user_value_store.cpp \
    age_qt_gui/age_ui_qt_video.cpp \
    age_qt_gui/age_ui_qt_video_post_processor.cpp \
    age_qt_gui/age_ui_qt_video_renderer.cpp

RESOURCES += \
    age_qt_gui/shaders/age_qt_gui_shaders.qrc
