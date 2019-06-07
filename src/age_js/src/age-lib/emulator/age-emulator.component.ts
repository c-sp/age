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
import {Observable} from "rxjs";
import {AgeEmulationRunner, IAgeEmulationRuntimeInfo} from "../emulation";
import {AgeEmulationFactoryService, AgeTaskStatusHandlerService, TAgeRomFile} from "../emulation-loader";


@Component({
    selector: "age-emulator",
    template: `
        <age-emulation *ngIf="(emulationRunner$ | async) as emulationRunner"
                       [emulationRunner]="emulationRunner"
                       (updateRuntimeInfo)="updateRuntimeInfo.emit($event)"></age-emulation>
    `,
    styles: [`
        :host {
            text-align: center;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
    providers: [
        // these are just fallback services in case there are no global ones
        // (global services allow for displaying loading status information and
        // for WASM code file caching independent of this component's lifecycle)
        AgeTaskStatusHandlerService,
        AgeEmulationFactoryService,
    ],
})
export class AgeEmulatorComponent {

    @Output() readonly updateRuntimeInfo = new EventEmitter<IAgeEmulationRuntimeInfo | undefined>();

    private readonly _emulationFactory: AgeEmulationFactoryService;
    private _emulationRunner$?: Observable<AgeEmulationRunner>;

    constructor(ownEmulationFactory: AgeEmulationFactoryService,
                @Inject(AgeEmulationFactoryService) @SkipSelf() @Optional()
                    parentEmulationFactory: AgeEmulationFactoryService | null) {

        this._emulationFactory = parentEmulationFactory || ownEmulationFactory;
    }

    get emulationRunner$(): Observable<AgeEmulationRunner> | undefined {
        return this._emulationRunner$;
    }

    @Input() set loadRomFile(romFileToLoad: TAgeRomFile) {
        this.updateRuntimeInfo.emit(undefined);
        this._emulationRunner$ = this._emulationFactory.newEmulation$(romFileToLoad);
    }
}
