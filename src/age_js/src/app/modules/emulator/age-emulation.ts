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

import {AgeGbButton} from './age-emulator-keymap';
import {AgeRect, EmGbModule} from '../../common';


export class AgeScreenBuffer {

    constructor(readonly buffer: ArrayBufferView,
                readonly offset: number) {
    }
}


export interface AgeEmulation {

    getCyclesPerSecond(): number;

    getScreenSize(): AgeRect;

    getScreenBuffer(): AgeScreenBuffer;

    emulateCycles(cyclesToEmulate: number, sampleRate: number): boolean;

    buttonDown(button: number): void;

    buttonUp(button: number): void;

    getAudioBuffer(): Int16Array;
}


export class AgeGbEmulation implements AgeEmulation {

    constructor(private readonly _emGbModule: EmGbModule,
                romFileContents: ArrayBuffer) {

        const romArray = new Uint8Array(romFileContents, 0, romFileContents.byteLength);
        const bufferPtr = this._emGbModule._gb_allocate_rom_buffer(romArray.length);
        this._emGbModule.HEAPU8.set(romArray, bufferPtr);
        this._emGbModule._gb_new_emulator();
    }

    getCyclesPerSecond(): number {
        return this._emGbModule._gb_get_cycles_per_second();
    }

    getScreenSize(): AgeRect {
        return new AgeRect(
            this._emGbModule._gb_get_screen_width(),
            this._emGbModule._gb_get_screen_height()
        );
    }

    getScreenBuffer(): AgeScreenBuffer {
        const screenBuffer = this._emGbModule._gb_get_screen_front_buffer();
        return new AgeScreenBuffer(this._emGbModule.HEAPU8, screenBuffer);
    }

    emulateCycles(cyclesToEmulate: number, sampleRate: number): boolean {
        return this._emGbModule._gb_emulate(cyclesToEmulate, sampleRate);
    }

    buttonDown(button: AgeGbButton): void {
        this._emGbModule._gb_set_buttons_down(button);
    }

    buttonUp(button: AgeGbButton): void {
        this._emGbModule._gb_set_buttons_up(button);
    }

    getAudioBuffer(): Int16Array {
        const bufferOffsetBytes = this._emGbModule._gb_get_audio_buffer();
        const bufferSizeBytes = this._emGbModule._gb_get_audio_buffer_size() * 4;
        const bufferEndBytes = bufferOffsetBytes + bufferSizeBytes;
        return this._emGbModule.HEAP16.slice(bufferOffsetBytes / 2, bufferEndBytes / 2);
    }
}


export class AgeEmulationRunner {

    readonly screenSize: AgeRect;

    private _lastEmuTime: number;
    private _screenBuffer: AgeScreenBuffer;
    private _audioBuffer: Int16Array;

    constructor(private readonly _emulation: AgeEmulation) {
        this.screenSize = this._emulation.getScreenSize();
        this._lastEmuTime = Date.now();
        this._screenBuffer = this._emulation.getScreenBuffer();
        this._audioBuffer = this._emulation.getAudioBuffer();
    }

    get screenBuffer(): AgeScreenBuffer {
        return this._screenBuffer;
    }

    get audioBuffer(): Int16Array {
        return this._audioBuffer;
    }

    /**
     * Run the emulation for a number of emulated cycles equivalent to the time between the last call to
     * {@link emulate()} and the current time.
     *
     * @returns true if the emulated screen has changed, false otherwise
     */
    emulate(sampleRate: number): boolean {
        const now = Date.now();
        const millisElapsed = now - this._lastEmuTime;
        this._lastEmuTime = now;

        // calculate the number of cycles to emulate based on the elapsed time
        // (limit cycles to not freeze the browser in case of performance issues)
        const cyclesPerSecond = this._emulation.getCyclesPerSecond();
        const maxCyclesToEmulate = cyclesPerSecond / 20;
        const cyclesToEmulate = Math.min(maxCyclesToEmulate, cyclesPerSecond * millisElapsed / 1000);

        // emulate
        const newFrame = this._emulation.emulateCycles(cyclesToEmulate, sampleRate);
        this._audioBuffer = this._emulation.getAudioBuffer();
        if (newFrame) {
            this._screenBuffer = this._emulation.getScreenBuffer();
        }
        return newFrame;
    }

    buttonDown(button: number): void {
        this._emulation.buttonDown(button);
    }

    buttonUp(button: number): void {
        this._emulation.buttonUp(button);
    }
}
