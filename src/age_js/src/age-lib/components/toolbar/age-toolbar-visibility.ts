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

import {merge, Observable, of, Subject} from 'rxjs';
import {delay, distinctUntilChanged, map, switchMap, tap} from 'rxjs/operators';
import {AgeBreakpointObserverService} from '../../common';


export class AgeToolbarVisibility {

    private readonly _mouseSubject = new Subject<{}>();
    private readonly _clickSubject = new Subject<{}>();
    private readonly _setVisibleSubject = new Subject<boolean>();
    private _toolbarVisible = true;

    constructor(private readonly _breakpointObserver: AgeBreakpointObserverService) {
    }


    get toolbarVisible$(): Observable<boolean> {
        // Check, if the user has the ability to hover over elements
        // (this can change during runtime e.g. by enabling/disabling the
        // "responsive view" in Chrome Developer Tools).
        //
        // We check for the "hover" value,
        // so that devices without hover ability and browsers not
        // supporting the "hover" media query will fall back to
        // "click to show or hide the toolbar".
        const eventBasedVisibility$ = this._breakpointObserver.hasHoverAbility$.pipe(
            switchMap(hasHover => hasHover
                // user has hover ability => show on mouse-move-event
                ? this._mouseOverVisibility$
                // user has no hover ability => listen to click events
                : this._clickVisibility$,
            ),
        );

        return merge(of(this._toolbarVisible), this._setVisibleSubject.asObservable()).pipe(
            switchMap(visible => {
                // If explicitly made visible, terminate all event-based visibility
                // (e.g. the scheduled hiding of the toolbar).
                if (visible) {
                    return of(true);
                }

                // If explicitly hidden, continue with event-based visibility.
                // We store the value before listening to event-based visibility,
                // as the latter may initialize with that value.
                this._toolbarVisible = false;
                return merge(of(this._toolbarVisible), eventBasedVisibility$);
            }),
            // filter unchanged values
            distinctUntilChanged(),
            // store the current value
            tap(visible => this._toolbarVisible = visible),
        );
    }


    setVisible(visible: boolean): void {
        this._setVisibleSubject.next(visible);
    }

    mouseMove(): void {
        this._mouseSubject.next();
    }

    click(): void {
        this._clickSubject.next();
    }


    private get _clickVisibility$(): Observable<boolean> {
        // visibility toggled by click event
        return this._clickSubject.asObservable().pipe(
            map(() => !this._toolbarVisible),
        );
    }

    private get _mouseOverVisibility$(): Observable<boolean> {
        return merge(
            // current visibility (may schedule hiding the toolbar)
            of(this._toolbarVisible),
            // visible by mouse-move event
            this._mouseSubject.asObservable().pipe(
                map(() => true),
            ),
        ).pipe(
            switchMap(toolbarVisible => toolbarVisible
                // show the toolbar and schedule hiding it
                ? merge(of(true), of(false).pipe(delay(3000)))
                // hide the toolbar
                : of(false),
            ),
        );
    }
}
