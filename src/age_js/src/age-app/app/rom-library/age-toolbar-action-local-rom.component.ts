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
import {AgeIconsService, IAgeLocalRomFile} from "age-lib";


@Component({
    selector: "age-toolbar-action-local-rom",
    template: `
        <button mat-button (click)="fileInput.click()">
            <mat-icon [svgIcon]="deviceIcon"></mat-icon>
        </button>

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

    readonly deviceIcon: string;

    @Output() readonly openLocalRom = new EventEmitter<IAgeLocalRomFile>();

    constructor(icons: AgeIconsService) {
        // we need this only to show different icons for different device types
        // so there is no need to be 100% accurate
        const isMobile = !!navigator.userAgent.match(
            /(iPhone|iPod|iPad|Android|webOS|BlackBerry|IEMobile|Opera Mini)/i,
        );

        this.deviceIcon = isMobile ? icons.faMobileAlt : icons.faDesktop;
    }

    emitSelectedRom(fileInput: HTMLInputElement): void {
        const files = fileInput.files;
        if (files && files.length) {
            this.openLocalRom.emit({
                type: "local-rom-file",
                localFile: files[0],
            });
            // clear the value,
            // so that the change-event is fired if the user selects the same file again
            fileInput.value = "";
        }
    }
}
