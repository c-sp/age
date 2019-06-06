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
    Component,
    EventEmitter,
    Inject,
    Input,
    Optional,
    Output,
    SkipSelf,
} from "@angular/core";
import {combineLatest, Observable} from "rxjs";
import {map} from "rxjs/operators";
import {AgeEmulationPackage, IAgeEmulationRuntimeInfo} from "./common";
import {AgeRomFileLoaderService, AgeWasmModuleLoaderService, TAgeRomFile} from "./loader";
import {AgeTaskStatusService} from "./loader/age-task-status.service";


@Component({
    selector: "age-emulator",
    template: `
        <!-- TODO task status list -->
        <age-emulation *ngIf="(emulationPackage$ | async) as emulationPackage"
                       [emulationPackage]="emulationPackage"
                       (updateRuntimeInfo)="updateRuntimeInfo.emit($event)"></age-emulation>
    `,
    styles: [`
        :host {
            text-align: center;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
    providers: [
        AgeTaskStatusService,
        AgeRomFileLoaderService,
        AgeWasmModuleLoaderService, // this is just a fallback in case there is no global WASM loader
    ],
})
export class AgeEmulatorComponent {

    @Output() readonly updateRuntimeInfo = new EventEmitter<IAgeEmulationRuntimeInfo | undefined>();

    private readonly _wasmModuleLoader: AgeWasmModuleLoaderService;
    private _emulationPackage$?: Observable<AgeEmulationPackage>;

    constructor(private readonly _romFileLoader: AgeRomFileLoaderService,
                ownWasmModuleLoader: AgeWasmModuleLoaderService,
                @Inject(AgeWasmModuleLoaderService) @SkipSelf() @Optional()
                    parentWasmModuleLoader: AgeWasmModuleLoaderService | null) {

        this._wasmModuleLoader = parentWasmModuleLoader || ownWasmModuleLoader;
    }

    get emulationPackage$(): Observable<AgeEmulationPackage> | undefined {
        return this._emulationPackage$;
    }

    @Input() set loadRomFile(romFileToLoad: TAgeRomFile | undefined) {
        this.updateRuntimeInfo.emit(undefined);

        this._emulationPackage$ = !romFileToLoad
            ? undefined
            : combineLatest([
                this._wasmModuleLoader.wasmModule$,
                this._romFileLoader.loadRomFile$(romFileToLoad),
            ]).pipe(
                map(values => new AgeEmulationPackage(values[0], values[1])),
            );
    }
}
