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

import {ChangeDetectionStrategy, ChangeDetectorRef, Component, ElementRef, Input, NgZone, OnInit} from "@angular/core";
import {BehaviorSubject, combineLatest, Observable, of} from "rxjs";
import {map, shareReplay, switchMap, tap} from "rxjs/operators";
import {AgeEmulation, AgeEmulationService, IAgeTaskStatus, TAgeRomFile} from "../../emulation";
import {AgePlayPauseStatus} from "../toolbar";
import {emulationViewport$, IAgeViewport} from "./age-emulation-viewport-calculator";


@Component({
    selector: "age-emulator-container",
    template: `
        <div *ngIf="(state$ | async) as state"
             [ngStyle]="state.viewportStyle">

            <age-emulation *ngIf="state.emulation as emulation"
                           [emulation]="emulation"
                           [pauseEmulation]="pauseEmulation"></age-emulation>

            <div class="gui-overlay">

                <age-toolbar-background>
                    <mat-toolbar>
                        <age-toolbar-action-play [playPauseStatus]="playPauseStatus"
                                                 (paused)="isPaused = $event"></age-toolbar-action-play>

                        <age-toolbar-action-volume [isMuted]="false"></age-toolbar-action-volume>

                        <ng-content></ng-content>
                    </mat-toolbar>
                </age-toolbar-background>

                <ng-container *ngIf="!state.emulation">

                    <age-task-status *ngIf="state.taskStatusList as statusList; else openRomHint"
                                     [taskStatusList]="statusList"></age-task-status>

                    <ng-template #openRomHint>
                        <div class="rom-hint">
                            please open a rom file
                        </div>
                    </ng-template>

                </ng-container>

            </div>

        </div>
    `,
    styles: [`
        :host {
            /* the host element's size is used to calculate the emulator screen size and position */
            display: block;
            position: relative;
            /*
             minimal dimensions we need to display components like the toolbar in a sensible way
             (we choose 320px as 320dp is the minimal screen width of mobile phones)
              */
            min-height: 320px;
            min-width: 320px;
        }

        .gui-overlay {
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            text-align: center;
        }

        mat-toolbar {
            background: none;
        }

        age-task-status {
            padding-top: 1em;
        }

        .rom-hint {
            padding-top: 3em;
        }
    `],
    providers: [
        AgeEmulationService,
    ],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeEmulatorContainerComponent implements OnInit {

    private readonly _romFileSubject = new BehaviorSubject<TAgeRomFile | undefined>(undefined);
    private _state$?: Observable<IState>;
    private _hasEmulation = false;
    private _forcePause = false;
    private _isPaused = false;

    constructor(private readonly _hostElementRef: ElementRef,
                private readonly _emulationService: AgeEmulationService,
                private readonly _changeDetectorRef: ChangeDetectorRef,
                private readonly _ngZone: NgZone) {
    }

    ngOnInit(): void {
        const emulationStatus$ = this._romFileSubject.asObservable().pipe(
            switchMap(romFile => !romFile ? of(undefined) : this._emulationService.newEmulation$(romFile)),
            // if a new rom file has been loaded, clear the previous "is-paused" flag
            tap(emuStatus => {
                if (emuStatus && emuStatus.emulation) {
                    this._isPaused = false;
                }
            }),
            // load only once
            shareReplay(1),
        );

        const viewportStyle$ = emulationViewport$(
            this._hostElementRef,
            emulationStatus$.pipe(
                map(emuStatus => emuStatus && emuStatus.emulation && emuStatus.emulation.screenSize),
            ),
            this._ngZone,
        ).pipe(
            map(createViewportStyle),
        );

        this._state$ = combineLatest([
            emulationStatus$,
            viewportStyle$,
        ]).pipe(
            map(values => {
                this._changeDetectorRef.markForCheck();
                const emuStatus = values[0];
                return {
                    emulation: emuStatus && emuStatus.emulation,
                    taskStatusList: emuStatus && emuStatus.taskStatusList,
                    viewportStyle: values[1],
                };
            }),
            tap(state => this._hasEmulation = !!state.emulation),
        );
    }


    get state$(): Observable<IState> | undefined {
        return this._state$;
    }

    get pauseEmulation(): boolean {
        return this._forcePause || this._isPaused;
    }

    get playPauseStatus(): AgePlayPauseStatus {
        if (!this._hasEmulation) {
            return AgePlayPauseStatus.DISABLED;
        }
        return this.pauseEmulation ? AgePlayPauseStatus.IS_PAUSED : AgePlayPauseStatus.IS_PLAYING;
    }

    @Input() set forcePause(forcePause: boolean) {
        this._forcePause = forcePause;
    }

    @Input() set isPaused(isPaused: boolean) {
        this._isPaused = isPaused;
    }

    @Input() set romFile(romFile: TAgeRomFile | undefined) {
        this._romFileSubject.next(romFile);
    }
}


interface IState {
    readonly emulation?: AgeEmulation;
    readonly taskStatusList?: ReadonlyArray<IAgeTaskStatus>;
    readonly viewportStyle: object;
}


function createViewportStyle(viewport: IAgeViewport): object {
    return {
        position: "absolute",
        left: cssPxString(viewport.left),
        top: cssPxString(viewport.top),
        width: cssPxString(viewport.width),
        height: cssPxString(viewport.height),
    };

    function cssPxString(value: number): string {
        return (value === 0) ? "0" : `${value}px`;
    }
}
