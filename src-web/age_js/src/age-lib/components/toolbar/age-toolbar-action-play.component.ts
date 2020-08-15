//
// Copyright 2020 Christoph Sprenger
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

import {ChangeDetectionStrategy, Component, EventEmitter, Input, Output} from '@angular/core';
import {AgeIconsService, assertNever} from '../../common';


export enum AgePlayPauseStatus {
    DISABLED = 'DISABLED',
    IS_PLAYING = 'IS_PLAYING',
    IS_PAUSED = 'IS_PAUSED',
}


@Component({
    selector: 'age-toolbar-action-play',
    template: `
        <button mat-button
                [disabled]="disabled"
                (click)="paused.emit(!isPaused)">

            <mat-icon [svgIcon]="iconName"></mat-icon>

        </button>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeToolbarActionPlayComponent {

    @Output() readonly paused = new EventEmitter<boolean>();

    private _disabled = false;
    private _isPaused = false;

    constructor(private readonly _icons: AgeIconsService) {
    }

    get disabled(): boolean {
        return this._disabled;
    }

    get iconName(): string {
        return this.isPaused ? this._icons.faPlay : this._icons.faPause;
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
