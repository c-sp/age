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

import {Injectable} from "@angular/core";
import {BehaviorSubject, Observable, Subscriber} from "rxjs";
import {tap} from "rxjs/operators";


export type TTaskStatus = "working" | "success" | "failure" | "cancelled";
export type TTaskId = number;
// tslint:disable-next-line:no-any
export type TTaskError = any;

export interface ITaskStatus {
    readonly taskId: TTaskId;
    readonly taskDescription: string;
    readonly taskStatus: TTaskStatus;
    readonly taskError?: TTaskError;
}


/**
 * Helper Service for collecting information about the current rom loading process.
 *
 * This service must be provided by a suitable component.
 * It must NOT be provided by the `root` injector since it should not be shared
 * across multiple AGE web components.
 */
@Injectable()
export class AgeTaskStatusHandlerService {

    private readonly _taskStatusListSubject = new BehaviorSubject<ReadonlyArray<ITaskStatus> | undefined>(undefined);
    private readonly _taskStatusList$ = this._taskStatusListSubject.asObservable();

    private readonly _taskIdToListIndexMap = new Map<TTaskId, number>();
    private readonly _taskStatusList: ITaskStatus[] = [];
    private _nextTaskId = 1;

    get taskStatusList$(): Observable<ReadonlyArray<ITaskStatus> | undefined> {
        return this._taskStatusList$;
    }

    getTaskStatus(taskId: TTaskId): ITaskStatus | undefined {
        const taskListIndex = this._taskIdToListIndexMap.get(taskId);
        return (taskListIndex === undefined) ? undefined : this._taskStatusList[taskListIndex];
    }


    clearTasks(): void {
        this._taskStatusListSubject.next(undefined);
        this._taskIdToListIndexMap.clear();
        this._taskStatusList.splice(0, this._taskStatusList.length);
    }

    addTask(taskDescription: string, initialStatus: TTaskStatus = "working"): TTaskId {
        const taskId = this._nextTaskId++;

        this._taskStatusList.push({
            taskId,
            taskDescription,
            taskStatus: initialStatus,
        });
        this._taskIdToListIndexMap.set(taskId, this._taskStatusList.length - 1);
        this._taskStatusListSubject.next(this._taskStatusList);

        return taskId;
    }

    setTaskStatus(taskId: TTaskId, taskStatus: TTaskStatus, taskError?: TTaskError): void {
        const taskListIndex = this._taskIdToListIndexMap.get(taskId);
        if (taskListIndex === undefined) {
            return;
        }

        this._taskStatusList[taskListIndex] = {
            ...this._taskStatusList[taskListIndex],
            taskStatus,
            taskError,
        };
        this._taskStatusListSubject.next(this._taskStatusList);
    }

    addTask$<T>(taskDescription: string, obs$: Observable<T>): Observable<T> {
        const taskId = this.addTask(taskDescription);
        const setCancelled = () => {
            // only working tasks can be cancelled
            const status = this.getTaskStatus(taskId);
            if ((status && status.taskStatus) === "working") {
                this.setTaskStatus(taskId, "cancelled");
            }
        };

        return obs$
            .pipe(
                tap({
                    // we use "next" instead of "complete" to detect task success the moment
                    // tap() is reached instead of the moment of observable completion
                    // (the latter might never occur, e.g. if a subsequent operator throws an error)
                    next: () => this.setTaskStatus(taskId, "success"),
                    error: err => this.setTaskStatus(taskId, "failure", err),
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
}
