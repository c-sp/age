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
import {
    AgeEmulationRunnerService,
    AgePlayPauseStatus,
    IAgeEmulationRunnerStatus,
    IAgeRomFileUrl,
    TAgeRomFile,
} from "age-lib";
import {Observable} from "rxjs";


@Component({
    selector: "age-app-emulator",
    template: `
        <mat-toolbar>
            <age-toolbar-action-play [playPauseStatus]="AgePlayPauseStatus.DISABLED"></age-toolbar-action-play>
            <age-toolbar-action-volume [isMuted]="false"></age-toolbar-action-volume>
            <age-toolbar-spacer></age-toolbar-spacer>
            <age-toolbar-action-local-rom (openLocalRom)="newEmulationRunner($event)"></age-toolbar-action-local-rom>
        </mat-toolbar>

        <ng-container *ngIf="(emulationRunnerStatus$ |async) as emuStatus">

            <age-emulator *ngIf="emuStatus.emulationRunner as emuRunner; else noRunner"
                          [emulationRunner]="emuRunner"></age-emulator>

            <ng-template #noRunner>
                <age-task-status [taskStatusList]="emuStatus.taskStatusList"></age-task-status>
            </ng-template>

        </ng-container>
    `,
    styles: [`
        :host {
            display: block;
            min-width: 160px;
            /*min-height: 144px;*/
            min-height: 208px;
            text-align: center;
            position: relative;
        }

        age-emulator {
            height: calc(100% - 64px);
        }

        /* TODO move toolbar above emulator element and use absolute position */
        /*mat-toolbar {
            position: absolute;
            top: 0;
            left: 0;
        }*/

        age-task-status {
            display: block;
            padding: 1em;
            font-size: smaller;
        }
    `],
    providers: [
        AgeEmulationRunnerService,
    ],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeAppEmulatorComponent {

    readonly AgePlayPauseStatus = AgePlayPauseStatus;

    private _emulationRunnerStatus$?: Observable<IAgeEmulationRunnerStatus>;

    constructor(private readonly _emulationRunnerService: AgeEmulationRunnerService) {
    }


    get emulationRunnerStatus$(): Observable<IAgeEmulationRunnerStatus> | undefined {
        return this._emulationRunnerStatus$;
    }

    @Input() set romUrl(fileUrl: string | null | undefined) {
        const romFile: IAgeRomFileUrl | undefined = fileUrl ? {type: "rom-file-url", fileUrl} : undefined;
        this.newEmulationRunner(romFile);
    }

    newEmulationRunner(romFile?: TAgeRomFile): void {
        this._emulationRunnerStatus$ = romFile && this._emulationRunnerService.newEmulationRunner$(romFile);
    }
}
