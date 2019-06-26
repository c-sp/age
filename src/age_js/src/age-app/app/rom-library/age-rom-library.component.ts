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
import {faAngleLeft} from "@fortawesome/free-solid-svg-icons/faAngleLeft";
import {faTimes} from "@fortawesome/free-solid-svg-icons/faTimes";
import {IAgeLocalRomFile} from "age-lib";
import {AgeNavigationService} from "../routing";
import {IAgeOnlineRom} from "./age-rom-library-contents.component";


@Component({
    selector: "age-rom-library",
    template: `
        <mat-toolbar [color]="'primary'">

            <a mat-button [routerLink]="navigationService.rootUrl" [ngClass]="{'hidden': !mobileMode}">
                <fa-icon [icon]="iconBackToEmulation"></fa-icon>
            </a>

            <age-toolbar-spacer></age-toolbar-spacer>

            <age-toolbar-action-local-rom (openLocalRom)="openLocalRom($event)"></age-toolbar-action-local-rom>

            <age-toolbar-spacer></age-toolbar-spacer>

            <a mat-button [routerLink]="navigationService.rootUrl" [ngClass]="{'hidden': mobileMode}">
                <fa-icon [icon]="iconCloseRomLibrary"></fa-icon>
            </a>

        </mat-toolbar>

        <age-rom-library-contents (romClicked)="openRomUrl($event)"
                                  [justifyContent]="'center'"></age-rom-library-contents>
    `,
    styles: [`
        :host {
            display: block;
            max-width: 60em;
            margin-left: auto;
            margin-right: auto;
        }

        mat-toolbar {
            position: sticky;
            top: 0;
        }

        .hidden {
            visibility: hidden;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeRomLibraryComponent {

    readonly iconBackToEmulation = faAngleLeft;
    readonly iconCloseRomLibrary = faTimes;

    @Input() mobileMode = false;
    @Output() readonly openLocalRomFile = new EventEmitter<IAgeLocalRomFile>();

    constructor(readonly navigationService: AgeNavigationService) {
    }

    openRomUrl(onlineRom: IAgeOnlineRom): void {
        this.navigationService.navigateToOpenRomUrl(onlineRom.romUrl);
    }

    openLocalRom(localRom: IAgeLocalRomFile): void {
        this.openLocalRomFile.emit(localRom);
        if (this.mobileMode) {
            this.navigationService.navigateToRoot();
        }
    }
}
