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
import {AgeEmulationPackage, AgeRect, AgeRomFileToLoad, IAgeEmulationRuntimeInfo} from "age-lib";
import {TitleBarButton} from "./title-bar/age-title-bar.component";


@Component({
    selector: "age-app-root",
    template: `
        <div class="container"
             (click)="closeDialogs($event)"
             (keypress)="closeDialogs($event)">

            <div>
                <age-title-bar [runtimeInfo]="emulationRuntimeInfo"
                               (buttonClicked)="toggleDialog($event)"></age-title-bar>
            </div>

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

            <div #emulatorContainer>
                <div *ngIf="showSplashScreen">
                    <age-splash-screen></age-splash-screen>
                </div>

                <age-loader [romFileToLoad]="romFileToLoad"
                            (loadingComplete)="loadingComplete($event)"></age-loader>

                <age-emulator *ngIf="!showSplashScreen"
                              [emulationPackage]="emulationPackage"
                              [viewport]="viewport"
                              (updateRuntimeInfo)="emulationRuntimeInfo = $event"></age-emulator>
            </div>

        </div>
    `,
    styles: [`
        .container {
            display: flex;
            height: 100%;
            flex-direction: column;
        }

        .container > :nth-last-child(1) {
            text-align: center;
            margin: .2em;
            flex: 1;
            min-height: 200px;
        }

        .container > :nth-last-child(1) > div {
            margin-top: 3em;
            margin-bottom: 3em;
        }

        .dialog {
            position: absolute;
            left: 50%;
            max-width: 30em;
            transform: translateX(-50%) translateY(.5em);
        }

        .close-dialog-icon {
            position: absolute;
            top: .25em;
            right: .25em;
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

    @ViewChild("emulatorContainer", {static: false}) private _emulatorContainer?: ElementRef;
    @ViewChild("dialogDiv", {static: false}) private _dialogDiv?: ElementRef;

    private _emulationPackage?: AgeEmulationPackage;
    private _viewport = new AgeRect(1, 1);
    private _ignoreCloseDialogs = false;


    get showSplashScreen(): boolean {
        return !this.romFileToLoad;
    }

    get emulationPackage(): AgeEmulationPackage | undefined {
        return this._emulationPackage;
    }

    get viewport(): AgeRect {
        return this._viewport;
    }


    openRom(fileToLoad: AgeRomFileToLoad): void {
        this.romFileToLoad = fileToLoad;
        this._emulationPackage = undefined;
        this.emulationRuntimeInfo = undefined;
        this.closeDialogs();
    }

    loadingComplete(emulationPackage: AgeEmulationPackage) {
        this._emulationPackage = emulationPackage;
        this.emulatorContainerResize(); // initial load: update emulator screen size
    }

    toggleDialog(button: TitleBarButton): void {
        if (this.showDialog !== button) {
            this.showDialog = button;
            this._ignoreCloseDialogs = true; // the same click triggers an immediate "closeDialogs"
        }
        // we rely on the immediate "closeDialogs" to close the dialog
    }

    closeDialogs(event?: Event): void {
        const eventContainsDialog = event && this._dialogDiv && this._dialogDiv.nativeElement.contains(event.target);

        if (!event || (!eventContainsDialog && !this._ignoreCloseDialogs)) {
            this.showDialog = undefined;
        }
        this._ignoreCloseDialogs = false;
    }

    @HostListener("window:resize") emulatorContainerResize(): void {
        if (this._emulatorContainer) {
            this._viewport = new AgeRect(
                this._emulatorContainer.nativeElement.offsetWidth,
                this._emulatorContainer.nativeElement.offsetHeight,
            );
        }
    }
}
