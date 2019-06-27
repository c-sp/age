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

import {BreakpointObserver} from '@angular/cdk/layout';
import {Injectable} from '@angular/core';
import {Observable} from 'rxjs';
import {map, shareReplay} from 'rxjs/operators';


@Injectable({
    providedIn: 'root',
})
export class AgeBreakpointObserverService {

    readonly hasHoverAbility$: Observable<boolean>;

    constructor(private readonly _breakpointObserver: BreakpointObserver) {
        this.hasHoverAbility$ = this.matches$('(hover: hover)').pipe(
            // cache and share the result as this media query is used quite often
            shareReplay(1),
        );
    }

    matches$(mediaQuery: string): Observable<boolean> {
        return this._breakpointObserver.observe(mediaQuery).pipe(
            map(breakpointState => breakpointState.matches),
        );
    }
}
