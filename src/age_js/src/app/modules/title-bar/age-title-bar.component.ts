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

import {ChangeDetectionStrategy, Component, EventEmitter, Input, Output} from '@angular/core';
import {AgeEmulationRuntimeInfo} from '../../common';


export enum TitleBarButton {
    OPEN_ROM,
    INFO
}


@Component({
    selector: 'age-title-bar',
    template: `
        <div class="container age-ui">

            <div class="runtime">
                <ng-container *ngIf="runtimeInfo as info">

                    <span *ngIf="info.emulatedSeconds" title="number of emulated seconds">
                        {{info.emulatedSeconds | number:'1.0-2'}}s
                    </span>

                    <span *ngIf="info.emulationSpeed" title="current emulation speed relative to the Gameboy's speed">
                        {{percent(info.emulationSpeed) | number:'1.0-2'}}%
                    </span>

                    <span *ngIf="info.emulationMaxSpeed" title="estimated highest possible emulation speed">
                        {{percent(info.emulationMaxSpeed) | number:'1.0-2'}}%
                    </span>

                </ng-container>
            </div>

            <div class="title">
                <div class="age-ui-title">
                    AGE
                </div>
                <div *ngIf="runtimeInfo as info; else noRom"
                     title="title of the rom currently being emulated">
                    running {{info.romName}}
                </div>
                <ng-template #noRom>
                    <div>
                        no rom loaded
                    </div>
                </ng-template>
            </div>

            <div class="buttons">
                <i class="fa fa-folder-open age-ui-clickable age-ui-big-icon"
                   title="open Gameboy rom file ..."
                   (click)="buttonClicked.emit(TitleBarButton.OPEN_ROM)"></i>

                <i class="fa fa-info-circle age-ui-clickable age-ui-big-icon"
                   title="about AGE ..."
                   (click)="buttonClicked.emit(TitleBarButton.INFO)"></i>
            </div>

        </div>
    `,
    styles: [`
        .container {
            display: flex;
            align-items: center;
            width: 100%;
            font-size: small;
        }

        .container > div {
            padding: .5em;
        }

        .container > .runtime {
            flex: 1 1;
            text-align: right;
        }

        .container > .runtime > span {
            display: inline-block;
            min-width: 3em;
            margin-left: .5em;
        }

        .container > .title {
            width: 18em;
            text-align: center;
        }

        .container > .buttons {
            flex: 1 1;
            text-align: left;
        }

        .container > .buttons > i {
            margin-right: .25em;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeTitleBarComponent {

    readonly TitleBarButton = TitleBarButton;

    @Input() runtimeInfo?: AgeEmulationRuntimeInfo;

    @Output() readonly buttonClicked = new EventEmitter<TitleBarButton>();

    percent(value: number): number {
        return Math.round(value * 100);
    }
}
