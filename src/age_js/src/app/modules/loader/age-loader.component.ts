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

import {ChangeDetectionStrategy, ChangeDetectorRef, Component, EventEmitter, Input, Output} from '@angular/core';
import {AgeEmulationPackage, AgeRomFileToLoad, EmGbModule} from '../../common';


@Component({
    selector: 'age-loader',
    template: `

        <!--
         The WASM loader is special in that it also unloads the WASM when destroyed.
         Thus we keep it alive the whole time, though not always visible.
         -->
        <age-wasm-loader [showState]="loading"
                         (emGbModuleLoaded)="emGbModuleLoaded($event)"></age-wasm-loader>

        <ng-container *ngIf="loading">

            <age-rom-file-loader #fileLoader
                                 [romFileToLoad]="romFileToLoad"></age-rom-file-loader>

            <age-rom-file-extractor [fileContents]="fileLoader.fileLoaded|async"
                                    (fileExtracted)="romFileExtracted($event)"></age-rom-file-extractor>

        </ng-container>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeLoaderComponent {

    @Output()
    readonly loadingComplete = new EventEmitter<AgeEmulationPackage>();

    private _emGbModule?: EmGbModule;
    private _romFileToLoad?: AgeRomFileToLoad;
    private _romFileContents?: ArrayBuffer;

    constructor(private _changeDetector: ChangeDetectorRef) {
    }


    @Input()
    set romFileToLoad(romFileToLoad: AgeRomFileToLoad | undefined) {
        // stop any loading process, if the file was cleared
        this._romFileToLoad = romFileToLoad;
        this._romFileContents = undefined;
    }

    get romFileToLoad(): AgeRomFileToLoad | undefined {
        return this._romFileToLoad;
    }

    get loading(): boolean {
        return !!this._romFileToLoad && (!this._emGbModule || !this._romFileContents);
    }


    emGbModuleLoaded(emGbModule?: EmGbModule): void {
        this._emGbModule = emGbModule;
        this.checkForLoadingComplete();
    }

    romFileExtracted(romFileContents: ArrayBuffer): void {
        this._romFileContents = romFileContents;
        this.checkForLoadingComplete();
    }


    private checkForLoadingComplete(): void {
        this._changeDetector.detectChanges();
        if (!!this._emGbModule && !!this._romFileContents) {
            this.loadingComplete.emit(new AgeEmulationPackage(this._emGbModule, this._romFileContents));
        }
    }
}
