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

import {
    ChangeDetectionStrategy,
    ChangeDetectorRef,
    Component,
    EventEmitter,
    HostListener,
    Input,
    OnDestroy,
    OnInit,
    Output,
} from "@angular/core";
import {AgeEmulationRunner, IAgeEmulationRuntimeInfo} from "../emulation";
import {AgeGbKeyMap} from "../settings";
import {AgeAudio} from "./audio/age-audio";


@Component({
    selector: "age-emulator",
    template: `
        <age-canvas-renderer *ngIf="emulationRunner as emuRunner"
                             [screenWidth]="emuRunner.screenSize.width"
                             [screenHeight]="emuRunner.screenSize.height"
                             [newFrame]="emuRunner.screenBuffer"></age-canvas-renderer>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeEmulatorComponent implements OnInit, OnDestroy {

    @Output() readonly updateRuntimeInfo = new EventEmitter<IAgeEmulationRuntimeInfo | undefined>();

    private readonly _keyMap = new AgeGbKeyMap();

    private _audio?: AgeAudio;
    private _timerHandle!: number;
    private _emulationRunner?: AgeEmulationRunner;
    private _lastRuntimeInfo?: IAgeEmulationRuntimeInfo;

    constructor(private readonly _changeDetector: ChangeDetectorRef) {
    }

    ngOnInit(): void {
        this._timerHandle = window.setInterval(
            () => this.runEmulation(),
            10,
        );
    }

    async ngOnDestroy() {
        window.clearInterval(this._timerHandle);
        if (this._audio) {
            await this._audio.close();
        }
    }


    get emulationRunner(): AgeEmulationRunner | undefined {
        return this._emulationRunner;
    }

    @Input() set emulationRunner(emulationRunner: AgeEmulationRunner | undefined) {
        this._emulationRunner = emulationRunner;
        // init after the emulationRunner has been set,
        // because the latter is triggere by a user interaction which
        // is required to create the AudioContext
        if (!this._audio) {
            this._audio = new AgeAudio();
        }
    }


    @HostListener("document:keydown", ["$event"]) handleKeyDown(event: KeyboardEvent) {
        const gbButton = this._keyMap.getButtonForKey(event.key);

        if (this._emulationRunner && gbButton) {
            this._emulationRunner.buttonDown(gbButton);
            event.preventDefault();
        }
    }

    @HostListener("document:keyup", ["$event"]) handleKeyUp(event: KeyboardEvent) {
        const gbButton = this._keyMap.getButtonForKey(event.key);

        if (this._emulationRunner && gbButton) {
            this._emulationRunner.buttonUp(gbButton);
            event.preventDefault();
        }
    }


    private runEmulation(): void {
        let newRuntimeInfo;

        if (this._emulationRunner && this._audio) {
            this._emulationRunner.emulate(this._audio.sampleRate);
            newRuntimeInfo = this._emulationRunner.runtimeInfo;

            this._audio.stream(this._emulationRunner.audioBuffer);
            this._changeDetector.markForCheck();
        }

        if (!isSameRuntimeInfo(newRuntimeInfo, this._lastRuntimeInfo)) {
            this._lastRuntimeInfo = newRuntimeInfo;
            this.updateRuntimeInfo.emit(this._lastRuntimeInfo);
        }
    }
}


function isSameRuntimeInfo(x?: IAgeEmulationRuntimeInfo, y?: IAgeEmulationRuntimeInfo): boolean {
    return !!x && !!y
        && (x.romName === y.romName)
        && (x.emulatedSeconds === y.emulatedSeconds)
        && (x.emulationSpeed === y.emulationSpeed)
        && (x.emulationMaxSpeed === y.emulationMaxSpeed)
        || (!x && !y);
}
