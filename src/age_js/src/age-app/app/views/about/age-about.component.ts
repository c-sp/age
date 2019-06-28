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

import {ChangeDetectionStrategy, Component} from '@angular/core';
import {AgeBreakpointObserverService, AgeIconsService} from 'age-lib';
import {Observable} from 'rxjs';
import {VERSION_INFO} from '../../../../environments/version';
import {AgeNavigationService, IAgeViewMode, viewMode$} from '../../common';


@Component({
    selector: 'age-about',
    template: `
        <mat-toolbar *ngIf="(viewMode$ | async) as viewMode"
                     [color]="'primary'">

            <a mat-button [routerLink]="navigationService.rootUrl" [ngClass]="{'hidden': !viewMode.mobileView}">
                <mat-icon [svgIcon]="icons.faAngleLeft"></mat-icon>
            </a>

            <age-toolbar-spacer></age-toolbar-spacer>
            <span>about AGE</span>
            <age-toolbar-spacer></age-toolbar-spacer>

            <a mat-button [routerLink]="navigationService.rootUrl" [ngClass]="{'hidden': viewMode.mobileView}">
                <mat-icon [svgIcon]="icons.faTimes"></mat-icon>
            </a>

        </mat-toolbar>

        <div class="contents">
            <h1>AGE</h1>
            <h2>Another Gameboy Emulator</h2>

            <div>
                created by<br>
                <a class="age-ui-clickable" [age-href]="'https://gitlab.com/csprenger'">Christoph Sprenger</a>
            </div>

            <div>
                based on commit <a class="age-ui-clickable" [age-href]="commitLink">{{commitHash | slice:0:8}}</a><br>
                ({{commitDate | date:'long'}})
            </div>

            <div>
                <h3>Key Mappings</h3>
                <table>
                    <tr>
                        <th>Gameboy</th>
                        <th>Keyboard</th>
                    </tr>
                    <tr>
                        <td>Up</td>
                        <td>Up Arrow</td>
                    </tr>
                    <tr>
                        <td>Down</td>
                        <td>Down Arrow</td>
                    </tr>
                    <tr>
                        <td>Left</td>
                        <td>Left Arrow</td>
                    </tr>
                    <tr>
                        <td>Right</td>
                        <td>Right Arrow</td>
                    </tr>
                    <tr>
                        <td>A</td>
                        <td>A</td>
                    </tr>
                    <tr>
                        <td>B</td>
                        <td>S</td>
                    </tr>
                    <tr>
                        <td>Start</td>
                        <td>Space</td>
                    </tr>
                    <tr>
                        <td>Select</td>
                        <td>Enter</td>
                    </tr>
                </table>
            </div>

        </div>
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

        .contents {
            padding: 3em 1em 5em;
            text-align: center;
        }

        .contents > div {
            padding-top: 1em;
            text-align: center;
        }

        table {
            margin: auto;
        }

        table th {
            font-style: italic;
            text-align: right;
        }

        table th:nth-child(even) {
            text-align: left;
            padding-left: .5em;
        }

        table td {
            text-align: right;
        }

        table td:nth-child(even) {
            text-align: left;
            padding-left: .5em;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeAboutComponent {

    readonly commitHash = VERSION_INFO.hash;
    readonly commitLink = `https://gitlab.com/csprenger/AGE/tree/${this.commitHash}`;
    readonly commitDate = VERSION_INFO.date;

    readonly viewMode$: Observable<IAgeViewMode>;

    constructor(readonly navigationService: AgeNavigationService,
                readonly icons: AgeIconsService,
                breakpointObserverService: AgeBreakpointObserverService) {

        this.viewMode$ = viewMode$(breakpointObserverService);
    }
}
