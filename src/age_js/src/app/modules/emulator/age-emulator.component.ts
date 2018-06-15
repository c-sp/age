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

import {ChangeDetectionStrategy, ChangeDetectorRef, Component, HostListener, Inject, Input, OnDestroy, OnInit} from '@angular/core';
import {AgeEmulationRunner, AgeGbEmulation} from './age-emulation';
import {AgeGbKeyMap} from './age-emulator-keymap';
import {AgeRect} from '../common/age-rect';
import {AgeEmulationPackage} from '../common/age-emulation-package';
import {AgeAudio} from './audio/age-audio';


@Component({
    selector: 'age-emulator',
    template: `
        <ng-container *ngIf="emulationRunner as emulator">
            <age-canvas-renderer [screenSize]="emulator.screenSize"
                                 [newFrame]="emulator.screenBuffer"
                                 [viewport]="viewport"></age-canvas-renderer>
        </ng-container>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeEmulatorComponent implements OnInit, OnDestroy {

    @Input() viewport = new AgeRect(1, 1);

    private readonly _keyMap = new AgeGbKeyMap();

    private _audio!: AgeAudio;
    private _timerHandle!: number;
    private _emulationRunner?: AgeEmulationRunner;

    constructor(@Inject(ChangeDetectorRef) private readonly _changeDetector: ChangeDetectorRef) {
    }

    ngOnInit(): void {
        this._audio = new AgeAudio();
        this._timerHandle = window.setInterval(
            () => {
                if (this._emulationRunner) {
                    this._emulationRunner.emulate();
                    this._audio.stream(this._emulationRunner.audioBuffer);
                    this._changeDetector.detectChanges();
                }
            },
            10
        );
    }

    ngOnDestroy(): void {
        window.clearInterval(this._timerHandle);
        this._audio.close();
    }


    get emulationRunner(): AgeEmulationRunner | undefined {
        return this._emulationRunner;
    }

    @Input()
    set emulationPackage(emulationPackage: AgeEmulationPackage | undefined) {
        if (!emulationPackage) {
            this._emulationRunner = undefined;
        } else {
            this._emulationRunner = new AgeEmulationRunner(
                new AgeGbEmulation(
                    emulationPackage.emGbModule,
                    emulationPackage.romFileContents
                )
            );
        }
    }


    @HostListener('document:keydown', ['$event'])
    handleKeyDown(event: KeyboardEvent) {
        const gbButton = this._keyMap.getButtonForKey(event.key);

        if (this._emulationRunner && gbButton) {
            this._emulationRunner.buttonDown(gbButton);
            event.preventDefault();
        }
    }

    @HostListener('document:keyup', ['$event'])
    handleKeyUp(event: KeyboardEvent) {
        const gbButton = this._keyMap.getButtonForKey(event.key);

        if (this._emulationRunner && gbButton) {
            this._emulationRunner.buttonUp(gbButton);
            event.preventDefault();
        }
    }
}
