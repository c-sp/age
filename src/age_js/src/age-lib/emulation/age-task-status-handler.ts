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

import {BehaviorSubject, Observable, Subscriber} from "rxjs";
import {tap} from "rxjs/operators";


export type TAgeTaskStatus = "working" | "success" | "failure" | "cancelled";
export type TAgeTaskId = number;
// tslint:disable-next-line:no-any
export type TAgeTaskError = any;

export interface IAgeTaskStatus {
    readonly taskId: TAgeTaskId;
    readonly taskDescription: string;
    readonly taskStatus: TAgeTaskStatus;
    readonly taskError?: TAgeTaskError;
}


/**
 * Helper Service for collecting information about the current rom loading process.
 */
export class AgeTaskStatusHandler {

    private readonly _taskStatusListSubject = new BehaviorSubject<ReadonlyArray<IAgeTaskStatus>>([]);
    private readonly _taskStatusList$ = this._taskStatusListSubject.asObservable();

    private readonly _taskIdToListIndexMap = new Map<TAgeTaskId, number>();
    private readonly _taskStatusList: IAgeTaskStatus[] = [];
    private _nextTaskId = 1;

    get taskStatusList$(): Observable<ReadonlyArray<IAgeTaskStatus>> {
        return this._taskStatusList$;
    }


    addTask(taskDescription: string, initialStatus: TAgeTaskStatus = "working"): TAgeTaskId {
        const taskId = this._nextTaskId++;

        this._taskStatusList.push({
            taskId,
            taskDescription,
            taskStatus: initialStatus,
        });
        this._taskIdToListIndexMap.set(taskId, this._taskStatusList.length - 1);
        this._broadcastTaskStatus();

        return taskId;
    }

    addTask$<T>(taskDescription: string, obs$: Observable<T>): Observable<T> {
        const taskId = this.addTask(taskDescription);
        const setCancelled = () => {
            // only working tasks can be cancelled
            const status = this._getTaskStatus(taskId);
            if ((status && status.taskStatus) === "working") {
                this._setTaskStatus(taskId, "cancelled");
            }
        };

        return obs$
            .pipe(
                tap({
                    // we use "next" instead of "complete" to detect task success the moment
                    // tap() is reached instead of the moment of observable completion
                    // (the latter might never occur, e.g. if a subsequent operator throws an error)
                    next: () => this._setTaskStatus(taskId, "success"),
                    error: err => this._setTaskStatus(taskId, "failure", err),
                }),
            )
            // to detect task cancellation we have to add a teardown logic to the subscriber
            .lift({
                call(subscriber: Subscriber<T>, source: Observable<T>): void {
                    subscriber.add(setCancelled);
                    source.subscribe(subscriber);
                },
            });
    }


    private _getTaskStatus(taskId: TAgeTaskId): IAgeTaskStatus | undefined {
        const taskListIndex = this._taskIdToListIndexMap.get(taskId);
        return (taskListIndex === undefined) ? undefined : this._taskStatusList[taskListIndex];
    }

    private _setTaskStatus(taskId: TAgeTaskId, taskStatus: TAgeTaskStatus, taskError?: TAgeTaskError): void {
        const taskListIndex = this._taskIdToListIndexMap.get(taskId);
        if (taskListIndex === undefined) {
            return;
        }

        this._taskStatusList[taskListIndex] = {
            ...this._taskStatusList[taskListIndex],
            taskStatus,
            taskError,
        };
        this._broadcastTaskStatus();
    }

    private _broadcastTaskStatus(): void {
        // emit a new array instance so that Angular's change detection recognises the new status
        this._taskStatusListSubject.next(this._taskStatusList.slice());
    }
}
