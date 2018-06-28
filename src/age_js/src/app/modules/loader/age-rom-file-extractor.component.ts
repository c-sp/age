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
import {AgeLoaderState} from './age-loader-state.component';
import * as FileType from 'file-type';
import * as JSZip from 'jszip';


@Component({
    selector: 'age-rom-file-extractor',
    template: `
        <age-loader-state [state]="fileExtractorState">
            <ng-container ageLoaderWorking>extracting rom file ...</ng-container>
            <ng-container ageLoaderSuccess>rom file extracted</ng-container>
            <ng-container ageLoaderError>error extracting rom file</ng-container>
        </age-loader-state>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeRomFileExtractorComponent {

    private static async extractRomFile(fileContents: ArrayBuffer) {
        const zip = await JSZip.loadAsync(fileContents);

        // get a list of all rom files within that archive
        const files = zip.file(/.*(\.gb)|(\.gbc)$/i);
        if (!files || !files.length) {
            throw Error('no Gameboy rom file found');
        }

        // sort the files in case there are multiple rom files available
        // (deterministic rom file loading)
        files.sort((a, b) => {
            return a.name.localeCompare(b.name);
        });

        // extract the first file
        return files[0].async('arraybuffer');
    }


    @Output()
    readonly fileExtracted = new EventEmitter<ArrayBuffer>(true); // must be async as we call emit() during change detection at times

    private _extractorState?: AgeLoaderState;

    constructor(private _changeDetector: ChangeDetectorRef) {
    }


    get fileExtractorState(): AgeLoaderState | undefined {
        return this._extractorState;
    }

    @Input()
    set fileContents(fileContents: ArrayBuffer | undefined) {
        this._extractorState = undefined;
        if (fileContents) {

            // If this is a zip file, look for a rom file within that zip file.
            // Otherwise we just pass on the file contents.
            // (funnily enough Gameboy roms are sometimes identified as MP3 files)
            const type = FileType(new Uint8Array(fileContents));
            if (type && (type.ext === 'zip')) {
                this._extractorState = AgeLoaderState.WORKING;

                AgeRomFileExtractorComponent.extractRomFile(fileContents).then(
                    extractedFile => {
                        this.setExtractorState(AgeLoaderState.SUCCESS);
                        this.fileExtracted.emit(extractedFile);
                    },
                    error => {
                        console.error(error);
                        this.setExtractorState(AgeLoaderState.ERROR);
                    }
                );

            } else {
                // don't set any loader state, as we don't really do anything here
                this.fileExtracted.emit(fileContents);
            }
        }
    }


    private setExtractorState(extractorState: AgeLoaderState): void {
        this._extractorState = extractorState;
        this._changeDetector.detectChanges();
    }
}
