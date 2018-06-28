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

import {AfterViewInit, ChangeDetectionStrategy, Component, ElementRef, HostListener, Input, ViewChild} from '@angular/core';
import {AgeEmulationPackage, AgeEmulationRuntimeInfo, AgeRect, AgeRomFileToLoad} from './common';
import {TitleBarButton} from './modules/title-bar/age-title-bar.component';


// TODO display warning if AudioWorklet is not available or use polyfill

// TODO display warning on timer precision (Date.now and Performance.now),
//      see https://developer.mozilla.org/en-US/docs/Web/API/Performance/now


@Component({
    selector: 'age-app-root',
    template: `
        <div class="container" (click)="closeDialogs($event)">

            <div>
                <age-title-bar [runtimeInfo]="emulationRuntimeInfo"
                               (buttonClicked)="toggleDialog($event)"></age-title-bar>
            </div>

            <div #dialogDiv>
                <age-info *ngIf="showInfo"
                          class="dialog"
                          (closeClicked)="closeDialogs()"></age-info>

                <age-open-rom *ngIf="showOpenRom"
                              class="dialog"
                              (openRom)="openRom($event)"
                              (closeClicked)="closeDialogs()"></age-open-rom>
            </div>

            <div #emulatorContainer>
                <age-loader [romFileToLoad]="romFileToLoad"
                            (loadingComplete)="loadingComplete($event)"></age-loader>

                <age-emulator [emulationPackage]="emulationPackage"
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

        .dialog {
            position: absolute;
            left: 50%;
            max-width: 30em;
            transform: translateX(-50%) translateY(.5em);
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeAppComponent implements AfterViewInit {

    @Input() romFileToLoad?: AgeRomFileToLoad;
    @Input() emulationRuntimeInfo?: AgeEmulationRuntimeInfo;

    @ViewChild('emulatorContainer') private _emulatorContainer!: ElementRef;
    @ViewChild('dialogDiv') private _dialogDiv!: ElementRef;

    private _emulationPackage?: AgeEmulationPackage;
    private _viewport = new AgeRect(1, 1);
    private _showDialog?: TitleBarButton;
    private _ignoreCloseDialogs = false;


    ngAfterViewInit(): void {
        this.emulatorContainerResize();
    }


    get emulationPackage(): AgeEmulationPackage | undefined {
        return this._emulationPackage;
    }

    get viewport(): AgeRect {
        return this._viewport;
    }

    get showInfo(): boolean {
        return this._showDialog === TitleBarButton.INFO;
    }

    get showOpenRom(): boolean {
        return this._showDialog === TitleBarButton.OPEN_ROM;
    }


    openRom(fileToLoad: AgeRomFileToLoad): void {
        this.romFileToLoad = fileToLoad;
        this._emulationPackage = undefined;
        this.emulationRuntimeInfo = undefined;
        this.closeDialogs();
    }

    loadingComplete(emulationPackage: AgeEmulationPackage) {
        this._emulationPackage = emulationPackage;
    }

    toggleDialog(button: TitleBarButton): void {
        if (this._showDialog !== button) {
            this._showDialog = button;
            this._ignoreCloseDialogs = true; // the same click triggers an immediate "closeDialogs"
        }
    }

    closeDialogs(event?: Event): void {
        if (!event || (!this._dialogDiv.nativeElement.contains(event.target) && !this._ignoreCloseDialogs)) {
            this._showDialog = undefined;
        }
        this._ignoreCloseDialogs = false;
    }

    @HostListener('window:resize')
    emulatorContainerResize(): void {
        this._viewport = new AgeRect(
            this._emulatorContainer.nativeElement.offsetWidth,
            this._emulatorContainer.nativeElement.offsetHeight
        );
    }
}
