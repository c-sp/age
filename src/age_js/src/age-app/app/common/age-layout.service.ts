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
import {Observable, Subject} from "rxjs";
import {shareReplay} from "rxjs/operators";


export interface ILayout {
    readonly minimal: boolean;
    readonly layout: AgeLayout;
}

export enum AgeLayout {
    /**
     * The library is the only visible component.
     *
     * (only when the `library` route is active with minimal layout)
     */
    LIBRARY_ONLY = "LIBRARY_ONLY",

    /**
     * The emulator is the only visible component.
     *
     * (for minimal layout only when the `library` route is inactive,
     * optional for other layouts)
     */
    EMULATOR_ONLY = "EMULATOR_ONLY",

    /**
     * Emulator component and library component are both visible.
     *
     * (only for non-minimal layouts)
     */
    EMULATOR_AND_LIBRARY = "EMULATOR_AND_LIBRARY",
}


@Injectable({
    providedIn: "root",
})
export class AgeLayoutService extends AgeSubscriptionSink {

    private readonly _layoutChangeSubject = new Subject<ILayout>();
    private readonly _layoutChange$ = this._layoutChangeSubject.asObservable().pipe(shareReplay(1));
    /**
     * indicates a minimal layout designed for mobile phones
     * or a very narrow browser window
     */
    private _minimalLayout = true;
    private _libraryRouteActive = false;
    private _maximizeEmulator = false;

    constructor(breakpointObserver: BreakpointObserver) {
        super();
        this.newSubscription = breakpointObserver
            .observe("(min-width: 32em)") // 512px @ font-size:16px
            .subscribe(breakpointState => {
                this._minimalLayout = !breakpointState.matches;
                this._updateLayout();
            });
    }


    get layoutChange$(): Observable<ILayout> {
        return this._layoutChange$;
    }

    set libraryRouteActive(libraryRouteActive: boolean) {
        this._libraryRouteActive = libraryRouteActive;
        this._updateLayout();
    }

    set maximizeEmulator(maximizeEmulator: boolean) {
        this._maximizeEmulator = maximizeEmulator;
        this._updateLayout();
    }


    private _updateLayout(): void {
        const layout = (this._minimalLayout && this._libraryRouteActive)
            ? AgeLayout.LIBRARY_ONLY
            : (!this._minimalLayout && !this._maximizeEmulator)
                ? AgeLayout.EMULATOR_AND_LIBRARY
                : AgeLayout.EMULATOR_ONLY;

        const currentLayout: ILayout = {
            minimal: this._minimalLayout,
            layout,
        };
        this._layoutChangeSubject.next(currentLayout);
    }
}
