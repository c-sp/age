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

import {ChangeDetectionStrategy, Component, EventEmitter, Output} from '@angular/core';
import {VERSION_INFO} from '../../../environments/version';


@Component({
    selector: 'age-info',
    template: `
        <div class="container age-ui">

            <i class="fa fa-times-circle age-ui-clickable age-ui-top-right"
               (click)="closeClicked.emit()"></i>

            <div>
                <div class="age-ui-title">
                    <age-repo-link>
                        <span>A</span>nother
                        <span>G</span>ameboy
                        <span>E</span>mulator
                    </age-repo-link>
                </div>
                <div>{{versionDate | date:'long'}}</div>
                <div>
                    <age-repo-commit-link>commit</age-repo-commit-link>
                </div>
            </div>

            <div>
                <div class="age-ui-highlight">Key Mappings</div>
                <table>
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

        </div>
    `,
    styles: [`
        .container {
            padding: 2em;
            opacity: 0.95;
            text-align: center;
        }

        .container > :nth-child(2) {
            margin-bottom: 2em;
        }

        .container > :nth-child(2) > :nth-child(2) {
            margin-top: 1em;
        }

        .container > :nth-child(2) span {
            color: #E0E8FF;
        }

        .container > div:nth-child(n+3) > :nth-child(1) {
            font-size: large;
            font-weight: bold;
            padding-bottom: .5em;
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
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeInfoComponent {

    readonly versionDate = VERSION_INFO.date;

    @Output() readonly closeClicked = new EventEmitter();
}
