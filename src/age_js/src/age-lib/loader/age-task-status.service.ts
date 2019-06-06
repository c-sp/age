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

export interface ITaskStatus {
    readonly taskDescription: string;
    readonly taskStatus: TTaskStatus;
}


@Injectable()
export class AgeTaskStatusService {

    private readonly _taskStatusListSubject = new BehaviorSubject<ReadonlyArray<ITaskStatus>>([]);
    private readonly _taskStatusList$ = this._taskStatusListSubject.asObservable();

    private readonly _taskIdToListIndexMap = new Map<TTaskId, number>();
    private readonly _taskStatusList: ITaskStatus[] = [];
    private _nextTaskId = 1;

    get taskStatusList$(): Observable<ReadonlyArray<ITaskStatus>> {
        return this._taskStatusList$;
    }

    getTaskStatus(taskId: TTaskId): ITaskStatus | undefined {
        const taskListIndex = this._taskIdToListIndexMap.get(taskId);
        return (taskListIndex === undefined) ? undefined : this._taskStatusList[taskListIndex];
    }


    addTask(taskDescription: string, initialStatus: TTaskStatus = "working"): TTaskId {
        const taskId = this._nextTaskId++;

        this._taskStatusList.push({
            taskDescription,
            taskStatus: initialStatus,
        });
        this._taskIdToListIndexMap.set(taskId, this._taskStatusList.length - 1);
        this._taskStatusListSubject.next(this._taskStatusList);
        console.log("###", taskDescription, initialStatus);

        return taskId;
    }

    setTaskStatus(taskId: TTaskId, taskStatus: TTaskStatus): void {
        const taskListIndex = this._taskIdToListIndexMap.get(taskId);
        if (taskListIndex === undefined) {
            return;
        }
        console.log("###", this._taskStatusList[taskListIndex].taskDescription, taskStatus);
        this._taskStatusList[taskListIndex] = {
            ...this._taskStatusList[taskListIndex],
            taskStatus,
        };
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
                // Task success and failure can be detected using the "complete" and "error" callbacks.
                tap({
                    error: err => {
                        this.setTaskStatus(taskId, "failure");
                        console.error(err); // TODO just for debugging, remove later
                    },
                    complete: () => this.setTaskStatus(taskId, "success"),
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
