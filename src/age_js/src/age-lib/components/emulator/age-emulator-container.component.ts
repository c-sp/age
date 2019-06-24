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

import {
    ChangeDetectionStrategy,
    ChangeDetectorRef,
    Component,
    ElementRef,
    HostBinding,
    Input,
    NgZone,
} from "@angular/core";
import {BehaviorSubject, Observable, of} from "rxjs";
import {map, shareReplay, switchMap, tap} from "rxjs/operators";
import {AgeBreakpointObserverService, AgeSubscriptionSink} from "../../common";
import {AgeEmulationService, IAgeEmulationStatus, TAgeRomFile} from "../../emulation";
import {AgePlayPauseStatus} from "../toolbar";
import {AgeToolbarVisibility} from "../toolbar/age-toolbar-visibility";
import {emulationViewport$, IAgeViewport} from "./age-emulation-viewport-calculator";


@Component({
    selector: "age-emulator-container",
    template: `
        <div (click)="clickViewport()"
             (keypress)="clickViewport()"
             (mouseenter)="mouseEnterLeaveViewport($event)"
             (mouseleave)="mouseEnterLeaveViewport($event)"
             [ngStyle]="viewportStyle$ | async">

            <ng-container *ngIf="(emulationStatus$ | async) as emuStatus">

                <age-emulation *ngIf="emuStatus.emulation as emulation"
                               [audioVolume]="audioVolume"
                               [emulation]="emulation"
                               [pauseEmulation]="pauseEmulation"></age-emulation>

                <div class="gui-overlay">

                    <age-toolbar-background>
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

        :host.hide-toolbar age-toolbar-background {
            display: none;
        }
    `],
    providers: [
        AgeEmulationService,
    ],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeEmulatorContainerComponent extends AgeSubscriptionSink {

    readonly emulationStatus$: Observable<Partial<IAgeEmulationStatus>>;
    readonly viewportStyle$: Observable<object>;

    audioVolume = 0;

    private readonly _romFileSubject = new BehaviorSubject<TAgeRomFile | undefined>(undefined);
    private readonly _toolbarVisibility: AgeToolbarVisibility;

    private _showToolbar = true;
    private _hasEmulation = false;
    private _forcePause = false;
    private _isPaused = false;

    constructor(hostElementRef: ElementRef,
                emulationService: AgeEmulationService,
                breakpointObserver: AgeBreakpointObserverService,
                ngZone: NgZone,
                changeDetectorRef: ChangeDetectorRef) {
        super();

        this.emulationStatus$ = this._romFileSubject.asObservable().pipe(
            switchMap<TAgeRomFile | undefined, Observable<Partial<IAgeEmulationStatus>>>(
                romFile => !romFile ? of({}) : emulationService.newEmulation$(romFile),
            ),
            // if a new rom file has been loaded, clear the previous "is-paused" flag
            tap(emuStatus => {
                this._hasEmulation = !!emuStatus.emulation;
                // emulation ready -> clear "paused" flag
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

        this.newSubscription = this._toolbarVisibility.toolbarVisible$.subscribe(showToolbar => {
            this._showToolbar = showToolbar;
            changeDetectorRef.markForCheck();
        });
    }


    @HostBinding("class.hide-toolbar")
    get hideToolbar(): boolean {
        return !this._showToolbar && this._hasEmulation;
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


    mouseEnterLeaveViewport(mouseEvent: MouseEvent): void {
        this._toolbarVisibility.mouseEnterLeave(mouseEvent);
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
    };

    function cssPxString(value: number): string {
        return (value === 0) ? "0" : `${value}px`;
    }
}
