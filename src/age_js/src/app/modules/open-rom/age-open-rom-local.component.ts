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
    selector: 'age-open-rom-local',
    template: `
        <!-- the div is used to limit the size of the label -->
        <div>
            <label for="fileInput"
                   class="age-ui-clickable"
                   title="Open a Gameboy rom file on the local device.
The rom file is not being uploaded anywhere, it will not leave your device.">

                <i class="fa fa-folder-open age-ui-big-icon"></i>
                <span>open local file</span>

            </label>
        </div>

        <input #fileInput
               type="file"
               id="fileInput"
               accept=".gb, .gbc, .zip"
               (change)="emitSelectedFile()">
    `,
    styles: [`
        div {
            display: flex;
        }

        label {
            display: flex;
            align-items: center;
        }

        label i {
            margin-right: .25em;
        }

        input {
            display: none;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeOpenRomLocalComponent {

    @Output() readonly openFile = new EventEmitter<File>();

    @ViewChild('fileInput') private _fileInput!: ElementRef;

    emitSelectedFile() {
        const files: FileList = this._fileInput.nativeElement.files;

        if (files && files.length) {
            this.openFile.emit(files[0]);
        }
    }
}
