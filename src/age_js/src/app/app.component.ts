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
import {VERSION_INFO} from '../environments/version';
import {AgeEmulationPackage, AgeEmulationRuntimeInfo, AgeRect, AgeRomFileToLoad} from './common';
import {BarButton} from './modules/bar/age-bar.component';


// TODO display warning if AudioWorklets are not available

// TODO display warning on timer precision (Date.now and Performance.now),
//      see https://developer.mozilla.org/en-US/docs/Web/API/Performance/now


@Component({
    selector: 'age-app-root',
    template: `
        <div class="container">

            <div>
                <age-bar [runtimeInfo]="emulationRuntimeInfo"
                         (buttonClicked)="barButtonClick($event)"></age-bar>
            </div>

            <div>
                <age-info *ngIf="showInfo" class="dialog" (closeClicked)="closeDialogs()"></age-info>
            </div>

            <div>
                <age-file-selector (fileSelected)="selectFileToLoad($event)"></age-file-selector>
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

        .dialog {
            position: absolute;
            left: 50%;
            transform: translateX(-50%) translateY(.5em);
        }

        .container > div:nth-child(3) {
            margin-top: 2em;
            margin-bottom: 2em;
            text-align: center;
        }

        .container > div:nth-last-child(1) {
            text-align: center;
            margin: 5px;
            flex: 1;
            min-height: 200px;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AppComponent implements AfterViewInit {

    @Input() showInfo = false;
    @Input() romFileToLoad?: AgeRomFileToLoad;
    @Input() emulationRuntimeInfo?: AgeEmulationRuntimeInfo;

    @ViewChild('emulatorContainer')
    private _emulatorContainer!: ElementRef;
    private _emulationPackage?: AgeEmulationPackage;
    private _viewport = new AgeRect(1, 1);

    private _versionInfo = VERSION_INFO;


    ngAfterViewInit(): void {
        this.emulatorContainerResize();
    }


    get versionDate(): string {
        return this._versionInfo.date;
    }

    get versionHash(): string {
        return this._versionInfo.hash;
    }

    get emulationPackage(): AgeEmulationPackage | undefined {
        return this._emulationPackage;
    }

    get viewport(): AgeRect {
        return this._viewport;
    }


    selectFileToLoad(fileToLoad: AgeRomFileToLoad): void {
        this.romFileToLoad = fileToLoad;
        this._emulationPackage = undefined;
        this.emulationRuntimeInfo = undefined;
    }

    loadingComplete(emulationPackage: AgeEmulationPackage) {
        this._emulationPackage = emulationPackage;
    }

    barButtonClick(button: BarButton): void {
        switch (button) {
            case BarButton.INFO:
                this.showInfo = !this.showInfo;
                break;
            default:
                throw new Error(`invalid button identifier: ${button}`);
        }
    }

    closeDialogs(): void {
        this.showInfo = false;
    }

    @HostListener('window:resize')
    emulatorContainerResize(): void {
        this._viewport = new AgeRect(
            this._emulatorContainer.nativeElement.offsetWidth,
            this._emulatorContainer.nativeElement.offsetHeight
        );
    }
}
