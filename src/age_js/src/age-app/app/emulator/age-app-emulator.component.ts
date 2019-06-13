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
    AgeEmulationFactoryService,
    AgeEmulationRunner,
    AgeSubscriptionSink,
    AgeTaskStatusHandlerService,
    IAgeRomFileUrl,
    TAgeRomFile,
} from "age-lib";
import {Observable} from "rxjs";


@Component({
    selector: "age-app-emulator",
    template: `
        <age-emulator *ngIf="(emulationRunner$ |async) as emuRunner; else noEmuRunner"
                      [emulationRunner]="emuRunner"></age-emulator>

        <ng-template #noEmuRunner>
            <age-task-status></age-task-status>
        </ng-template>
    `,
    styles: [`
        :host {
            display: block;
            min-width: 160px;
            min-height: 144px;
            text-align: center;
        }

        age-task-status {
            display: block;
            padding: 1em;
            font-size: smaller;
        }
    `],
    providers: [
        AgeTaskStatusHandlerService,
        AgeEmulationFactoryService,
    ],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeAppEmulatorComponent extends AgeSubscriptionSink {

    private _emulationRunner$?: Observable<AgeEmulationRunner>;

    constructor(private readonly _emulationFactory: AgeEmulationFactoryService) {
        super();
    }

    get emulationRunner$(): Observable<AgeEmulationRunner> | undefined {
        return this._emulationRunner$;
    }


    @Input() set romUrl(fileUrl: string | null | undefined) {
        const romFile: IAgeRomFileUrl | undefined = fileUrl ? {type: "rom-file-url", fileUrl} : undefined;
        this._newEmulationRunner(romFile);
    }


    private _newEmulationRunner(romFile?: TAgeRomFile): void {
        this._emulationRunner$ = romFile ? this._emulationFactory.newEmulation$(romFile) : undefined;
    }
}
