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

import {ChangeDetectionStrategy, Component, EventEmitter, Input, Output} from "@angular/core";
import {IconDefinition} from "@fortawesome/fontawesome-svg-core";
import {faPause} from "@fortawesome/free-solid-svg-icons/faPause";
import {faPlay} from "@fortawesome/free-solid-svg-icons/faPlay";
import {assertNever} from "../../common";


export enum AgePlayPauseStatus {
    DISABLED = "DISABLED",
    IS_PLAYING = "IS_PLAYING",
    IS_PAUSED = "IS_PAUSED",
}


@Component({
    selector: "age-toolbar-action-play",
    template: `
        <age-toolbar-action [disabled]="disabled"
                            [icon]="icon"
                            (clicked)="paused.emit(!isPaused)"></age-toolbar-action>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeToolbarActionPlayComponent {

    @Output() readonly paused = new EventEmitter<boolean>();

    private _disabled = false;
    private _isPaused = false;

    get disabled(): boolean {
        return this._disabled;
    }

    get icon(): IconDefinition {
        return this.isPaused ? faPlay : faPause;
    }

    get isPaused(): boolean {
        return this._isPaused;
    }

    @Input() set playPauseStatus(playPauseStatus: AgePlayPauseStatus) {
        switch (playPauseStatus) {
            case AgePlayPauseStatus.DISABLED:
            case AgePlayPauseStatus.IS_PAUSED:
                this._isPaused = true;
                break;
            case AgePlayPauseStatus.IS_PLAYING:
                this._isPaused = false;
                break;
            default:
                assertNever(playPauseStatus);
                break;
        }
        this._disabled = playPauseStatus === AgePlayPauseStatus.DISABLED;
    }
}
