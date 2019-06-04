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

import {AgeRect, IAgeEmulationRuntimeInfo, IEmGbModule} from "../common";
import {AgeGbButton} from "./age-emulator-keymap";


export class AgeScreenBuffer {

    constructor(readonly buffer: ArrayBufferView,
                readonly offset: number) {
    }
}


interface IAgeEmulation {

    getCyclesPerSecond(): number;

    getEmulatedCycles(): number;

    getRomName(): string;

    getScreenSize(): AgeRect;

    getScreenBuffer(): AgeScreenBuffer;

    emulateCycles(cyclesToEmulate: number, sampleRate: number): boolean;

    buttonDown(button: number): void;

    buttonUp(button: number): void;

    getAudioBuffer(): Int16Array;
}


export class AgeGbEmulation implements IAgeEmulation {

    constructor(private readonly _emGbModule: IEmGbModule,
                romFileContents: ArrayBuffer) {

        const romArray = new Uint8Array(romFileContents, 0, romFileContents.byteLength);
        const bufferPtr = this._emGbModule._gb_allocate_rom_buffer(romArray.length);
        this._emGbModule.HEAPU8.set(romArray, bufferPtr);
        this._emGbModule._gb_new_emulator();
    }

    getCyclesPerSecond(): number {
        return this._emGbModule._gb_get_cycles_per_second();
    }

    getEmulatedCycles(): number {
        return this._emGbModule._gb_get_emulated_cycles();
    }

    getRomName(): string {
        let romName = "";

        for (let i = this._emGbModule._gb_get_rom_name(), end = i + 32; i < end; ++i) {
            // the rom name is null terminated and made of ascii chars
            const code = this._emGbModule.HEAPU8[i];
            if (!code) {
                break;
            }
            romName += String.fromCharCode(code);
        }

        // some rom names seem to be padded with underscores: trim them
        while (romName.endsWith("_")) {
            romName = romName.substr(0, romName.length - 1);
        }

        return romName;
    }

    getScreenSize(): AgeRect {
        return new AgeRect(
            this._emGbModule._gb_get_screen_width(),
            this._emGbModule._gb_get_screen_height(),
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
    private _runtimeInfoGenerator: AgeEmulationRuntimeInfoGenerator;

    constructor(private readonly _emulation: IAgeEmulation) {
        this.screenSize = this._emulation.getScreenSize();
        this._lastEmuTime = performance.now();
        this._screenBuffer = this._emulation.getScreenBuffer();
        this._audioBuffer = this._emulation.getAudioBuffer();
        this._runtimeInfoGenerator = new AgeEmulationRuntimeInfoGenerator(this._emulation);
    }

    get screenBuffer(): AgeScreenBuffer {
        return this._screenBuffer;
    }

    get audioBuffer(): Int16Array {
        return this._audioBuffer;
    }

    get runtimeInfo(): IAgeEmulationRuntimeInfo {
        return this._runtimeInfoGenerator.runtimeInfo;
    }

    /**
     * Run the emulation for a number of emulated cycles equivalent to the time between the last call to
     * {@link emulate()} and the current time.
     *
     * @returns true if the emulated screen has changed, false otherwise
     */
    emulate(sampleRate: number): boolean {
        const now = performance.now();
        const millisElapsed = now - this._lastEmuTime;
        this._lastEmuTime = now;

        // calculate the number of cycles to emulate based on the elapsed time
        // (limit cycles to not freeze the browser in case of performance issues)
        const cyclesPerSecond = this._emulation.getCyclesPerSecond();
        const maxCyclesToEmulate = cyclesPerSecond / 20;
        const cyclesToEmulate = Math.min(maxCyclesToEmulate, cyclesPerSecond * millisElapsed / 1000);

        // emulate
        const pNow = performance.now();
        const newFrame = this._emulation.emulateCycles(cyclesToEmulate, sampleRate);
        const emuMillis = performance.now() - pNow;

        // update buffers and runtime information
        this._audioBuffer = this._emulation.getAudioBuffer();
        if (newFrame) {
            this._screenBuffer = this._emulation.getScreenBuffer();
        }
        this._runtimeInfoGenerator.checkForUpdate(now, emuMillis);

        return newFrame;
    }

    buttonDown(button: number): void {
        this._emulation.buttonDown(button);
    }

    buttonUp(button: number): void {
        this._emulation.buttonUp(button);
    }
}


class AgeEmulationRuntimeInfoGenerator {

    private _runtimeInfo: IAgeEmulationRuntimeInfo;
    private _lastRuntimeInfoTime = performance.now();
    private _lastEmulatedCycles = 0;
    private _emuMillis = 0;

    constructor(private readonly _emulation: IAgeEmulation) {
        this._runtimeInfo = {
            romName: this._emulation.getRomName(),
        };
    }

    get runtimeInfo(): IAgeEmulationRuntimeInfo {
        return this._runtimeInfo;
    }

    checkForUpdate(now: number, emuMillis: number): void {
        this._emuMillis += emuMillis;
        const elapsedMillis = now - this._lastRuntimeInfoTime;

        if (elapsedMillis > 500) {
            const cyclesPerSecond = this._emulation.getCyclesPerSecond();
            const emulatedCycles = this._emulation.getEmulatedCycles();
            const elapsedCycles = emulatedCycles - this._lastEmulatedCycles;

            this._runtimeInfo = {
                romName: this._runtimeInfo.romName,
                emulatedSeconds: emulatedCycles / cyclesPerSecond,
                emulationSpeed: elapsedCycles * 1000 / elapsedMillis / cyclesPerSecond,
                emulationMaxSpeed: elapsedCycles * 1000 / Math.max(1, this._emuMillis) / cyclesPerSecond,
            };

            this._lastRuntimeInfoTime = now;
            this._lastEmulatedCycles = emulatedCycles;
            this._emuMillis = 0;
        }
    }
}
