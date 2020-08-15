//
// Copyright 2020 Christoph Sprenger
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

import {ChangeDetectionStrategy, Component, Input} from '@angular/core';
import {AgeIconsService, assertNever} from '../../common';
import {IAgeTaskStatus, TAgeTaskId, TAgeTaskStatus} from '../../emulation';


@Component({
    selector: 'age-task-status',
    template: `
        <table>
            <tr *ngFor="let status of statusList; trackBy: trackByTaskId">

                <td>
                    <mat-icon [svgIcon]="status.iconName"
                              [ngClass]="{'rotate': status.rotateIcon}"></mat-icon>
                </td>

                <td class="label">
                    <div>{{status.label}}</div>
                    <span class="mat-small">{{status.hint | slice:0:100}}</span>
                </td>

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

        tr > :nth-child(2) {
            padding: 0.25em;
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

    private _statusList = new Array<IStatus>();

    constructor(readonly icons: AgeIconsService) {
    }


    get statusList(): ReadonlyArray<IStatus> {
        return this._statusList;
    }

    @Input() set taskStatusList(taskStatusList: ReadonlyArray<IAgeTaskStatus>) {
        this._statusList = taskStatusList.map<IStatus>(taskStatus => {
            return {
                taskId: taskStatus.taskId,
                iconName: this._iconName(taskStatus.taskStatus),
                rotateIcon: taskStatus.taskStatus === 'working',
                label: taskStatus.taskDescription,

                hint: (taskStatus.taskStatus === 'failure')
                    ? taskStatus.taskError && taskStatus.taskError.toString()
                    : taskStatus.taskStatus,
            };
        });
    }

    trackByTaskId(_index: number, status: IStatus): TAgeTaskId {
        return status.taskId;
    }


    private _iconName(taskStatus: TAgeTaskStatus): string {
        switch (taskStatus) {
            case 'working':
                return this.icons.faCog;
            case 'success':
                return this.icons.faCheck;
            case 'cancelled':
                return this.icons.faTimes;
            case 'failure':
                return this.icons.faExclamationCircle;
            default:
                return assertNever(taskStatus);
        }
    }
}


interface IStatus {
    readonly taskId: TAgeTaskId;
    readonly iconName: string;
    readonly rotateIcon: boolean;
    readonly label: string;
    readonly hint: string;
}
