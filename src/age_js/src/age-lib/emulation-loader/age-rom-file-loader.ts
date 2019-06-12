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
import * as fileType from "file-type";
import * as JSZip from "jszip";
import {from, Observable, of} from "rxjs";
import {map, switchMap} from "rxjs/operators";
import {AgeTaskStatusHandlerService} from "./age-task-status-handler.service";


export interface IAgeLocalRomFile {
    readonly type: "local-rom-file";
    readonly localFile: File;
}

export interface IAgeRomFileUrl {
    readonly type: "rom-file-url";
    readonly fileUrl: string;
}

export type TAgeRomFile = IAgeLocalRomFile | IAgeRomFileUrl;


export class AgeRomFileLoader {

    readonly supportedContentTypes: ReadonlyArray<string> = [
        "application/octet-stream",
        "application/zip",
    ];

    constructor(private readonly _httpClient: HttpClient,
                private readonly _taskStatusHandler: AgeTaskStatusHandlerService) {
    }

    loadRomFile$(romFileToLoad: TAgeRomFile): Observable<ArrayBuffer> {
        const romFile$ = (romFileToLoad.type === "local-rom-file")
            ? this._loadLocalRomFile$(romFileToLoad.localFile)
            : this._loadRomFileUrl$(romFileToLoad.fileUrl);

        return romFile$.pipe(
            switchMap(romFile => this._extractRomFile$(romFile)),
        );
    }


    private _loadLocalRomFile$(localRomFile: File): Observable<ArrayBuffer> {
        const readRomPromise = new Promise<ArrayBuffer>((resolve, reject) => {

            const fileReader = new FileReader();
            fileReader.onload = () => {
                if (!(fileReader.result instanceof ArrayBuffer)) {
                    throw new Error(`expected ArrayBuffer, found ${typeof fileReader.result}`);
                }
                resolve(fileReader.result);
            };
            fileReader.onerror = () => reject(fileReader.error);

            fileReader.readAsArrayBuffer(localRomFile);
        });

        // TODO cleanup fileReader on unsubscribe()
        //      (should be necessary only when loading is cancelled)
        return of(true).pipe(
            // add the task after subscribe() has been called
            switchMap(() => this._taskStatusHandler.addTask$("reading rom file", from(readRomPromise))),
        );
    }


    private _loadRomFileUrl$(romFileUrl: string): Observable<ArrayBuffer> {
        const loadRom$ = this._httpClient.get(
            romFileUrl,
            {
                responseType: "arraybuffer",
                observe: "response",
            },
        ).pipe(
            map(httpResponse => {
                // TODO use cors-anywhere on cors failure
                const contentType = httpResponse.headers.get("content-type") || "";
                if (this.supportedContentTypes.indexOf(contentType) < 0) {
                    throw new Error(`unknown content-type: ${contentType}`);
                }

                const fileContents = httpResponse.body;
                if (!fileContents || (fileContents.byteLength < 1)) {
                    throw new Error(`rom file is empty`);
                }

                return fileContents;
            }),
        );

        return of(true).pipe(
            // add the task after subscribe() has been called
            switchMap(() => this._taskStatusHandler.addTask$("downloading rom file", loadRom$)),
        );
    }


    private _extractRomFile$(romFile: ArrayBuffer): Observable<ArrayBuffer> {
        // If this is no zip file, there is nothing to extract
        const type = fileType(new Uint8Array(romFile));
        if (!type || (type.ext !== "zip")) {
            return of(romFile);
        }

        // If this is a zip file, look for a rom file within that zip file.
        // (fun fact: Gameboy roms are sometimes identified as MP3 files)
        return this._taskStatusHandler.addTask$(
            "extracting rom file from zip archive",
            from(extractRomFile(romFile)),
        );
    }
}


async function extractRomFile(zipFile: ArrayBuffer) {
    const zip = await JSZip.loadAsync(zipFile);

    // get a list of all rom files within that archive
    const files = zip.file(/.*(\.gb)|(\.gbc)$/i);
    if (!files || !files.length) {
        throw Error("no Gameboy rom file found");
    }

    // for deterministic rom file loading we sort the files
    // just in case there are multiple rom files available
    files.sort((a, b) => a.name.localeCompare(b.name));

    // extract the first file
    return files[0].async("arraybuffer");
}
