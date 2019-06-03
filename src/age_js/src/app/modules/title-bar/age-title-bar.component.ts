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
import {faFolderOpen} from "@fortawesome/free-solid-svg-icons/faFolderOpen";
import {faInfoCircle} from "@fortawesome/free-solid-svg-icons/faInfoCircle";
import {IAgeEmulationRuntimeInfo} from "../../common";


export enum TitleBarButton {
    OPEN_ROM = "OPEN_ROM",
    INFO = "INFO",
}


@Component({
    selector: "age-title-bar",
    template: `
        <div class="container age-ui">

            <div>
                <!-- inner div for achieving layout symmetry with width:100% -->
                <div class="runtime">
                    <ng-container *ngIf="runtimeInfo as info">

                    <span *ngIf="info.emulatedSeconds"
                          title="Number of emulated seconds">
                        {{info.emulatedSeconds | number:'1.1-1'}}s
                    </span>

                        <span *ngIf="info.emulationSpeed"
                              title="Current emulation speed
(100% being the Gameboy's actual speed)">
                        {{info.emulationSpeed}}%
                    </span>

                        <span *ngIf="info.emulationMaxSpeed"
                              title="Estimated highest possible emulation speed achievable on this device
(100% being the Gameboy's actual speed)">
                        {{info.emulationMaxSpeed}}%
                    </span>

                    </ng-container>
                </div>
            </div>

            <div class="title">
                <div class="age-ui-title">
                    AGE
                </div>
                <div *ngIf="runtimeInfo as info; else noRom"
                     title="Title of the rom currently being emulated">
                    running {{info.romName}}
                </div>
                <ng-template #noRom>
                    <div>
                        no rom loaded
                    </div>
                </ng-template>
            </div>

            <div>
                <!-- inner div for achieving layout symmetry with width:100% -->
                <div class="buttons">
                    <fa-icon [icon]="faFolderOpen" class="age-ui-clickable age-ui-big-icon icon"
                             title="Open a Gameboy rom file ..."
                             (click)="buttonClicked.emit(TitleBarButton.OPEN_ROM)"></fa-icon>

                    <fa-icon [icon]="faInfoCircle" class="age-ui-clickable age-ui-big-icon icon"
                             title="About AGE ..."
                             (click)="buttonClicked.emit(TitleBarButton.INFO)"></fa-icon>
                </div>
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

        .container > :nth-child(1) {
            flex: 1 1;
        }

        .container > :nth-child(3) {
            flex: 1 1;
        }

        .runtime {
            width: 100%;
            text-align: right;
        }

        .runtime > span {
            display: inline-block;
            min-width: 3em;
            margin-left: .5em;
        }

        .title {
            width: 18em;
            text-align: center;
        }

        .buttons {
            width: 100%;
            text-align: left;
        }

        .icon {
            margin-right: .25em;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeTitleBarComponent {

    readonly TitleBarButton = TitleBarButton;
    readonly faFolderOpen = faFolderOpen;
    readonly faInfoCircle = faInfoCircle;

    @Output() readonly buttonClicked = new EventEmitter<TitleBarButton>();

    private _runtimeInfo?: IAgeEmulationRuntimeInfo;

    get runtimeInfo(): IAgeEmulationRuntimeInfo | undefined {
        return this._runtimeInfo;
    }

    @Input()
    set runtimeInfo(info: IAgeEmulationRuntimeInfo | undefined) {
        this._runtimeInfo = !info ? undefined : {
            romName: info.romName,
            emulatedSeconds: info.emulatedSeconds && Math.round(info.emulatedSeconds * 10) / 10,
            emulationSpeed: info.emulationSpeed && Math.round(info.emulationSpeed * 100),
            emulationMaxSpeed: info.emulationMaxSpeed && Math.round(info.emulationMaxSpeed * 100),
        };
    }
}
