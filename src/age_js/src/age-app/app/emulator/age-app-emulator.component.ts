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
import {AgeIconsService, TAgeRomFile} from "age-lib";
import {Observable, Subject} from "rxjs";
import {AgeNavigationService} from "../routing";


@Component({
    selector: "age-app-emulator",
    template: `
        <mat-menu #ageMenu="matMenu"
                  [xPosition]="'before'">

            <a mat-menu-item [routerLink]="navigationService.libraryUrl">
                <mat-icon [svgIcon]="icons.ageCartridge"></mat-icon>
                open rom file
            </a>

            <button mat-menu-item>
                <mat-icon [svgIcon]="icons.faInfoCircle"></mat-icon>
                about AGE
            </button>

            <a mat-menu-item
               href="https://gitlab.com/csprenger/AGE"
               target="_blank"
               rel="noopener nofollow noreferrer"
               title="Link to the AGE git repository">

                <mat-icon [svgIcon]="icons.faGitlab"></mat-icon>
                AGE repository
            </a>

        </mat-menu>


        <age-emulator-container [forcePause]="forcePause"
                                [romFile]="romFile$ | async">

            <age-toolbar-spacer></age-toolbar-spacer>

            <button mat-button [matMenuTriggerFor]="ageMenu">
                <mat-icon [svgIcon]="icons.faEllipsisV"></mat-icon>
            </button>

        </age-emulator-container>
    `,
    styles: [`
        :host {
            display: block;
        }

        age-emulator-container {
            height: 100%;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeAppEmulatorComponent {

    @Input() forcePause = false;

    private readonly _romFileSubject = new Subject<TAgeRomFile>();
    private readonly _romFile$ = this._romFileSubject.asObservable();

    constructor(readonly icons: AgeIconsService,
                readonly navigationService: AgeNavigationService) {
    }

    get romFile$(): Observable<TAgeRomFile> {
        return this._romFile$;
    }

    openRomFile(romFile: TAgeRomFile): void {
        this._romFileSubject.next(romFile);
    }
}
