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

import {ChangeDetectionStrategy, Component} from "@angular/core";
import {faCheck} from "@fortawesome/free-solid-svg-icons/faCheck";
import {faCog} from "@fortawesome/free-solid-svg-icons/faCog";
import {faExclamationCircle} from "@fortawesome/free-solid-svg-icons/faExclamationCircle";
import {faTimes} from "@fortawesome/free-solid-svg-icons/faTimes";
import {Observable} from "rxjs";
import {AgeTaskStatusHandlerService, ITaskStatus, TTaskId} from "./age-task-status-handler.service";


@Component({
    selector: "age-task-status",
    template: `
        <ng-container *ngIf="(taskStatusList$ | async) as statusList">
            <div *ngFor="let status of statusList; trackBy: trackByTaskId">

                <ng-container [ngSwitch]="status.taskStatus">
                    <fa-icon *ngSwitchCase="'working'" [icon]="iconWorking" [spin]="true"></fa-icon>
                    <fa-icon *ngSwitchCase="'success'" [icon]="iconSuccess"></fa-icon>
                    <fa-icon *ngSwitchCase="'cancelled'" [icon]="iconCancelled"></fa-icon>
                    <fa-icon *ngSwitchDefault [icon]="iconError"></fa-icon>
                </ng-container>

                {{status.taskDescription}}
            </div>
        </ng-container>
    `,
    styles: [`
        fa-icon {
            margin-right: 0.5em;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeTaskStatusComponent {

    readonly iconWorking = faCog;
    readonly iconSuccess = faCheck;
    readonly iconCancelled = faTimes;
    readonly iconError = faExclamationCircle;

    readonly taskStatusList$: Observable<ReadonlyArray<ITaskStatus> | undefined>;

    constructor(statusHandler: AgeTaskStatusHandlerService) {
        this.taskStatusList$ = statusHandler.taskStatusList$;
    }

    trackByTaskId(_index: number, taskStatus: ITaskStatus): TTaskId {
        return taskStatus.taskId;
    }
}
