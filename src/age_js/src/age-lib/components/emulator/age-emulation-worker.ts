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

import {AgeSubscriptionLike} from "../../common";
import {AgeEmulation} from "../../emulation";
import {AgeGbKeyMap} from "../../settings";
import {AgeAudio} from "./audio/age-audio";


// TODO better handling of AudioContext creation
//      (see also: https://hackernoon.com/unlocking-web-audio-the-smarter-way-8858218c0e09)


export class AgeEmulationWorker extends AgeSubscriptionLike {

    private readonly _keyMap = new AgeGbKeyMap();
    private readonly _timerHandle: number;
    private _canvasCtx?: CanvasRenderingContext2D | null;
    private _audio = new AgeAudio();
    private _emulation?: AgeEmulation;
    private _pauseEmulation = false;

    constructor() {
        super(() => {
            window.clearInterval(this._timerHandle);
            if (this._audio) {
                this._audio.close();
            }
        });

        this._timerHandle = window.setInterval(
            () => this.runEmulation(),
            10,
        );
    }


    set audioVolume(volume: number) {
        this._audio.volume = volume;
    }

    get emulation(): AgeEmulation | undefined {
        return this._emulation;
    }

    set emulation(emulation: AgeEmulation | undefined) {
        this._emulation = emulation;
    }

    set pauseEmulation(pauseEmulation: boolean) {
        this._pauseEmulation = pauseEmulation;
    }

    set canvasCtx(canvasCtx: CanvasRenderingContext2D | null | undefined) {
        this._canvasCtx = canvasCtx;
    }


    handleKeyDown(event: KeyboardEvent) {
        const gbButton = this._keyMap.getButtonForKey(event.key);

        if (this._emulation && gbButton) {
            this._emulation.buttonDown(gbButton);
            event.preventDefault();
        }
    }

    handleKeyUp(event: KeyboardEvent) {
        const gbButton = this._keyMap.getButtonForKey(event.key);

        if (this._emulation && gbButton) {
            this._emulation.buttonUp(gbButton);
            event.preventDefault();
        }
    }


    private runEmulation(): void {
        const emulation = this._emulation;

        // TODO is the timing okay when reacting to pauseEmulation like this?
        if (emulation && !this._pauseEmulation) {
            const newFrame = emulation.emulate((this._audio && this._audio.sampleRate) || 48000);

            if (this._audio) {
                this._audio.stream(emulation.audioBuffer);
            }

            if (newFrame && this._canvasCtx) {
                const screenBuffer = emulation.screenBuffer;
                const screenWidth = emulation.screenSize.width;
                const screenHeight = emulation.screenSize.height;

                const numBytes = screenWidth * screenHeight * 4;
                const bytes = new Uint8ClampedArray(screenBuffer.buffer.buffer, screenBuffer.offset, numBytes);
                const imageData = new ImageData(bytes, screenWidth, screenHeight);

                this._canvasCtx.putImageData(imageData, 0, 0);
            }

            // TODO handle this.updateRuntimeInfo.emit(emulation.emulationInfo);
        }
    }
}
