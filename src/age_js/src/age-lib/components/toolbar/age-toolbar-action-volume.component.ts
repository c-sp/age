//
// Copyright 2019 Christoph Sprenger
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

import {ChangeDetectionStrategy, Component, EventEmitter, Input, OnInit, Output} from "@angular/core";
import {AgeBreakpointObserverService, AgeIconsService} from "../../common";


@Component({
    selector: "age-toolbar-action-volume",
    template: `
        <button mat-button (click)="toggleMute()">
            <mat-icon [svgIcon]="iconName"></mat-icon>
        </button>

        <age-toolbar-background *ngIf="breakpointObserver.hasHoverAbility$ | async">
            <mat-slider (input)="volume = $event.value"
                        [max]="maxVolume"
                        [min]="minVolume"
                        [(ngModel)]="volume"
                        [step]="1"
                        [vertical]="true"></mat-slider>
        </age-toolbar-background>
    `,
    styles: [`
        :host {
            position: relative;
        }

        age-toolbar-background {
            display: none;
            position: absolute;
            left: 0;
            top: 100%;
            padding-top: 0.5em;
            padding-bottom: 0.5em;
        }

        :host:hover age-toolbar-background {
            display: inline-block;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeToolbarActionVolumeComponent implements OnInit {

    readonly maxVolume = 100;
    readonly minVolume = 0;
    readonly defaultVolume = 20;

    @Output() readonly volumeChange = new EventEmitter<number>();

    private _volume = this.defaultVolume;
    private _volumeMuted = this._volume;

    constructor(readonly breakpointObserver: AgeBreakpointObserverService,
                private readonly _icons: AgeIconsService) {
    }

    ngOnInit(): void {
        this.volume = this.defaultVolume;
    }


    get iconName(): string {
        return this.isMuted ? this._icons.faVolumeMute : this._icons.faVolumeUp;
    }

    get isMuted(): boolean {
        return this._volume === this.minVolume;
    }

    get volume(): number {
        return this._volume;
    }


    @Input() set volume(volume: number) {
        // called by mat-slider and (maybe) parent component
        const sanitizedVolume = Math.min(this.maxVolume, Math.max(this.minVolume, volume));
        this._volumeMuted = this._volume = sanitizedVolume;
        this._nextVolumeChange();
    }

    toggleMute(): void {
        // note that we do NOT set this._volumeMuted here
        // (it contains the volume right before being muted)
        this._volume = this.isMuted
            ? ((this._volumeMuted === this.minVolume) ? this.defaultVolume : this._volumeMuted)
            : this.minVolume;
        this._nextVolumeChange();
    }


    private _nextVolumeChange(): void {
        this.volumeChange.emit(this._volume / 100);
    }
}
