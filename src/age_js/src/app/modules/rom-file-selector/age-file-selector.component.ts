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

import {ChangeDetectionStrategy, Component, EventEmitter, Output} from '@angular/core';
import {AgeRomFileToLoad} from '../common/age-rom-file-to-load';


@Component({
    selector: 'age-file-selector',
    template: `
        <div>
            <age-file-selector-local (fileSelected)="selectLocalFile($event)"></age-file-selector-local>
        </div>

        <age-file-selector-url (urlSelected)="selectFileFromURL($event)"></age-file-selector-url>
    `,
    styles: [`
        div {
            margin-bottom: .5em;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeFileSelectorComponent {

    @Output()
    readonly fileSelected = new EventEmitter<AgeRomFileToLoad>();

    selectLocalFile(file: File) {
        this.fileSelected.emit(AgeRomFileToLoad.forLocalFile(file));
    }

    selectFileFromURL(url: string) {
        this.fileSelected.emit(AgeRomFileToLoad.forUrl(url));
    }
}
