//
// Copyright 2019 Christoph Sprenger
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

import {AgeGbButton} from '../settings';
import {AgeScreenBuffer, AgeScreenSize, IAgeEmulatorInstance} from './age-emulation';
import {IAgeGbEmulatorModule} from './age-wasm-module';


export class AgeGbEmulatorInstance implements IAgeEmulatorInstance {

    constructor(private readonly _gbEmulator: IAgeGbEmulatorModule,
                romFileContents: ArrayBuffer) {

        const romArray = new Uint8Array(romFileContents, 0, romFileContents.byteLength);
        const bufferPtr = this._gbEmulator._gb_allocate_rom_buffer(romArray.length);
        this._gbEmulator.HEAPU8.set(romArray, bufferPtr);
        this._gbEmulator._gb_new_emulator();
    }

    getCyclesPerSecond(): number {
        return this._gbEmulator._gb_get_cycles_per_second();
    }

    getEmulatedCycles(): number {
        return this._gbEmulator._gb_get_emulated_cycles();
    }

    getRomName(): string {
        let romName = '';

        for (let i = this._gbEmulator._gb_get_rom_name(), end = i + 32; i < end; ++i) {
            // the rom name is null terminated and made of ascii chars
            const code = this._gbEmulator.HEAPU8[i];
            if (!code) {
                break;
            }
            romName += String.fromCharCode(code);
        }

        // some rom names seem to be padded with underscores: trim them
        while (romName.endsWith('_')) {
            romName = romName.substr(0, romName.length - 1);
        }

        return romName;
    }

    getScreenSize(): AgeScreenSize {
        return new AgeScreenSize(
            this._gbEmulator._gb_get_screen_width(),
            this._gbEmulator._gb_get_screen_height(),
        );
    }

    getScreenBuffer(): AgeScreenBuffer {
        const screenBuffer = this._gbEmulator._gb_get_screen_front_buffer();
        return new AgeScreenBuffer(this._gbEmulator.HEAPU8, screenBuffer);
    }

    emulateCycles(cyclesToEmulate: number, sampleRate: number): boolean {
        return this._gbEmulator._gb_emulate(cyclesToEmulate, sampleRate);
    }

    buttonDown(button: AgeGbButton): void {
        this._gbEmulator._gb_set_buttons_down(button);
    }

    buttonUp(button: AgeGbButton): void {
        this._gbEmulator._gb_set_buttons_up(button);
    }

    getAudioBuffer(): Int16Array {
        const bufferOffsetBytes = this._gbEmulator._gb_get_audio_buffer();
        const bufferSizeBytes = this._gbEmulator._gb_get_audio_buffer_size() * 4;
        const bufferEndBytes = bufferOffsetBytes + bufferSizeBytes;
        return this._gbEmulator.HEAP16.slice(bufferOffsetBytes / 2, bufferEndBytes / 2);
    }
}
