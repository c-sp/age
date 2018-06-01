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
    selector: 'age-file-selector-url',
    template: `
        Open gameboy rom file from URL:
        <!--<input #urlInput type="url" value="https://raw.githubusercontent.com/mills32/CUTE_DEMO/master/0_rom/CUTEDEMO.gbc">-->
        <input #urlInput type="url" value="http://gameboy.modermodemet.se/files/PHT-PZ.ZIP">
        <!--<input #urlInput type="url" value="http://www.pouet.net/prod.php?which=73290">-->
        <button (click)="openUrl()">open</button>
    `,
    styles: [`
        input {
            margin-left: 1em;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeFileSelectorURLComponent {

    readonly corsAnywhereUrl = 'https://cors-anywhere.herokuapp.com/';

    @Output()
    readonly urlSelected = new EventEmitter<string>();

    @ViewChild('urlInput')
    private _urlInput: ElementRef;

    openUrl(): void {
        const url = this._urlInput.nativeElement.value;
        if (!!url) {
            this.urlSelected.emit(this.corsAnywhereUrl + url);
        }
    }
}
