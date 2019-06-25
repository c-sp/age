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
import {TAgeRomFile} from "age-lib";
import {Observable, Subject} from "rxjs";
import {AGE_ICON_NAME_CARTRIDGE, AgeNavigationService} from "../common";


@Component({
    selector: "age-app-emulator",
    template: `
        <age-emulator-container [forcePause]="forcePause"
                                [romFile]="romFile$ | async">

            <age-toolbar-spacer></age-toolbar-spacer>

            <age-toolbar-action [matIconName]="romLibraryIconName"
                                (clicked)="navigationService.navigateToLibrary()"></age-toolbar-action>

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

    readonly romLibraryIconName = AGE_ICON_NAME_CARTRIDGE;

    @Input() forcePause = false;

    private readonly _romFileSubject = new Subject<TAgeRomFile>();
    private readonly _romFile$ = this._romFileSubject.asObservable();

    constructor(readonly navigationService: AgeNavigationService) {
    }

    get romFile$(): Observable<TAgeRomFile> {
        return this._romFile$;
    }

    openRomFile(romFile: TAgeRomFile): void {
        this._romFileSubject.next(romFile);
    }
}
