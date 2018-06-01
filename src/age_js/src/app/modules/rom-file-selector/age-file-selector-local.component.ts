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

import {ChangeDetectionStrategy, Component, ElementRef, EventEmitter, Output, ViewChild} from '@angular/core';


@Component({
    selector: 'age-file-selector-local',
    template: `
        Open local gameboy rom file:
        <input #fileInput
               type="file"
               id="fileInput"
               accept=".gb, .gbc, .zip"
               (change)="selectFile()">
    `,
    styles: [`
        input {
            margin-left: 1em;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeFileSelectorLocalComponent {

    @Output()
    readonly fileSelected = new EventEmitter<File>();

    @ViewChild('fileInput')
    private _fileInput: ElementRef;

    selectFile() {
        const files: FileList = this._fileInput.nativeElement.files;

        if (files && files.length) {
            this.fileSelected.emit(files[0]);
        }
    }
}
