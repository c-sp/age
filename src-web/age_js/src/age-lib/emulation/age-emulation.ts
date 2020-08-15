//
// Copyright 2020 Christoph Sprenger
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


export class AgeScreenBuffer {

    constructor(readonly buffer: ArrayBufferView,
                readonly offset: number) {
    }
}

export class AgeScreenSize {

    constructor(readonly width: number,
                readonly height: number) {
    }
}

export interface IAgeEmulatorInstance {

    getCyclesPerSecond(): number;

    getEmulatedCycles(): number;

    getRomName(): string;

    getScreenSize(): AgeScreenSize;

    getScreenBuffer(): AgeScreenBuffer;

    emulateCycles(cyclesToEmulate: number, sampleRate: number): boolean;

    buttonDown(button: number): void;

    buttonUp(button: number): void;

    getAudioBuffer(): Int16Array;
}


export class AgeEmulation {

    readonly screenSize: AgeScreenSize;

    private _lastEmuTime: number;
    private _screenBuffer: AgeScreenBuffer;
    private _audioBuffer: Int16Array;
    private _infoGenerator: AgeEmulationInfoGenerator;

    constructor(private readonly _emulation: IAgeEmulatorInstance) {
        this.screenSize = this._emulation.getScreenSize();
        this._lastEmuTime = performance.now();
        this._screenBuffer = this._emulation.getScreenBuffer();
        this._audioBuffer = this._emulation.getAudioBuffer();
        this._infoGenerator = new AgeEmulationInfoGenerator(this._emulation);
    }

    get screenBuffer(): AgeScreenBuffer {
        return this._screenBuffer;
    }

    get audioBuffer(): Int16Array {
        return this._audioBuffer;
    }

    get info(): IAgeEmulationInfo {
        return this._infoGenerator.emulationInfo;
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

        // update buffers and emulation info
        this._audioBuffer = this._emulation.getAudioBuffer();
        if (newFrame) {
            this._screenBuffer = this._emulation.getScreenBuffer();
        }
        this._infoGenerator.checkForUpdate(now, emuMillis);

        return newFrame;
    }

    buttonDown(button: number): void {
        this._emulation.buttonDown(button);
    }

    buttonUp(button: number): void {
        this._emulation.buttonUp(button);
    }
}


export interface IAgeEmulationInfo {
    readonly romName: string;
    readonly emulatedSeconds?: number;
    readonly emulationSpeed?: number;
    readonly emulationMaxSpeed?: number;
}

class AgeEmulationInfoGenerator {

    private _emulationInfo: IAgeEmulationInfo;
    private _lastInfoTime = performance.now();
    private _lastEmulatedCycles = 0;
    private _emuMillis = 0;

    constructor(private readonly _emulation: IAgeEmulatorInstance) {
        this._emulationInfo = {
            romName: this._emulation.getRomName(),
        };
    }

    get emulationInfo(): IAgeEmulationInfo {
        return this._emulationInfo;
    }

    checkForUpdate(now: number, emuMillis: number): void {
        this._emuMillis += emuMillis;
        const elapsedMillis = now - this._lastInfoTime;

        if (elapsedMillis > 500) {
            const cyclesPerSecond = this._emulation.getCyclesPerSecond();
            const emulatedCycles = this._emulation.getEmulatedCycles();
            const elapsedCycles = emulatedCycles - this._lastEmulatedCycles;

            this._emulationInfo = {
                ...this._emulationInfo,
                emulatedSeconds: emulatedCycles / cyclesPerSecond,
                emulationSpeed: elapsedCycles * 1000 / elapsedMillis / cyclesPerSecond,
                emulationMaxSpeed: elapsedCycles * 1000 / Math.max(1, this._emuMillis) / cyclesPerSecond,
            };

            this._lastInfoTime = now;
            this._lastEmulatedCycles = emulatedCycles;
            this._emuMillis = 0;
        }
    }
}
