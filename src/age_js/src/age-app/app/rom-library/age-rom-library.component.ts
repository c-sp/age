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

import {ChangeDetectionStrategy, Component, EventEmitter, Inject, InjectionToken, Output} from "@angular/core";
import {AgeBreakpointObserverService, AgeIconsService, IAgeLocalRomFile} from "age-lib";
import {Observable, Subject} from "rxjs";
import {AgeNavigationService, IAgeViewMode, viewMode$} from "../common";
import {IAgeOnlineRom} from "./age-rom-library-contents.component";


export const OPEN_LOCAL_ROM_FILE_SUBJECT = new InjectionToken<Subject<IAgeLocalRomFile>>("OpenLocalRomFileSubject");


@Component({
    selector: "age-rom-library",
    template: `
        <mat-toolbar *ngIf="(viewMode$ | async) as viewMode"
                     [color]="'primary'">

            <a mat-button [routerLink]="navigationService.rootUrl" [ngClass]="{'hidden': !viewMode.useMobileView}">
                <mat-icon [svgIcon]="icons.faAngleLeft"></mat-icon>
            </a>
            <age-toolbar-spacer></age-toolbar-spacer>

            <age-toolbar-action-local-rom (openLocalRom)="openLocalRom($event, viewMode)">
            </age-toolbar-action-local-rom>

            <age-toolbar-spacer></age-toolbar-spacer>
            <a mat-button [routerLink]="navigationService.rootUrl" [ngClass]="{'hidden': viewMode.useMobileView}">
                <mat-icon [svgIcon]="icons.faTimes"></mat-icon>
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

    readonly viewMode$: Observable<IAgeViewMode>;

    @Output() readonly openLocalRomFile = new EventEmitter<IAgeLocalRomFile>();

    constructor(readonly navigationService: AgeNavigationService,
                readonly icons: AgeIconsService,
                @Inject(OPEN_LOCAL_ROM_FILE_SUBJECT) private readonly _localRomFileSubject: Subject<IAgeLocalRomFile>,
                breakpointObserverService: AgeBreakpointObserverService) {

        this.viewMode$ = viewMode$(breakpointObserverService);
    }

    openRomUrl(onlineRom: IAgeOnlineRom): void {
        this.navigationService.navigateToOpenRomUrl(onlineRom.romUrl);
    }

    openLocalRom(localRom: IAgeLocalRomFile, viewMode: IAgeViewMode): void {
        this._localRomFileSubject.next(localRom);
        // navigate to the emulation, if it's not yet visible
        if (viewMode.useMobileView) {
            this.navigationService.navigateToRoot();
        }
    }
}
