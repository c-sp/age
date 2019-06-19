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

import {ChangeDetectionStrategy, Component, Input} from "@angular/core";
import {IconDefinition} from "@fortawesome/fontawesome-svg-core";
import {faVolumeMute} from "@fortawesome/free-solid-svg-icons/faVolumeMute";
import {faVolumeUp} from "@fortawesome/free-solid-svg-icons/faVolumeUp";


// TODO no hover on mobile devices => hide slider, just display mute/unmute button

@Component({
    selector: "age-toolbar-action-volume",
    template: `
        <age-toolbar-action [icon]="icon"></age-toolbar-action>
        <mat-slider [min]="0"
                    [max]="100"
                    [step]="1"></mat-slider>
    `,
    styles: [`
        :host {
            position: relative;
        }

        mat-slider {
            display: none;

            position: absolute;
            left: 100%;
            top: 50%;
            transform: translateY(-50%);

            /* override default mat-slider min-width */
            min-width: 70px;
            width: 70px;
        }

        :host:hover mat-slider {
            display: inline-block;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeToolbarActionVolumeComponent {

    private _icon = faVolumeUp;


    get icon(): IconDefinition {
        return this._icon;
    }

    @Input() set isMuted(isMuted: boolean) {
        this._icon = isMuted ? faVolumeMute : faVolumeUp;
    }
}
