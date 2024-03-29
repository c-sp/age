add_library(
        age_emulator_gb
        STATIC
        common/age_gb_clock.cpp
        common/age_gb_device.cpp
        common/age_gb_events.cpp
        common/age_gb_interrupts.cpp
        sound/age_gb_sound.cpp
        sound/age_gb_sound_registers.cpp
        age_gb_bus.cpp
        age_gb_cpu.cpp
        age_gb_cpu_opcodes.cpp
        age_gb_emulator.cpp
        age_gb_emulator_impl.cpp
        age_gb_joypad.cpp
        lcd/palettes/age_gb_lcd_palettes.cpp
        lcd/palettes/age_gb_lcd_palettes_cgb.cpp
        lcd/palettes/age_gb_lcd_palettes_compat.cpp
        lcd/render/age_gb_lcd_fifo_renderer.cpp
        lcd/render/age_gb_lcd_line_renderer.cpp
        lcd/render/age_gb_lcd_renderer.cpp
        lcd/age_gb_lcd.cpp
        lcd/age_gb_lcd_irqs.cpp
        lcd/age_gb_lcd_line.cpp
        lcd/age_gb_lcd_oam_access.cpp
        lcd/age_gb_lcd_registers.cpp
        memory/age_gb_mbc1.cpp
        memory/age_gb_mbc2.cpp
        memory/age_gb_mbc3.cpp
        memory/age_gb_mbc3_rtc.cpp
        memory/age_gb_mbc5.cpp
        memory/age_gb_mbc7.cpp
        memory/age_gb_memory.cpp
        memory/age_gb_memory_init.cpp
        age_gb_oam_dma.cpp
        age_gb_serial.cpp
        age_gb_timer.cpp
        age_gb_timer_state.cpp
)

target_link_libraries(age_emulator_gb age_common)

target_include_directories(age_emulator_gb PUBLIC api)

# uncomment this to generate assembler listings with g++ (interleaved)
#set_target_properties(age_emulator_gb PROPERTIES COMPILE_FLAGS "-save-temps -fverbose-asm -Wa,-adhlmn=main.lst")

# uncomment this to generate assembler listings with clang
#set_target_properties(age_emulator_gb PROPERTIES COMPILE_FLAGS "--save-temps")
