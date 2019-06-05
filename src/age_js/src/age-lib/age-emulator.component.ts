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

import {ChangeDetectionStrategy, Component, EventEmitter, Input, Output} from "@angular/core";
import {AgeEmulationPackage, AgeRomFileToLoad, IAgeEmulationRuntimeInfo} from "./common";

// export @Input and @Output types
export {AgeRomFileToLoad, IAgeEmulationRuntimeInfo};


@Component({
    selector: "age-emulator",
    template: `
        <age-loader [romFileToLoad]="loadRomFile"
                    (loadingComplete)="loadingComplete($event)"></age-loader>

        <age-emulation *ngIf="emulationPackage"
                       [emulationPackage]="emulationPackage"
                       (updateRuntimeInfo)="updateRuntimeInfo.emit($event)"></age-emulation>
    `,
    styles: [`
        :host {
            text-align: center;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeEmulatorComponent {

    @Output() readonly updateRuntimeInfo = new EventEmitter<IAgeEmulationRuntimeInfo | undefined>();

    private _romFileToLoad?: AgeRomFileToLoad;
    private _emulationPackage?: AgeEmulationPackage;


    get emulationPackage(): AgeEmulationPackage | undefined {
        return this._emulationPackage;
    }

    get loadRomFile(): AgeRomFileToLoad | undefined {
        return this._romFileToLoad;
    }

    @Input() set loadRomFile(fileToLoad: AgeRomFileToLoad | undefined) {
        this._romFileToLoad = fileToLoad;
        this._emulationPackage = undefined;
        this.updateRuntimeInfo.emit(undefined);
    }


    loadingComplete(emulationPackage: AgeEmulationPackage) {
        this._emulationPackage = emulationPackage;
    }
}
