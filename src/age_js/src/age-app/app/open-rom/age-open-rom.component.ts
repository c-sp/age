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
import {faInfoCircle} from "@fortawesome/free-solid-svg-icons/faInfoCircle";
import {AgeRomFileToLoad} from "age-lib";


@Component({
    selector: "age-open-rom",
    template: `
        <div class="container age-ui">

            <div>
                <!-- inner span for css class "age-ui-title" to prevent differently sized div margins -->
                <span class="age-ui-title">Open Gameboy rom file</span>
            </div>

            <div class="info">
                <fa-icon [icon]="faInfoCircle" class="icon"></fa-icon>
                <div>
                    AGE can load Gameboy rom files <span class="age-ui-highlight">(*.gb)</span>,
                    Gameboy Color rom files <span class="age-ui-highlight">(*.gbc)</span>
                    and zip files <span class="age-ui-highlight">(*.zip)</span>
                    containing such a rom file.
                </div>
            </div>

            <div>
                <age-open-rom-local (openFile)="selectLocalFile($event)"></age-open-rom-local>
            </div>

            <div>
                <age-open-rom-url (openUrl)="selectFileFromURL($event)"></age-open-rom-url>
            </div>

        </div>
    `,
    styles: [`
        .container {
            padding: 2em;
            opacity: 0.95;
        }

        .container > :nth-child(1) {
            text-align: center;
        }

        .container > div:nth-last-child(n+2) {
            margin-bottom: 2em;
        }

        .info {
            display: flex;
        }

        .info .icon {
            margin-right: .5em;
        }

        .info span {
            font-family: monospace;
            font-weight: bold;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeOpenRomComponent {

    readonly faInfoCircle = faInfoCircle;

    @Output() readonly openRom = new EventEmitter<AgeRomFileToLoad>();

    selectLocalFile(file: File) {
        this.openRom.emit(AgeRomFileToLoad.forLocalFile(file));
    }

    selectFileFromURL(url: string) {
        this.openRom.emit(AgeRomFileToLoad.forUrl(url));
    }
}
