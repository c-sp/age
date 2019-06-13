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
import {Event, NavigationEnd, Router} from "@angular/router";
import {AgeSubscriptionSink} from "age-lib";
import {Observable, Subject} from "rxjs";
import {filter, map, shareReplay} from "rxjs/operators";
import {ROUTE_FRAGMENT_LIBRARY, ROUTE_FRAGMENT_URL, ROUTE_PARAM_ROM_URL} from "./age-routes";


export type TAgeRoute = IAgeRouteRoot | IAgeRouteLibrary | IAgeRouteRomUrl;

export interface IAgeRouteRoot {
    readonly route: "root";
}

export interface IAgeRouteLibrary {
    readonly route: "library";
}

export interface IAgeRouteRomUrl {
    readonly route: "romUrl";
    readonly romUrl: string | null;
}

@Injectable({
    providedIn: "root",
})
export class AgeCurrentRouteService extends AgeSubscriptionSink {

    private readonly _currentRouteSubject = new Subject<TAgeRoute>();
    private readonly _currentRoute$ = this._currentRouteSubject.asObservable().pipe(shareReplay(1));

    constructor(private readonly _router: Router) {
        super();

        this.newSubscription = this._router.events
            .pipe(
                // only after successful navigation
                filter(event => event instanceof NavigationEnd),

                // process current route
                map<Event, TAgeRoute>(() => {
                    const rootRoute = this._router.routerState.snapshot.root;

                    if (rootRoute.firstChild) {
                        const childPath = rootRoute.firstChild.routeConfig && rootRoute.firstChild.routeConfig.path;

                        // at the library
                        if (childPath === ROUTE_FRAGMENT_LIBRARY) {
                            return {route: "library"};
                        }

                        // rom url
                        if (childPath === ROUTE_FRAGMENT_URL) {
                            const urlRoute = rootRoute.firstChild;

                            // rom url specified?
                            const romUrl = urlRoute.firstChild
                                && urlRoute.firstChild.paramMap.get(ROUTE_PARAM_ROM_URL);

                            return {route: "romUrl", romUrl};
                        }
                    }

                    // treat everything else as root
                    return {route: "root"};
                }),
            )
            .subscribe(route => this._currentRouteSubject.next(route));
    }

    get currentRoute$(): Observable<TAgeRoute> {
        return this._currentRoute$;
    }
}
