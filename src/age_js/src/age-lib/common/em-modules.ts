//
// Copyright 2018 Christoph Sprenger
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// TODO lib
export interface IEmModule {

    HEAPU8: Uint8Array;
    HEAP16: Int16Array;
}

// TODO lib
export interface IEmGbModule extends IEmModule {

    _gb_allocate_rom_buffer(romSize: number): number;
    _gb_new_emulator(): void;

    _gb_get_persistent_ram(): number;
    _gb_get_persistent_ram_size(): number;
    _gb_set_persistent_ram(): void;

    _gb_emulate(minCyclesToEmulate: number, sampleRate: number): boolean;
    _gb_get_cycles_per_second(): number;
    _gb_get_emulated_cycles(): number;

    _gb_set_buttons_down(buttons: number): void;
    _gb_set_buttons_up(buttons: number): void;

    _gb_get_screen_width(): number;
    _gb_get_screen_height(): number;
    _gb_get_screen_front_buffer(): number;

    _gb_get_audio_buffer(): number;
    _gb_get_audio_buffer_size(): number;

    _gb_get_rom_name(): number;
}
