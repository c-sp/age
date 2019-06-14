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

import {HttpClient} from "@angular/common/http";
import {Injectable} from "@angular/core";
import {combineLatest, merge, Observable, of} from "rxjs";
import {map} from "rxjs/operators";
import {AgeGbEmulation} from "./age-emulation";
import {AgeEmulationRunner} from "./age-emulation-runner";
import {AgeRomFileLoader, TAgeRomFile} from "./age-rom-file-loader";
import {AgeTaskStatusHandler, IAgeTaskStatus} from "./age-task-status-handler";
import {AgeWasmModuleLoader} from "./age-wasm-module-loader";


export interface IAgeEmulationRunnerStatus {
    readonly emulationRunner?: AgeEmulationRunner;
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
export class AgeEmulationRunnerService {

    private readonly _wasmModuleLoader: AgeWasmModuleLoader;

    constructor(private readonly _httpClient: HttpClient) {
        this._wasmModuleLoader = new AgeWasmModuleLoader(this._httpClient);
    }

    newEmulationRunner$(romFileToLoad: TAgeRomFile): Observable<IAgeEmulationRunnerStatus> {
        const taskStatusHandler = new AgeTaskStatusHandler();

        // load the required files and create a new emulation runner
        const emulationRunner$ = combineLatest([
            this._wasmModuleLoader.newWasmModule$(taskStatusHandler),
            new AgeRomFileLoader(this._httpClient, taskStatusHandler).loadRomFile$(romFileToLoad),
        ]).pipe(
            map(values => {
                const emulation = new AgeGbEmulation(values[0], values[1]);
                return new AgeEmulationRunner(emulation);
            }),
        );

        // combine the current loading task status and the final emulation runner into one result
        return combineLatest([
            taskStatusHandler.taskStatusList$,
            merge(of(undefined), emulationRunner$), // emit undefined until the emulation runner is ready
        ]).pipe(
            map(values => {
                return {
                    emulationRunner: values[1],
                    taskStatusList: values[0],
                };
            }),
        );
    }
}
