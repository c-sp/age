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

import {ChangeDetectionStrategy, Component, ElementRef, HostListener, Input, ViewChild} from "@angular/core";
import {faCheck} from "@fortawesome/free-solid-svg-icons/faCheck";
import {faCog} from "@fortawesome/free-solid-svg-icons/faCog";
import {faExclamationCircle} from "@fortawesome/free-solid-svg-icons/faExclamationCircle";
import {faTimes} from "@fortawesome/free-solid-svg-icons/faTimes";
import {faTimesCircle} from "@fortawesome/free-solid-svg-icons/faTimesCircle";
import {
    AgeEmulationFactoryService,
    AgeEmulationRunner,
    AgeTaskStatusHandlerService,
    IAgeEmulationRuntimeInfo,
    ITaskStatus,
    TAgeRomFile,
    TTaskId,
} from "age-lib";
import {Observable} from "rxjs";
import {TitleBarButton} from "./title-bar/age-title-bar.component";


@Component({
    selector: "age-app-root",
    template: `
        <age-title-bar [runtimeInfo]="emulationRuntimeInfo"
                       (buttonClicked)="toggleDialog($event)"></age-title-bar>

        <div #dialogDiv>
            <div *ngIf="showDialog" class="dialog">

                <age-info *ngIf="showDialog === TitleBarButton.INFO"></age-info>

                <age-open-rom *ngIf="showDialog === TitleBarButton.OPEN_ROM"
                              (openRom)="openRom($event)"></age-open-rom>

                <fa-icon [icon]="faTimesCircle" class="close-dialog-icon age-ui-clickable"
                         title="Close this dialog"
                         (click)="closeDialogs()"></fa-icon>
            </div>
        </div>

        <age-splash-screen *ngIf="!emulationRunner$"></age-splash-screen>

        <div *ngIf="(taskStatusList$ | async) as statusList"
             class="status-list">
            <div *ngFor="let status of statusList; trackBy: trackByTaskId">

                <ng-container [ngSwitch]="status.taskStatus">
                    <fa-icon *ngSwitchCase="'working'" [icon]="faCog" [spin]="true"></fa-icon>
                    <fa-icon *ngSwitchCase="'success'" [icon]="faCheck"></fa-icon>
                    <fa-icon *ngSwitchCase="'cancelled'" [icon]="faTimes"></fa-icon>
                    <fa-icon *ngSwitchDefault [icon]="faExclamationCircle" class="age-ui-error"></fa-icon>
                </ng-container>

                {{status.taskDescription}}
            </div>
        </div>

        <age-emulator *ngIf="(emulationRunner$ | async) as emuRunner"
                      [emulationRunner]="emuRunner"
                      (updateRuntimeInfo)="emulationRuntimeInfo = $event"></age-emulator>
    `,
    styles: [`
        :host {
            display: flex;
            height: 100%;
            flex-direction: column;
        }

        .dialog {
            position: absolute;
            left: 50%;
            max-width: 30em;
            transform: translateX(-50%) translateY(.5em);
            z-index: 10000;
        }

        .dialog > fa-icon {
            position: absolute;
            top: .25em;
            right: .25em;
        }

        age-splash-screen {
            margin-top: 3em;
            margin-bottom: 3em;
        }

        .status-list {
            margin-top: 3em;
            text-align: center;
            font-size: smaller;
        }

        .status-list > fa-icon {
            margin-right: 0.5em;
        }

        age-emulator {
            flex: 1 1;
            margin: .2em;
        }
    `],
    providers: [
        AgeTaskStatusHandlerService,
        AgeEmulationFactoryService,
    ],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeAppComponent {

    readonly faCheck = faCheck;
    readonly faCog = faCog;
    readonly faTimes = faTimes;
    readonly faExclamationCircle = faExclamationCircle;

    readonly TitleBarButton = TitleBarButton;
    readonly faTimesCircle = faTimesCircle;
    readonly taskStatusList$: Observable<ReadonlyArray<ITaskStatus> | undefined>;

    @Input() emulationRuntimeInfo?: IAgeEmulationRuntimeInfo;
    @Input() showDialog?: TitleBarButton;

    @ViewChild("dialogDiv", {static: false}) private _dialogDiv?: ElementRef;

    private _emulationRunner$?: Observable<AgeEmulationRunner>;
    private _ignoreCloseDialogs = false;

    constructor(private readonly _emulationFactory: AgeEmulationFactoryService,
                statusHandler: AgeTaskStatusHandlerService) {

        this.taskStatusList$ = statusHandler.taskStatusList$;
    }


    get emulationRunner$(): Observable<AgeEmulationRunner> | undefined {
        return this._emulationRunner$;
    }

    openRom(romFileToLoad: TAgeRomFile): void {
        this._emulationRunner$ = this._emulationFactory.newEmulation$(romFileToLoad);
        this.closeDialogs();
    }

    toggleDialog(button: TitleBarButton): void {
        if (this.showDialog !== button) {
            this.showDialog = button;
            this._ignoreCloseDialogs = true; // the same click triggers an immediate "closeDialogs"
        }
        // we rely on the immediate "closeDialogs" to close the dialog
    }

    @HostListener("click", ["$event"])
    @HostListener("keypress", ["$event"])
    closeDialogs(event?: Event): void {
        const eventContainsDialog = event && this._dialogDiv && this._dialogDiv.nativeElement.contains(event.target);

        if (!event || (!eventContainsDialog && !this._ignoreCloseDialogs)) {
            this.showDialog = undefined;
        }
        this._ignoreCloseDialogs = false;
    }

    trackByTaskId(_index: number, taskStatus: ITaskStatus): TTaskId {
        return taskStatus.taskId;
    }
}
