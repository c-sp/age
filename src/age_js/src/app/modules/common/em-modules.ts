interface EmModule {

    HEAPU8: Uint8Array;
}

interface EmGbModule extends EmModule {

    _gb_allocate_rom_buffer(romSize: number): number;
    _gb_new_emulator(): void;

    _gb_get_persistent_ram(): number;
    _gb_get_persistent_ram_size(): number;
    _gb_set_persistent_ram(): void;

    _gb_emulate(minCyclesToEmulate: number): boolean;
    _gb_get_cycles_per_second(): number;
    _gb_get_emulated_cycles(): number;

    _gb_set_buttons_down(buttons: number): void;
    _gb_set_buttons_up(buttons: number): void;

    _gb_get_screen_width(): number;
    _gb_get_screen_height(): number;
    _gb_get_screen_front_buffer(): number;

    _gb_get_audio_buffer(): number;
    _gb_get_pcm_sampling_rate(): number;
}
