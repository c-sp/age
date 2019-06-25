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

import {animate, state, style, transition, trigger} from "@angular/animations";
import {ChangeDetectionStrategy, Component, ElementRef, Input, NgZone} from "@angular/core";
import {BehaviorSubject, Observable, of} from "rxjs";
import {map, shareReplay, switchMap, tap} from "rxjs/operators";
import {AgeBreakpointObserverService} from "../../common";
import {AgeEmulationService, IAgeEmulationStatus, TAgeRomFile} from "../../emulation";
import {AgePlayPauseStatus} from "../toolbar";
import {AgeToolbarVisibility} from "../toolbar/age-toolbar-visibility";
import {emulationViewport$, IAgeViewport} from "./age-emulation-viewport-calculator";


@Component({
    selector: "age-emulator-container",
    template: `
        <div (click)="clickViewport()"
             (keypress)="clickViewport()"
             (mousemove)="mouseMoveViewport()"
             [ngStyle]="viewportStyle$ | async">

            <ng-container *ngIf="(emulationStatus$ | async) as emuStatus">

                <age-emulation *ngIf="emuStatus.emulation as emulation"
                               [audioVolume]="audioVolume"
                               [emulation]="emulation"
                               [pauseEmulation]="pauseEmulation"></age-emulation>

                <div class="gui-overlay">

                    <age-toolbar-background [@showToolbar]="toolbarVisible$ | async">
                        <mat-toolbar>
                            <age-toolbar-action-play [playPauseStatus]="playPauseStatus"
                                                     (paused)="isPaused = $event"></age-toolbar-action-play>

                            <age-toolbar-action-volume (volumeChange)="audioVolume = $event">
                            </age-toolbar-action-volume>

                            <ng-content></ng-content>
                        </mat-toolbar>
                    </age-toolbar-background>

                    <ng-container *ngIf="!emuStatus.emulation">

                        <age-task-status *ngIf="emuStatus.taskStatusList as statusList; else openRomHint"
                                         [taskStatusList]="statusList"></age-task-status>

                        <ng-template #openRomHint>
                            <div class="rom-hint">
                                please open a rom file
                            </div>
                        </ng-template>

                    </ng-container>

                </div>

            </ng-container>

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
            display: block;
            padding-top: 1em;
        }

        .rom-hint {
            padding-top: 3em;
        }
    `],
    animations: [
        trigger("showToolbar", [
            state("true", style({
                opacity: "1",
            })),
            state("false", style({
                opacity: "0",
                display: "none",
            })),
            transition("* => true", [
                style({display: "block"}),
                animate("0.3s"),
            ]),
            transition("* => false", animate("0.3s")),
        ]),
    ],
    providers: [
        AgeEmulationService,
    ],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeEmulatorContainerComponent {

    readonly emulationStatus$: Observable<Partial<IAgeEmulationStatus>>;
    readonly viewportStyle$: Observable<object>;
    readonly toolbarVisible$: Observable<boolean>;

    audioVolume = 0;

    private readonly _romFileSubject = new BehaviorSubject<TAgeRomFile | undefined>(undefined);
    private readonly _toolbarVisibility: AgeToolbarVisibility;

    private _hasEmulation = false;
    private _forcePause = false;
    private _isPaused = false;

    constructor(hostElementRef: ElementRef,
                emulationService: AgeEmulationService,
                breakpointObserver: AgeBreakpointObserverService,
                ngZone: NgZone) {

        this.emulationStatus$ = this._romFileSubject.asObservable().pipe(
            switchMap<TAgeRomFile | undefined, Observable<Partial<IAgeEmulationStatus>>>(
                romFile => !romFile ? of({}) : emulationService.newEmulation$(romFile),
            ),
            tap(emuStatus => {
                this._hasEmulation = !!emuStatus.emulation;
                // show the toolbar as long as no emulation is ready
                this._toolbarVisibility.setVisible(!this._hasEmulation);
                // clear the "is-paused" flag, if the emulation is ready
                if (this._hasEmulation) {
                    this._isPaused = false;
                }
            }),
            // load only once
            shareReplay(1),
        );

        this.viewportStyle$ = emulationViewport$(
            hostElementRef,
            this.emulationStatus$.pipe(
                map(emuStatus => emuStatus.emulation && emuStatus.emulation.screenSize),
            ),
            ngZone,
        ).pipe(
            map(createViewportStyle),
        );

        this._toolbarVisibility = new AgeToolbarVisibility(breakpointObserver);
        this.toolbarVisible$ = this._toolbarVisibility.toolbarVisible$;
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


    mouseMoveViewport(): void {
        this._toolbarVisibility.mouseMove();
    }

    clickViewport(): void {
        this._toolbarVisibility.click();
    }
}


function createViewportStyle(viewport: IAgeViewport): object {
    return {
        position: "absolute",
        left: cssPxString(viewport.left),
        top: cssPxString(viewport.top),
        width: cssPxString(viewport.width),
        height: cssPxString(viewport.height),
        // For some reason the canvas element is always displayed with some margin/padding below,
        // which triggers a vertical scrollbar.
        // By applying the padding below, the scrollbar disappears.
        // Having the emulator "screen" not touch the browser GUI looks nicer anyway ...
        padding: "2px",
    };

    function cssPxString(value: number): string {
        return (value === 0) ? "0" : `${value}px`;
    }
}
