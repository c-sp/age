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

import {BreakpointObserver} from "@angular/cdk/layout";
import {Injectable} from "@angular/core";
import {AgeSubscriptionSink} from "age-lib";
import {BehaviorSubject, combineLatest, Observable, Subject} from "rxjs";
import {map, shareReplay, tap} from "rxjs/operators";
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

    constructor(breakpointObserver: BreakpointObserver,
                currentRouteService: AgeCurrentRouteService) {
        super();

        const events$ = combineLatest([
            //
            // Purpose of the breakpoints is to prevent the combined view
            // (both emulator and library) on any viewport that is too small
            // like e.g. mobile phones.
            //
            // For a list of devices and their viewport have a look at:
            // https://material.io/tools/devices/
            //
            // According to that site the biggest mobile phones have a viewport
            // of 480px.
            // We thus choose 31em (496px @ font-size:16px) in each dimension
            // as minimal requirement to display the combined view.
            //
            breakpointObserver.observe("(min-width: 31em)"),
            breakpointObserver.observe("(min-height: 31em)"),
            currentRouteService.currentRoute$,
            this._attributeChangeSubject.asObservable(),
        ]);

        this.newSubscription = events$
            .pipe(
                map(values => {
                    const widthSufficient = values[0].matches;
                    const heightSufficient = values[1].matches;
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
                tap(view => console.log("view", view)),
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
