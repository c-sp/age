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
// diHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

import {ChangeDetectionStrategy, Component, Inject, ViewChild} from "@angular/core";
import {ActivatedRouteSnapshot, NavigationEnd, Router, UrlSegment} from "@angular/router";
import {AgeBreakpointObserverService, AgeSubscriptionSink, IAgeLocalRomFile, TAgeRomFile} from "age-lib";
import {combineLatest, Observable, Subject} from "rxjs";
import {filter, map, tap} from "rxjs/operators";
import {IAgeViewMode, ROUTE_FRAGMENT_URL, viewMode$} from "./common";
import {AgeAppEmulatorComponent} from "./emulator";
import {OPEN_LOCAL_ROM_FILE_SUBJECT} from "./rom-library";


@Component({
    selector: "age-app-root",
    template: `
        <ng-container *ngIf="(emuState$ | async) as emuState">

            <!--
             keep the emulation component alive even if it is not visible so that the user
             can always come back to it (e.g. after browsing the rom library)
              -->
            <age-app-emulator [forcePause]="emuState.hide"
                              [ngClass]="{
                                            'display-none': emuState.hide,
                                            'full-height': emuState.fullHeight
                                         }"></age-app-emulator>

            <router-outlet></router-outlet>
        </ng-container>
    `,
    styles: [`
        :host {
            display: block;
            height: 100%;
            overflow: auto;
        }

        age-app-emulator {
            /* size the emulator based on it's contents minimal size */
            min-height: min-content;
        }

        .full-height {
            height: 100%;
        }

        .display-none {
            display: none;
        }
    `],
    providers: [
        {
            provide: OPEN_LOCAL_ROM_FILE_SUBJECT,
            useValue: new Subject<IAgeLocalRomFile>(),
        },
    ],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeAppComponent extends AgeSubscriptionSink {

    readonly emuState$: Observable<IEmulationState>;

    @ViewChild(AgeAppEmulatorComponent, {static: false})
    private _emulatorComp?: AgeAppEmulatorComponent;

    constructor(breakpointObserverService: AgeBreakpointObserverService,
                router: Router,
                @Inject(OPEN_LOCAL_ROM_FILE_SUBJECT) localRomFileSubject: Subject<IAgeLocalRomFile>) {
        super();

        // TODO too late, initial route not parsed
        const openRomUrl$ = router.events.pipe(
            // only after successful navigation
            filter(event => event instanceof NavigationEnd),

            // get current route snapshot
            map(() => router.routerState.snapshot.root),

            // we collect all UrlSegments instead of just looking at the routeConfig
            // as the latter might contain "**" (redirected route)
            map(getUrlSegments),

            // open rom url
            tap(urlSegments => {
                if ((urlSegments.length > 1) && (urlSegments[0] === ROUTE_FRAGMENT_URL)) {
                    const fileUrl = urlSegments[1];
                    this.openRomFile({type: "rom-file-url", fileUrl});
                }
            }),
        );

        const _viewMode$ = viewMode$(breakpointObserverService);

        this.emuState$ = combineLatest([_viewMode$, openRomUrl$]).pipe(
            map<[IAgeViewMode, string[]], IEmulationState>(values => {
                const viewMode = values[0];
                const urlSegments = values[1];

                const hasChildComponent = !!urlSegments.length && (urlSegments[0] !== ROUTE_FRAGMENT_URL);
                return {
                    hide: hasChildComponent && viewMode.useMobileView,
                    fullHeight: !hasChildComponent,
                };
            }),
        );

        this.newSubscription = localRomFileSubject.asObservable().subscribe(
            localRomFile => this.openRomFile(localRomFile),
        );
    }

    openRomFile(romFile: TAgeRomFile): void {
        if (this._emulatorComp) {
            this._emulatorComp.openRomFile(romFile);
        }
    }
}


interface IEmulationState {
    readonly hide: boolean;
    readonly fullHeight: boolean;
}


function getUrlSegments(routeSnapshot: ActivatedRouteSnapshot | null): string[] {
    const urlSegments = new Array<UrlSegment>();

    while (routeSnapshot) {
        urlSegments.push(...routeSnapshot.url);
        routeSnapshot = routeSnapshot.firstChild;
    }

    return urlSegments
        .filter(urlSegment => !!urlSegment.path)
        .map(urlSegment => urlSegment.path);
}
