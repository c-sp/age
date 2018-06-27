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
    selector: 'age-open-rom-url',
    template: `
        <div class="container">
            <div>
                from URL:
            </div>

            <div>
                <input #urlInput type="url" value="http://gameboy.modermodemet.se/files/PHT-PZ.ZIP">
                <button (click)="click()">ok</button>
            </div>
        </div>
    `,
    styles: [`
        .container > div:nth-child(1) {
            padding-bottom: .5em;
        }

        .container > div:nth-child(2) {
            display: flex;
        }

        input {
            flex: 1;
            margin-right: .5em;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeOpenRomUrlComponent {

    readonly corsAnywhereUrl = 'https://cors-anywhere.herokuapp.com/';

    @Output()
    readonly openUrl = new EventEmitter<string>();

    @ViewChild('urlInput')
    private _urlInput!: ElementRef;

    click(): void {
        const url = this._urlInput.nativeElement.value;
        if (!!url) {
            this.openUrl.emit(this.corsAnywhereUrl + url);
        }
    }
}
