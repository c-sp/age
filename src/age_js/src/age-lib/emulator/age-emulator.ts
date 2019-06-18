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

import {AgeSubscriptionLike} from "../common";
import {AgeEmulationRunner} from "../emulation";
import {AgeGbKeyMap} from "../settings";
import {AgeAudio} from "./audio/age-audio";


export class AgeEmulator extends AgeSubscriptionLike {

    private readonly _keyMap = new AgeGbKeyMap();
    private readonly _timerHandle: number;
    private _canvasCtx?: CanvasRenderingContext2D;
    private _audio?: AgeAudio;
    private _emulationRunner?: AgeEmulationRunner;

    constructor() {
        super(() => {
            window.clearInterval(this._timerHandle);
            if (this._audio) {
                this._audio.close();
                this._audio = undefined;
            }
        });

        this._timerHandle = window.setInterval(
            () => this.runEmulation(),
            10,
        );
    }


    get emulationRunner(): AgeEmulationRunner | undefined {
        return this._emulationRunner;
    }

    set emulationRunner(emulationRunner: AgeEmulationRunner | undefined) {
        this._emulationRunner = emulationRunner;
        // init after the emulationRunner has been set,
        // because the latter is triggere by a user interaction which
        // is required to create the AudioContext
        if (!this._audio) {
            this._audio = new AgeAudio();
        }
    }

    set canvasCtx(canvasCtx: CanvasRenderingContext2D) {
        this._canvasCtx = canvasCtx;
    }


    handleKeyDown(event: KeyboardEvent) {
        const gbButton = this._keyMap.getButtonForKey(event.key);

        if (this._emulationRunner && gbButton) {
            this._emulationRunner.buttonDown(gbButton);
            event.preventDefault();
        }
    }

    handleKeyUp(event: KeyboardEvent) {
        const gbButton = this._keyMap.getButtonForKey(event.key);

        if (this._emulationRunner && gbButton) {
            this._emulationRunner.buttonUp(gbButton);
            event.preventDefault();
        }
    }


    private runEmulation(): void {
        const emuRunner = this._emulationRunner;

        if (emuRunner) {
            const newFrame = emuRunner.emulate((this._audio && this._audio.sampleRate) || 1);

            if (this._audio) {
                this._audio.stream(emuRunner.audioBuffer);
            }

            if (newFrame && this._canvasCtx) {
                const screenBuffer = emuRunner.screenBuffer;
                const screenWidth = emuRunner.screenSize.width;
                const screenHeight = emuRunner.screenSize.height;

                const numBytes = screenWidth * screenHeight * 4;
                const bytes = new Uint8ClampedArray(screenBuffer.buffer.buffer, screenBuffer.offset, numBytes);
                const imageData = new ImageData(bytes, screenWidth, screenHeight);

                this._canvasCtx.putImageData(imageData, 0, 0);
            }

            // TODO handle this.updateRuntimeInfo.emit(emuRunner.runtimeInfo);
        }
    }
}
