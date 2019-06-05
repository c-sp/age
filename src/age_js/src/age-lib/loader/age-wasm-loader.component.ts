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
import {
    ChangeDetectionStrategy,
    ChangeDetectorRef,
    Component,
    EventEmitter,
    Input,
    OnDestroy,
    OnInit,
    Output,
} from "@angular/core";
import {combineLatest, from, of} from "rxjs";
import {switchMap, tap} from "rxjs/operators";
import {IEmGbModule} from "../common";
import {AgeLoaderState} from "./age-loader-state.component";


@Component({
    selector: "age-wasm-loader",
    template: `
        <ng-container *ngIf="showState">

            <age-loader-state [state]="javascriptLoadingState">
                <ng-container ageLoaderWorking>loading Javascript ...</ng-container>
                <ng-container ageLoaderSuccess>Javascript loaded</ng-container>
                <ng-container ageLoaderError>error loading Javascript</ng-container>
            </age-loader-state>

            <age-loader-state [state]="runtimeInitState">
                <ng-container ageLoaderNoState>WebAssembly: waiting for Javascript</ng-container>
                <ng-container ageLoaderWorking>initializing WebAssembly ...</ng-container>
                <ng-container ageLoaderSuccess>WebAssembly initialized</ng-container>
                <ng-container ageLoaderError>error initializing WebAssembly</ng-container>
            </age-loader-state>

        </ng-container>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeWasmLoaderComponent implements OnInit, OnDestroy {

    @Input() showState = false;

    @Output() readonly emGbModuleLoaded = new EventEmitter<IEmGbModule | undefined>();

    private _javascriptLoadingState?: AgeLoaderState;
    private _runtimeInitState?: AgeLoaderState;
    private _wasmModule?: IEmGbModule;


    constructor(private _changeDetector: ChangeDetectorRef,
                private _httpClient: HttpClient) {
    }

    ngOnInit(): void {
        // We rely on dynamic imports to lazy load the JavaScript generated by emscripten.
        // "webpackIgnore" is used to prevent webpack from creating a chunk for this JavaScript
        // (which would require the file to exist of course,
        // which in turn means we cannot parallelize the age-wasm and the age-js build jobs).
        //
        // One should obviously not activate the TypeScript Compiler option "removeComments"
        // since this will break webpack Magic Comments.

        // tslint:disable-next-line:no-any
        const importJs = import(/* webpackIgnore: true */ "/assets/age_wasm.js" as any);

        of(true)
            .pipe(
                tap(() => this._javascriptLoadingState = this._updateView(AgeLoaderState.WORKING)),
                switchMap(() => combineLatest([
                    from(importJs),
                    this._httpClient.get("assets/age_wasm.wasm", {responseType: "arraybuffer"}),
                ])),
                tap(values => {
                    this._javascriptLoadingState = this._updateView(AgeLoaderState.SUCCESS);
                    this._runtimeInitState = this._updateView(AgeLoaderState.WORKING);
                    const wasmModule = values[0].default({
                        wasmBinary: new Uint8Array(values[1]),
                        onAbort: () => {
                            this._runtimeInitState = this._updateView(AgeLoaderState.ERROR);
                            this.emGbModuleLoaded.emit(undefined);
                        },
                        onRuntimeInitialized: () => {
                            this._runtimeInitState = this._updateView(AgeLoaderState.SUCCESS);
                            this.emGbModuleLoaded.emit(this._wasmModule = wasmModule);
                        },
                    });
                }),
            )
            .subscribe();
    }

    ngOnDestroy(): void {
        if (this._wasmModule) {
            // tslint:disable-next-line:no-any
            const wasmModule: any = this._wasmModule;

            wasmModule.noExitRuntime = false;
            try {
                wasmModule.exit(0);
            } catch (err) {
                // ignore emscripten cleanup errors
            }
        }
    }


    get javascriptLoadingState(): AgeLoaderState | undefined {
        return this._javascriptLoadingState;
    }

    get runtimeInitState(): AgeLoaderState | undefined {
        return this._runtimeInitState;
    }


    private _updateView<T>(value: T): T {
        this._changeDetector.markForCheck();
        return value;
    }
}
