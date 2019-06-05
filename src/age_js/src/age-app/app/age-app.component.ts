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

import {ChangeDetectionStrategy, Component, ElementRef, HostListener, Input, ViewChild} from "@angular/core";
import {faTimesCircle} from "@fortawesome/free-solid-svg-icons/faTimesCircle";
import {AgeRomFileToLoad, IAgeEmulationRuntimeInfo} from "age-lib";
import {TitleBarButton} from "./title-bar/age-title-bar.component";


@Component({
    selector: "age-app-root",
    template: `
        <age-title-bar [runtimeInfo]="emulationRuntimeInfo"
                       (buttonClicked)="toggleDialog($event)"></age-title-bar>

        <div #dialogDiv>
            <div *ngIf="showDialog" class="dialog">

                <age-info *ngIf="showDialog === TitleBarButton.INFO"></age-info>

                <age-open-rom *ngIf="showDialog === TitleBarButton.OPEN_ROM"
                              (openRom)="openRom($event)"></age-open-rom>

                <fa-icon [icon]="faTimesCircle" class="close-dialog-icon age-ui-clickable"
                         title="Close this dialog"
                         (click)="closeDialogs()"></fa-icon>
            </div>
        </div>

        <age-splash-screen *ngIf="!romFileToLoad"></age-splash-screen>

        <age-emulator *ngIf="romFileToLoad"
                      [loadRomFile]="romFileToLoad"
                      (updateRuntimeInfo)="emulationRuntimeInfo = $event"></age-emulator>
    `,
    styles: [`
        :host {
            display: flex;
            height: 100%;
            flex-direction: column;
        }

        .dialog {
            position: absolute;
            left: 50%;
            max-width: 30em;
            transform: translateX(-50%) translateY(.5em);
            z-index: 10000;
        }

        .dialog > fa-icon {
            position: absolute;
            top: .25em;
            right: .25em;
        }

        age-splash-screen {
            margin-top: 3em;
            margin-bottom: 3em;
        }

        age-emulator {
            flex: 1 1;
            margin: .2em;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeAppComponent {

    readonly TitleBarButton = TitleBarButton;
    readonly faTimesCircle = faTimesCircle;

    @Input() romFileToLoad?: AgeRomFileToLoad;
    @Input() emulationRuntimeInfo?: IAgeEmulationRuntimeInfo;
    @Input() showDialog?: TitleBarButton;

    @ViewChild("dialogDiv", {static: false}) private _dialogDiv?: ElementRef;

    private _ignoreCloseDialogs = false;

    openRom(fileToLoad: AgeRomFileToLoad): void {
        this.romFileToLoad = fileToLoad;
        this.closeDialogs();
    }

    toggleDialog(button: TitleBarButton): void {
        if (this.showDialog !== button) {
            this.showDialog = button;
            this._ignoreCloseDialogs = true; // the same click triggers an immediate "closeDialogs"
        }
        // we rely on the immediate "closeDialogs" to close the dialog
    }

    @HostListener("click", ["$event"])
    @HostListener("keypress", ["$event"])
    closeDialogs(event?: Event): void {
        const eventContainsDialog = event && this._dialogDiv && this._dialogDiv.nativeElement.contains(event.target);

        if (!event || (!eventContainsDialog && !this._ignoreCloseDialogs)) {
            this.showDialog = undefined;
        }
        this._ignoreCloseDialogs = false;
    }
}
