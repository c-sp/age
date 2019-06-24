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
import {AgeBreakpointObserverService, AgeSubscriptionSink} from "age-lib";
import {BehaviorSubject, combineLatest, Observable, Subject} from "rxjs";
import {map, shareReplay} from "rxjs/operators";
import {AgeCurrentRouteService} from "./routing";


export enum AgeView {
    ONLY_LIBRARY = "ONLY_LIBRARY",
    ONLY_EMULATOR = "ONLY_EMULATOR",
    COMBINED_FOCUS_LIBRARY = "COMBINED_FOCUS_LIBRARY",
    COMBINED_FOCUS_EMULATOR = "COMBINED_FOCUS_EMULATOR",
    // TODO add EMULATOR_FULLSCREEN
}

export type TAgeFocusElement = "emulator" | "library";


@Injectable({
    providedIn: "root",
})
export class AgeViewService extends AgeSubscriptionSink {

    private readonly _viewChangeSubject = new Subject<AgeView>();
    private readonly _viewChange$ = this._viewChangeSubject.asObservable().pipe(shareReplay(1));

    private readonly _attributeChangeSubject = new BehaviorSubject<{}>({});
    private _focusElement: TAgeFocusElement = "library";

    constructor(breakpointObserver: AgeBreakpointObserverService,
                currentRouteService: AgeCurrentRouteService) {
        super();

        const events$ = combineLatest([
            //
            // Purpose of the breakpoints is to prevent the combined view
            // (emulator and rom library both visible) on any viewport that is
            // too small, e.g. mobile phones.
            //
            // For a list of devices and their viewport have a look at
            // Material Design's device metrics:
            // https://material.io/tools/devices/
            //
            // These metrics suggest to use 4 "columns" for screens with
            // a width of less than 480dp and to use 8 or more "columns" for
            // screens wider than that.
            // Using 480px (a CSS pixel is device independent just like dp)
            // as the breakpoint for allowing the combined view seems sensible.
            //
            breakpointObserver.matches$("(min-width: 480px)"),
            breakpointObserver.matches$("(min-height: 480px)"),
            currentRouteService.currentRoute$,
            this._attributeChangeSubject.asObservable(),
        ]);

        this.newSubscription = events$
            .pipe(
                map(values => {
                    const widthSufficient = values[0];
                    const heightSufficient = values[1];
                    const route = values[2];

                    // minimal view: either library or emulator
                    if (!widthSufficient || !heightSufficient) {
                        return (route.route === "library") ? AgeView.ONLY_LIBRARY : AgeView.ONLY_EMULATOR;
                    }

                    // combined view
                    return (this._focusElement === "library")
                        ? AgeView.COMBINED_FOCUS_LIBRARY
                        : AgeView.COMBINED_FOCUS_EMULATOR;
                }),
            )
            .subscribe(view => this._viewChangeSubject.next(view));
    }


    get viewChange$(): Observable<AgeView> {
        return this._viewChange$;
    }

    toggleFocusElement(): void {
        this._focusElement = (this._focusElement === "library") ? "emulator" : "library";
        this._attributeChangeSubject.next({});
    }
}
