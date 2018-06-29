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


// TODO use AudioWorklet polyfill to support more browsers


//
// Links:
//  http://blog.mecheye.net/2017/09/i-dont-know-who-the-web-audio-api-is-designed-for/
//  https://webaudio.github.io/web-audio-api/#audioworklet
//  https://github.com/GoogleChromeLabs/audioworklet-polyfill
//

export class AgeAudio {

    private readonly _audioCtx = new AudioContext();
    private _workletNode?: AudioWorkletNode;

    constructor() {
        /* tslint:disable no-any */ // at the moment I see no other way ...
        const audioCtx: any = this._audioCtx;
        /* tslint:enable no-any */

        if (audioCtx.audioWorklet) {
            audioCtx.audioWorklet.addModule('assets/age-audio-stream.js').then(() => {
                this._workletNode = new AudioWorkletNode(this._audioCtx, 'age-audio-stream', {
                    numberOfInputs: 1,
                    numberOfOutputs: 1,
                    outputChannelCount: [2]
                });

                this._workletNode.connect(this._audioCtx.destination);
            });
        }
    }

    close(): void {
        this._audioCtx.close();
    }

    get sampleRate(): number {
        // according to http://blog.mecheye.net/2017/09/i-dont-know-who-the-web-audio-api-is-designed-for/
        // the sample rate can change when switching audio devices
        return this._audioCtx.sampleRate;
    }

    stream(buffer: Int16Array): void {
        if (this._workletNode) {
            this._workletNode.port.postMessage({
                sampleRate: this._audioCtx.sampleRate,
                samples: buffer
            });
        }
    }
}
