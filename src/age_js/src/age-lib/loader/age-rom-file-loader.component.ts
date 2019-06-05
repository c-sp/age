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

import {HttpClient, HttpErrorResponse} from "@angular/common/http";
import {
    ChangeDetectionStrategy,
    ChangeDetectorRef,
    Component,
    EventEmitter,
    Input,
    OnDestroy,
    Output,
} from "@angular/core";
import {AgeRomFileToLoad} from "../common";
import {AgeLoaderState} from "./age-loader-state.component";


@Component({
    selector: "age-rom-file-loader",
    template: `
        <age-loader-state [state]="fileLoaderState">
            <ng-container ageLoaderWorking>loading rom file ...</ng-container>
            <ng-container ageLoaderSuccess>rom file loaded</ng-container>
            <ng-container ageLoaderError>error loading rom file</ng-container>
        </age-loader-state>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeRomFileLoaderComponent implements OnDestroy {

    readonly supportedContentTypes = [
        "application/octet-stream",
        "application/zip",
    ];

    @Output() readonly fileLoaded = new EventEmitter<ArrayBuffer>();

    private _loaderState?: AgeLoaderState;
    private _fileReader?: FileReader;

    constructor(private _httpClient: HttpClient,
                private _changeDetector: ChangeDetectorRef) {
    }

    ngOnDestroy(): void {
        this.cleanupReader();
    }


    get fileLoaderState(): AgeLoaderState | undefined {
        return this._loaderState;
    }

    @Input() set romFileToLoad(romFileToLoad: AgeRomFileToLoad) {
        this.cleanupReader(); // stop the current loader before starting a new one
        this._loaderState = undefined;

        if (romFileToLoad.file) {
            this.loadLocalFile(romFileToLoad.file);
            this._loaderState = AgeLoaderState.WORKING;

        } else if (romFileToLoad.url) {
            this.loadFileFromUrl(romFileToLoad.url);
            this._loaderState = AgeLoaderState.WORKING;
        }
    }


    private loadLocalFile(file: File) {
        const fileReader = this._fileReader = new FileReader();

        fileReader.onload = () => {
            if (!(fileReader.result instanceof ArrayBuffer)) {
                throw new Error(`expected ArrayBuffer, found ${typeof fileReader.result}`);
            }
            this.loaderFinished(fileReader.result);
        };
        fileReader.onerror = () => this.loaderError();

        fileReader.readAsArrayBuffer(file);
    }

    private loadFileFromUrl(url: string) {
        this._httpClient.get(
            url,
            {
                responseType: "arraybuffer",
                observe: "response",
            },
        ).subscribe(
            httpResponse => {
                const contentType = httpResponse.headers.get("content-type") || "";
                if (this.supportedContentTypes.indexOf(contentType) < 0) {
                    throw new Error(`unknown content-type: ${contentType}`);
                }

                const fileContents = httpResponse.body;
                if (fileContents) {
                    this.loaderFinished(fileContents);
                } else {
                    this.loaderError();
                }
            },
            (httpErrorResponse: HttpErrorResponse) => {
                httpErrorResponse.headers.keys().map(key => {
                    console.log(`header ${key}: ${httpErrorResponse.headers.get(key)}`);
                });
                console.log(`error reading url ${url}`, httpErrorResponse);
                this.loaderError();
            },
        );
    }


    private loaderFinished(fileContents: ArrayBuffer): void {
        this.setLoaderState(AgeLoaderState.SUCCESS);
        this.fileLoaded.emit(fileContents);
    }

    private loaderError(): void {
        this.setLoaderState(AgeLoaderState.ERROR);
    }

    private setLoaderState(loaderState: AgeLoaderState) {
        this._loaderState = loaderState;
        this._changeDetector.markForCheck();
    }

    private cleanupReader(): void {
        if (this._fileReader) {

            // readyState LOADING = 1
            if (this._fileReader.readyState === 1) {
                this._fileReader.abort();
            }

            this._fileReader = undefined;
        }
    }
}
