//
// Copyright 2019 Christoph Sprenger
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

import {merge, Observable, of, Subject} from "rxjs";
import {delay, map, switchMap, tap} from "rxjs/operators";
import {AgeBreakpointObserverService} from "../../common";


export class AgeToolbarVisibility {

    private readonly _mouseSubject = new Subject<{}>();
    private readonly _clickSubject = new Subject<{}>();

    constructor(private readonly _breakpointObserver: AgeBreakpointObserverService) {
    }


    get toolbarVisible$(): Observable<boolean> {
        let toolbarVisible = true;

        // Check, if the user has the ability to hover over elements.
        // (can this value change e.g. by plugging in/out a mouse?)
        //
        // We check for the "hover" value,
        // so that devices without hover ability and browsers not
        // supporting the "hover" media query will fall back to
        // "click to show or hide the toolbar".
        return this._breakpointObserver.hasHoverAbility$.pipe(
            switchMap(hasHover => {

                // user has hover ability => show on mouse-move-event
                if (hasHover) {
                    return this._mouseSubject.asObservable().pipe(
                        map(() => true),
                    );
                }

                // user has no hover ability => listen to click events
                return this._clickSubject.asObservable().pipe(
                    map(() => !toolbarVisible),
                );
            }),

            // auto-hide the toolbar after some delay
            switchMap(showToolbar => showToolbar
                // show the toolbar and schedule hiding it later
                ? merge(of(true), of(false).pipe(delay(2000)))
                // just hide the toolbar
                : of(false),
            ),

            // store the current value
            tap(visible => toolbarVisible = visible),
        );
    }


    mouseMove(): void {
        this._mouseSubject.next();
    }

    click(): void {
        this._clickSubject.next({});
    }
}
