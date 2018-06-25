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

import {AfterViewInit, ChangeDetectionStrategy, Component, ElementRef, HostListener, ViewChild} from '@angular/core';
import {VERSION_INFO} from '../environments/version';
import {AgeEmulationPackage, AgeEmulationRuntimeInfo, AgeRect, AgeRomFileToLoad} from './common';


// TODO display warning if AudioWorklets are not available

// TODO display warning on timer precision (Date.now and Performance.now),
//      see https://developer.mozilla.org/en-US/docs/Web/API/Performance/now


@Component({
    selector: 'age-app-root',
    template: `
        <div class="container">

            <div>
                <age-bar [runtimeInfo]="emulationRuntimeInfo"></age-bar>
            </div>

            <div>
                <div><b>Build Details</b></div>
                <div>Commit Hash: {{versionHash}}</div>
                <div>Committed on: {{versionDate | date:'y-MM-dd HH:mm:ss'}}</div>
            </div>

            <div>
                <table>
                    <tr>
                        <th colspan="4">Key Mappings</th>
                    </tr>
                    <tr>
                        <th>Gameboy</th>
                        <th>Keyboard</th>
                        <th>Gameboy</th>
                        <th>Keyboard</th>
                    </tr>
                    <tr>
                        <td>Up</td>
                        <td>Up Arrow</td>
                        <td>A</td>
                        <td>A</td>
                    </tr>
                    <tr>
                        <td>Down</td>
                        <td>Down Arrow</td>
                        <td>B</td>
                        <td>S</td>
                    </tr>
                    <tr>
                        <td>Left</td>
                        <td>Left Arrow</td>
                        <td>Start</td>
                        <td>Space</td>
                    </tr>
                    <tr>
                        <td>Right</td>
                        <td>Right Arrow</td>
                        <td>Select</td>
                        <td>Enter</td>
                    </tr>
                </table>
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
            overflow: auto;
            font-family: sans-serif;
        }

        .container > div {
            margin-bottom: 2em;
            text-align: center;
            font-size: medium;
        }

        .container > div:nth-child(2) {
            font-size: smaller;
        }

        .container > div:nth-child(3) {
            font-size: smaller;
        }

        .container > div:nth-child(5) {
            flex: 1;
            min-height: 200px;
            min-width: 200px;
            overflow: hidden;
        }

        table {
            margin: auto;
        }

        table th {
            font-style: italic;
            font-weight: normal;
            text-align: right;
        }

        table th:nth-child(even) {
            text-align: left;
            padding-left: .5em;
        }

        table tr:nth-child(1) th {
            font-style: normal;
            font-weight: bold;
            text-align: center;
        }

        table td {
            text-align: right;
        }

        table td:nth-child(even) {
            text-align: left;
            padding-left: .5em;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AppComponent implements AfterViewInit {

    romFileToLoad?: AgeRomFileToLoad;
    emulationRuntimeInfo?: AgeEmulationRuntimeInfo;

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

    @HostListener('window:resize')
    emulatorContainerResize(): void {
        this._viewport = new AgeRect(
            this._emulatorContainer.nativeElement.offsetWidth,
            this._emulatorContainer.nativeElement.offsetHeight
        );
    }
}
