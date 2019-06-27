//
// Copyright 2019 Christoph Sprenger
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

import {HttpClient} from "@angular/common/http";
import {Injectable} from "@angular/core";
import {combineLatest, merge, Observable, of} from "rxjs";
import {map} from "rxjs/operators";
import {AgeEmulation} from "./age-emulation";
import {AgeGbEmulatorInstance} from "./age-gb-emulator-instance";
import {AgeRomFileLoader, TAgeRomFile} from "./age-rom-file-loader";
import {AgeTaskStatusHandler, IAgeTaskStatus} from "./age-task-status-handler";
import {AgeWasmModuleLoader} from "./age-wasm-module-loader";


export interface IAgeEmulationStatus {
    readonly emulation?: AgeEmulation;
    readonly taskStatusList: ReadonlyArray<IAgeTaskStatus>;
}


/**
 * Helper Service for loading a rom file.
 *
 * This service must be provided by a suitable component.
 * It must NOT be provided by the `root` injector since it should not be shared
 * across multiple AGE web components.
 */
@Injectable()
export class AgeEmulationService {

    private readonly _wasmModuleLoader: AgeWasmModuleLoader;

    constructor(private readonly _httpClient: HttpClient) {
        this._wasmModuleLoader = new AgeWasmModuleLoader(this._httpClient);
    }

    newEmulation$(romFileToLoad: TAgeRomFile): Observable<IAgeEmulationStatus> {
        const taskStatusHandler = new AgeTaskStatusHandler();

        // load the required files and create a new emulation
        const emulation$ = combineLatest([
            this._wasmModuleLoader.newWasmModule$(taskStatusHandler),
            new AgeRomFileLoader(this._httpClient, taskStatusHandler).loadRomFile$(romFileToLoad),
        ]).pipe(
            map(values => {
                const emulation = new AgeGbEmulatorInstance(values[0], values[1]);
                return new AgeEmulation(emulation);
            }),
        );

        // combine the current loading task status and the final emulation into one result
        return combineLatest([
            taskStatusHandler.taskStatusList$,
            merge(of(undefined), emulation$), // emit undefined until the emulation is ready
        ]).pipe(
            map(values => {
                return {
                    emulation: values[1],
                    taskStatusList: values[0],
                };
            }),
        );
    }
}
