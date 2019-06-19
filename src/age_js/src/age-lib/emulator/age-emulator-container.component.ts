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
import {AgeEmulationRunnerService, IAgeEmulationRunnerStatus, TAgeRomFile} from "../emulation";
import {AgePlayPauseStatus} from "../toolbar";


@Component({
    selector: "age-emulator-container",
    template: `
        <ng-container *ngIf="(emulationRunnerStatus$ |async) as emuStatus; else selectRomHint">

            <age-emulator *ngIf="emuStatus.emulationRunner as emuRunner; else noRunner"
                          [emulationRunner]="emuRunner"></age-emulator>

            <ng-template #noRunner>
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
                <age-toolbar-action-play [playPauseStatus]="PlayPauseStatus.DISABLED"></age-toolbar-action-play>
                <age-toolbar-action-volume [isMuted]="false"></age-toolbar-action-volume>
                <ng-content></ng-content>
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

        age-emulator {
            height: 100%;
        }

        .toolbar {
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
        }

        .toolbar > :nth-child(1) {
            opacity: 0.7;
            position: absolute;
            left: 0;
            top: 0;
        }

        .toolbar > :nth-child(2) {
            background: none;
        }
    `],
    providers: [
        AgeEmulationRunnerService,
    ],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeEmulatorContainerComponent {

    readonly PlayPauseStatus = AgePlayPauseStatus;

    private _emulationRunnerStatus$?: Observable<IAgeEmulationRunnerStatus>;

    constructor(private readonly _emulationRunnerService: AgeEmulationRunnerService) {
    }


    get emulationRunnerStatus$(): Observable<IAgeEmulationRunnerStatus> | undefined {
        return this._emulationRunnerStatus$;
    }

    @Input() set romFile(romFile: TAgeRomFile | undefined) {
        this._emulationRunnerStatus$ = romFile && this._emulationRunnerService.newEmulationRunner$(romFile);
    }
}
