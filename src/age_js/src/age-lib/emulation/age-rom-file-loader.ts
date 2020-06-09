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

import {HttpClient, HttpErrorResponse} from '@angular/common/http';
import * as JSZip from 'jszip';
import {from, Observable, of} from 'rxjs';
import {catchError, map, switchMap} from 'rxjs/operators';
import {AgeTaskStatusHandler} from './age-task-status-handler';


export interface IAgeLocalRomFile {
    readonly type: 'local-rom-file';
    readonly localFile: File;
}

export interface IAgeRomFileUrl {
    readonly type: 'rom-file-url';
    readonly fileUrl: string;
}

export type TAgeRomFile = IAgeLocalRomFile | IAgeRomFileUrl;


export class AgeRomFileLoader {

    constructor(private readonly _httpClient: HttpClient,
                private readonly _taskStatusHandler: AgeTaskStatusHandler) {
    }

    loadRomFile$(romFileToLoad: TAgeRomFile): Observable<ArrayBuffer> {
        const romFile$ = (romFileToLoad.type === 'local-rom-file')
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
            switchMap(() => this._taskStatusHandler.addTask$('reading rom file', from(readRomPromise))),
        );
    }


    private _loadRomFileUrl$(romFileUrl: string, isCorsRetry = false): Observable<ArrayBuffer> {
        const loadRom$ = this._httpClient.get(
            romFileUrl,
            {
                responseType: 'arraybuffer',
                observe: 'response',
            },
        ).pipe(
            map(httpResponse => {
                // We don't care for the content-type here, because:
                //  - we detect zip files by magic number using the file-type library
                //  - depending on the server configuration the content-type is simply unusable
                //    (e.g. text/plain for binary data)
                const fileContents = httpResponse.body;
                if (!fileContents || (fileContents.byteLength < 1)) {
                    throw new Error(`rom file is empty`);
                }
                return fileContents;
            }),
        );

        return of(true).pipe(
            // add the task after subscribe() has been called
            switchMap(() => this._taskStatusHandler.addTask$(
                isCorsRetry ? 'downloading rom file (CORS Anywhere)' : 'downloading rom file',
                loadRom$,
            )),

            // CORS Anywhere
            catchError((errorResponse: HttpErrorResponse) => {
                // try CORS Anywhere, if this might be a CORS error
                // (no indication besides status 0)
                if (!isCorsRetry && (errorResponse.status === 0)) {
                    return this._loadRomFileUrl$(`https://cors-anywhere.herokuapp.com/${romFileUrl}`, true);
                }
                // else just throw the error
                throw errorResponse;
            }),
        );
    }


    private _extractRomFile$(romFile: ArrayBuffer): Observable<ArrayBuffer> {
        // I had too many issues with https://github.com/sindresorhus/file-type
        // inside a browser for detecting zip files.
        // Instead we rely on JSZip for detecting a zip file and treat a file
        // as unzipped rom file if JSZip fails.
        return from(tryOpenZipFile(romFile)).pipe(

            switchMap(jsZip => {
                // If this is no zip file, there is nothing to extract
                if (!jsZip) {
                    return of(romFile);
                }

                // If this is a zip file, look for a rom file within that zip file.
                // (fun fact: Gameboy roms are sometimes identified as MP3 files)
                return this._taskStatusHandler.addTask$(
                    'extracting rom file from zip archive',
                    from(extractRomFile(jsZip)),
                );
            }),
        );
    }
}


async function tryOpenZipFile(file: ArrayBuffer): Promise<JSZip | undefined> {
    let jsZip: JSZip;
    try {
        jsZip = await JSZip.loadAsync(file);

    } catch (err) {
        // this is no valid zip file
        return undefined;
    }
    return jsZip;
}

async function extractRomFile(jsZip: JSZip): Promise<ArrayBuffer> {
    // get a list of all rom files within that archive
    const files = jsZip.file(/.*(\.gb)|(\.gbc)|(\.cgb)$/i);
    if (!files || !files.length) {
        throw Error('no Gameboy rom file found');
    }

    // for deterministic rom file loading we sort the files
    // just in case there are multiple rom files available
    files.sort((a, b) => a.name.localeCompare(b.name));

    // extract the first file
    return files[0].async('arraybuffer');
}
