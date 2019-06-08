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
import {Inject, Injectable, Optional, SkipSelf} from "@angular/core";
import {combineLatest, Observable, of} from "rxjs";
import {map, switchMap, tap} from "rxjs/operators";
import {AgeEmulationRunner, AgeGbEmulation} from "../emulation";
import {AgeRomFileLoader, TAgeRomFile} from "./age-rom-file-loader";
import {AgeTaskStatusHandlerService} from "./age-task-status-handler.service";
import {AgeWasmModuleLoader} from "./age-wasm-module-loader";


@Injectable()
export class AgeEmulationFactoryService {

    private readonly _taskStatusHandler: AgeTaskStatusHandlerService;
    private readonly _wasmModuleLoader: AgeWasmModuleLoader;
    private readonly _romFileLoader: AgeRomFileLoader;

    constructor(httpClient: HttpClient,
                ownTaskStatusHandler: AgeTaskStatusHandlerService,
                @Inject(AgeTaskStatusHandlerService) @SkipSelf() @Optional()
                    parentTaskStatusHandler: AgeTaskStatusHandlerService | null) {

        // if there is a global status handler, use it instead of the local one
        this._taskStatusHandler = parentTaskStatusHandler || ownTaskStatusHandler;

        this._wasmModuleLoader = new AgeWasmModuleLoader(httpClient, this._taskStatusHandler);
        this._romFileLoader = new AgeRomFileLoader(httpClient, this._taskStatusHandler);
    }

    newEmulation$(romFileToLoad: TAgeRomFile): Observable<AgeEmulationRunner> {
        return of(true).pipe(
            // clear task status list
            // (in case old status information is still available, e.g. to display error details)
            tap(() => this._taskStatusHandler.clearTasks()),

            // initialize WASM module and load rom file
            switchMap(() => combineLatest([
                this._wasmModuleLoader.wasmModule$,
                this._romFileLoader.loadRomFile$(romFileToLoad),
            ])),

            // create emulation runner
            map(values => {
                const emulation = new AgeGbEmulation(values[0], values[1]);
                return new AgeEmulationRunner(emulation);
            }),

            // clear task status list if everything went fine
            tap(() => this._taskStatusHandler.clearTasks()),
        );
    }
}
