add_library(
        age_emulator_gb
        STATIC
        common/age_gb_clock.cpp
        common/age_gb_device.cpp
        common/age_gb_events.cpp
        common/age_gb_interrupts.cpp
        sound/age_gb_sound.cpp
        sound/age_gb_sound_io_ports.cpp
        age_gb_bus.cpp
        age_gb_cpu.cpp
        age_gb_cpu_opcodes.cpp
        age_gb_emulator.cpp
        age_gb_emulator_impl.cpp
        age_gb_joypad.cpp
        age_gb_lcd.cpp
        age_gb_lcd_irqs.cpp
        age_gb_lcd_palettes.cpp
        age_gb_lcd_ports.cpp
        age_gb_lcd_render.cpp
        age_gb_lcd_scanline.cpp
        age_gb_lcd_sprites.cpp
        age_gb_memory.cpp
        age_gb_memory_init.cpp
        age_gb_serial.cpp
        age_gb_timer.cpp
        age_gb_timer_state.cpp
)

target_link_libraries(age_emulator_gb age_common)

target_include_directories(age_emulator_gb PUBLIC api)

# uncomment this to generate assembler listings with g++
#set_target_properties(age_emulator_gb PROPERTIES COMPILE_FLAGS "-save-temps -fverbose-asm -Wa,-adhlmn=main.lst")
