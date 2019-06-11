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

import {ChangeDetectionStrategy, Component, ElementRef, EventEmitter, Output, ViewChild} from "@angular/core";
import {faGlobeAfrica} from "@fortawesome/free-solid-svg-icons/faGlobeAfrica";


@Component({
    selector: "age-open-rom-url",
    template: `
        <div>
            <span [class.age-ui-clickable]="!urlHint"
                  title="Open the Gameboy rom file at the specified URL.
This will use a proxy host to circumvent possible Cross Origin Resource Sharing (CORS) issues."
                  (click)="emitValidUrl()"
                  (keypress)="emitValidUrl()">

                <fa-icon [icon]="faGlobeAfrica" class="age-ui-big-icon icon"></fa-icon>
                open URL
            </span>

            <span *ngIf="urlHint as hint" class="age-ui-error">
                <ng-container [ngSwitch]="hint">

                    <ng-container *ngSwitchCase="urlEmpty">
                        (no URL entered)
                    </ng-container>

                    <ng-container *ngSwitchCase="urlInvalid">
                        (URL invalid)
                    </ng-container>

                </ng-container>
            </span>
        </div>

        <div>
            <!--
             we subscribe to the "input" event with a no-op to automatically trigger the Angular change detection
             when the input's value changes
              -->
            <input #urlInput
                   type="url"
                   class="age-ui-input"
                   placeholder="please enter URL"
                   list="suggestedURLs"
                   (input)="urlHint">

            <datalist id="suggestedURLs">

                <option label="(Demo) Back To Color"
                        value="https://github.com/AntonioND/back-to-color/archive/v1.0.zip">

                <option label="(Demo) Cenotaph"
                        value="https://files.scene.org/get/parties/2012/revision12/oldskool_demo/dcs-ctph.zip">

                <option label="(Demo) Demotronic"
                        value="http://lcp.c64.org/files/2002-LCP/compo/demos/mb-dtrnc.zip">

                <option label="(Demo) It Came from Planet Zilog"
                        value="http://gameboy.modermodemet.se/files/PHT-PZ.ZIP">

                <option label="(Demo) Mental Respirator"
                        value="https://files.scene.org/get/parties/2005/breakpoint05/wild/pht-mr.zip">

                <option label="(Demo) Oh!"
                        value="http://www.nordloef.com/gbdemos/oh.zip">

                <option label="(Demo) Pickpocket"
                        value="http://www.izik.se/files/demos/inka-PP.zip">

            </datalist>
        </div>
    `,
    styles: [`
        :nth-child(1) {
            margin-bottom: .1em;
        }

        :nth-child(1) > :nth-child(2) {
            margin-left: .5em;
            font-size: smaller;
        }

        input {
            width: 100%;
        }

        .icon {
            margin-right: .25em;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeOpenRomUrlComponent {

    readonly faGlobeAfrica = faGlobeAfrica;

    readonly urlEmpty = "urlEmpty";
    readonly urlInvalid = "urlInvalid";

    @Output() readonly openUrl = new EventEmitter<string>();

    @ViewChild("urlInput", {static: false}) private _urlInput?: ElementRef;

    get urlHint(): string {
        if (!this._urlInput || !this._urlInput.nativeElement.value) {
            return this.urlEmpty;
        }
        if (!this._urlInput || !this._urlInput.nativeElement.checkValidity()) {
            return this.urlInvalid;
        }
        return "";
    }

    emitValidUrl(): void {
        if (this._urlInput && !this.urlHint) {
            // TODO move cors-anywhere call to age-lib & make it optional
            const url = `https://cors-anywhere.herokuapp.com/${this._urlInput.nativeElement.value}`;
            this.openUrl.emit(url);
        }
    }
}
