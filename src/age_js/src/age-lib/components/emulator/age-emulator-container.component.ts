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

import {ChangeDetectionStrategy, Component, Input} from "@angular/core";
import {Observable} from "rxjs";
import {tap} from "rxjs/operators";
import {AgeEmulationService, IAgeEmulationStatus, TAgeRomFile} from "../../emulation";
import {AgePlayPauseStatus} from "../toolbar";


@Component({
    selector: "age-emulator-container",
    template: `
        <ng-container *ngIf="(emulationStatus$ |async) as emuStatus; else selectRomHint">

            <age-emulation *ngIf="emuStatus.emulation as emulation; else noEmulation"
                           [emulation]="emulation"
                           [pauseEmulation]="pauseEmulation"></age-emulation>

            <ng-template #noEmulation>
                <age-task-status class="below-toolbar"
                                 [taskStatusList]="emuStatus.taskStatusList"></age-task-status>
            </ng-template>

        </ng-container>

        <ng-template #selectRomHint>
            <div class="below-toolbar">
                please open a rom file
            </div>
        </ng-template>

        <div class="toolbar">
            <mat-toolbar></mat-toolbar>
            <mat-toolbar>
                <age-toolbar-spacer></age-toolbar-spacer>

                <age-toolbar-action-play [playPauseStatus]="playPauseStatus"
                                         (paused)="isPaused = $event"></age-toolbar-action-play>

                <age-toolbar-action-volume [isMuted]="false"></age-toolbar-action-volume>

                <ng-content></ng-content>
                <age-toolbar-spacer></age-toolbar-spacer>
            </mat-toolbar>
        </div>
    `,
    styles: [`
        :host {
            display: block;
            /* make sure there is enough space to display the rom loading status */
            min-height: calc(64px + 10em);
            text-align: center;
            position: relative;
        }

        age-task-status {
            display: block;
        }

        .below-toolbar {
            padding-top: calc(64px + 2em);
        }

        age-emulation {
            height: 100%;
        }

        .toolbar {
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
        }

        .toolbar > :nth-child(1) {
            opacity: 0.65;
            position: absolute;
            left: 0;
            top: 0;
        }

        .toolbar > :nth-child(2) {
            background: none;
        }
    `],
    providers: [
        AgeEmulationService,
    ],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeEmulatorContainerComponent {

    private _emulationStatus$?: Observable<IAgeEmulationStatus>;
    private _hasEmulation = false;
    private _forcePause = false;
    private _isPaused = false;

    constructor(private readonly _emulationService: AgeEmulationService) {
    }


    get emulationStatus$(): Observable<IAgeEmulationStatus> | undefined {
        return this._emulationStatus$;
    }

    get pauseEmulation(): boolean {
        return this._forcePause || this._isPaused;
    }

    get playPauseStatus(): AgePlayPauseStatus {
        if (!this._hasEmulation) {
            return AgePlayPauseStatus.DISABLED;
        }
        return this.pauseEmulation ? AgePlayPauseStatus.IS_PAUSED : AgePlayPauseStatus.IS_PLAYING;
    }

    @Input() set forcePause(forcePause: boolean) {
        this._forcePause = forcePause;
    }

    @Input() set isPaused(isPaused: boolean) {
        this._isPaused = isPaused;
    }

    @Input() set romFile(romFile: TAgeRomFile | undefined) {
        if (!romFile) {
            this._hasEmulation = false;
            this._emulationStatus$ = undefined;
            return;
        }
        this._emulationStatus$ = this._emulationService.newEmulation$(romFile).pipe(
            tap(emulationStatus => this._hasEmulation = !!emulationStatus.emulation),
        );
    }
}
