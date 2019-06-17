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

import {ChangeDetectionStrategy, Component, ElementRef, Input, ViewChild} from "@angular/core";
import {faFolderOpen} from "@fortawesome/free-solid-svg-icons/faFolderOpen";
import {AgeEmulationRunnerService, IAgeEmulationRunnerStatus, IAgeRomFileUrl, TAgeRomFile} from "age-lib";
import {Observable} from "rxjs";


@Component({
    selector: "age-app-emulator",
    template: `
        <div class="tmp-toolbar">
            <label for="fileInput"
                   title="Open a Gameboy rom file on the local device.
The rom file is not being uploaded anywhere, it will not leave your device.">
                <fa-icon [icon]="iconOpenLocalRom"></fa-icon>
            </label>
            <input #fileInput
                   type="file"
                   id="fileInput"
                   accept=".gb, .gbc, .zip"
                   (change)="openLocalRom()">
        </div>

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
            min-height: 144px;
            text-align: center;
            position: relative;
        }

        age-emulator {
            height: 100%;
        }

        .tmp-toolbar {
            position: absolute;
        }

        .tmp-toolbar fa-icon {
            cursor: pointer;
        }

        .tmp-toolbar input {
            display: none;
        }

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

    readonly iconOpenLocalRom = faFolderOpen;

    @ViewChild("fileInput", {static: true}) private _fileInput?: ElementRef;

    private _emulationRunnerStatus$?: Observable<IAgeEmulationRunnerStatus>;

    constructor(private readonly _emulationRunnerService: AgeEmulationRunnerService) {
    }


    get emulationRunnerStatus$(): Observable<IAgeEmulationRunnerStatus> | undefined {
        return this._emulationRunnerStatus$;
    }

    @Input() set romUrl(fileUrl: string | null | undefined) {
        const romFile: IAgeRomFileUrl | undefined = fileUrl ? {type: "rom-file-url", fileUrl} : undefined;
        this._newEmulationRunner(romFile);
    }

    openLocalRom(): void {
        const files: FileList = this._fileInput && this._fileInput.nativeElement.files;
        if (files && files.length) {
            this._newEmulationRunner({
                type: "local-rom-file",
                localFile: files[0],
            });
        }
    }


    private _newEmulationRunner(romFile?: TAgeRomFile): void {
        this._emulationRunnerStatus$ = romFile && this._emulationRunnerService.newEmulationRunner$(romFile);
    }
}
