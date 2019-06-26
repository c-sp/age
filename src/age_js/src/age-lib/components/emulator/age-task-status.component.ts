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
import {AgeIconsService} from "../../common";
import {IAgeTaskStatus, TAgeTaskId} from "../../emulation";


@Component({
    selector: "age-task-status",
    template: `
        <table>
            <tr *ngFor="let status of taskStatusList; trackBy: trackByTaskId">

                <td [ngSwitch]="status.taskStatus">
                    <mat-icon *ngSwitchCase="'working'" [svgIcon]="icons.faCog" class="rotate"></mat-icon>
                    <mat-icon *ngSwitchCase="'success'" [svgIcon]="icons.faCheck"></mat-icon>
                    <mat-icon *ngSwitchCase="'cancelled'" [svgIcon]="icons.faTimes"></mat-icon>
                    <mat-icon *ngSwitchDefault [svgIcon]="icons.faExclamationCircle"></mat-icon>
                </td>

                <td>{{status.taskDescription}}</td>
            </tr>
        </table>
    `,
    styles: [`
        table {
            margin-left: auto;
            margin-right: auto;
        }

        tr > :nth-child(1) {
            padding-right: 0.5em;
        }

        .rotate {
            animation-duration: 2s;
            animation-iteration-count: infinite;
            animation-name: rotate;
            animation-timing-function: linear;
        }

        @keyframes rotate {
            from {
                transform: rotate(0deg);
            }
            to {
                transform: rotate(360deg);
            }
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeTaskStatusComponent {

    @Input() taskStatusList: ReadonlyArray<IAgeTaskStatus> = [];

    constructor(readonly icons: AgeIconsService) {
    }

    trackByTaskId(_index: number, taskStatus: IAgeTaskStatus): TAgeTaskId {
        return taskStatus.taskId;
    }
}
