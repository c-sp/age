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

import {AfterViewInit, ChangeDetectionStrategy, ChangeDetectorRef, Component} from '@angular/core';


@Component({
    selector: 'age-splash-screen',
    template: `
        <div class="container age-ui">

            <div class="title">
                Welcome to
                <div>AGE</div>
                <span>A</span>nother
                <span>G</span>ameboy
                <span>E</span>mulator
            </div>

            <div>
                To open a Gameboy rom file<br>
                please click the <i class="fa fa-folder-open"></i> icon above.
            </div>

            <div *ngIf="!audioWorkletAvailable" class="warning age-ui-error">
                <i class="fa fa-exclamation-circle age-ui-big-icon"></i>
                <div>
                    AGE will remain silent as the required audio API is not available:
                    this browser does not support
                    <a href="https://webaudio.github.io/web-audio-api/#AudioWorklet"
                       target="_blank"
                       rel="nofollow noreferrer"
                       title="Link to the AudioWorklet specification"
                       class="age-ui-clickable">AudioWorklet</a>.
                </div>
            </div>

            <div *ngIf="!preciseTimer" class="warning age-ui-error">
                <i class="fa fa-exclamation-circle age-ui-big-icon"></i>
                <div>
                    This browser's timer precision may be too low for AGE to run a smooth emulation.
                </div>
            </div>

        </div>
    `,
    styles: [`
        .container {
            text-align: center;
            background-color: black;
        }

        .container > :nth-child(n+2) {
            margin-top: 2em;
        }

        .title {
            font-size: x-large;
            font-weight: bold;
            color: rgb(180, 150, 120);
            text-shadow: .1em .1em rgba(180, 150, 120, 0.4);
        }

        .title span {
            color: rgb(230, 200, 170);
        }

        .title > div {
            color: rgb(230, 200, 170);
            font-size: 4em;
            text-shadow: .07em .07em rgba(180, 150, 120, 0.4);
        }

        .warning {
            max-width: 30em;
            margin-left: auto;
            margin-right: auto;
            text-align: left;
            display: flex;
            align-items: center;
        }

        .warning > i {
            margin-right: .25em;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeSplashScreenComponent implements AfterViewInit {

    private static isAudioWorkletAvailable(): boolean {
        const audioCtx = new AudioContext();

        /* tslint:disable no-any */ // at the moment I see no other way ...
        const result = !!(audioCtx as any).audioWorklet;
        /* tslint:enable no-any */

        audioCtx.close();
        return result;
    }


    private _audioWorkletAvailable = true;
    private _preciseTimer = true;

    private _timestamps: number[] = [];
    private _intervalHandle?: number;

    constructor(private _cdRef: ChangeDetectorRef) {
    }

    ngAfterViewInit(): void {
        this._audioWorkletAvailable = AgeSplashScreenComponent.isAudioWorkletAvailable();
        this.measureTimerPrecision();
    }

    get audioWorkletAvailable(): boolean {
        return this._audioWorkletAvailable;
    }

    get preciseTimer(): boolean {
        return this._preciseTimer;
    }


    private measureTimerPrecision() {
        // We measure the timer precision since it may be severely limited
        // depending on the browser and it's configuration:
        // https://developer.mozilla.org/en-US/docs/Web/API/Performance/now

        if (!this._intervalHandle) {
            this._timestamps = [performance.now()];

            this._intervalHandle = window.setInterval(
                () => this.measureTimerPrecisionStep(),
                2
            );
        }
    }

    private measureTimerPrecisionStep() {
        this._timestamps.push(performance.now());

        const duration = this._timestamps[this._timestamps.length - 1] - this._timestamps[0];
        if (duration > 200) {
            this.finishTimerPrecisionMeasurement();
        }
    }

    private finishTimerPrecisionMeasurement() {
        if (this._intervalHandle) {
            window.clearInterval(this._intervalHandle);
            this._intervalHandle = undefined;

            let minDiff = 100000;
            for (let i = 0, end = this._timestamps.length - 1; i < end; ++i) {
                const diff = this._timestamps[i + 1] - this._timestamps[i];
                if (diff > 0) {
                    minDiff = Math.min(diff, minDiff);
                }
            }

            this._preciseTimer = minDiff < 10;
            this._cdRef.detectChanges();
        }
    }
}
