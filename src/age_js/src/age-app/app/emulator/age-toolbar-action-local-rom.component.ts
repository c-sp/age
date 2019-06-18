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

import {ChangeDetectionStrategy, Component, EventEmitter, Output} from "@angular/core";
import {faFolderOpen} from "@fortawesome/free-solid-svg-icons/faFolderOpen";
import {IAgeLocalRomFile} from "age-lib";


@Component({
    selector: "age-toolbar-action-local-rom",
    template: `
        <age-toolbar-action [icon]="icon" (clicked)="fileInput.click()"></age-toolbar-action>
        <input #fileInput
               type="file"
               id="fileInput"
               accept=".gb, .gbc, .zip"
               (change)="emitSelectedRom(fileInput)">
    `,
    styles: [`
        input {
            display: none;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeToolbarActionLocalRomComponent {

    readonly icon = faFolderOpen;

    @Output() readonly openLocalRom = new EventEmitter<IAgeLocalRomFile>();

    emitSelectedRom(fileInput: HTMLInputElement): void {
        const files = fileInput.files;
        if (files && files.length) {
            this.openLocalRom.emit({
                type: "local-rom-file",
                localFile: files[0],
            });
        }
    }
}
