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

import {AgeBreakpointObserverService} from 'age-lib';
import {combineLatest, Observable} from 'rxjs';
import {map, shareReplay} from 'rxjs/operators';


export const MOBILE_WIDTH_PX = 480;
export const MOBILE_HEIGHT_PX = 580;

export interface IAgeViewMode {
    readonly mobileWidth: boolean;
    readonly mobileHeight: boolean;
    readonly mobileView: boolean;
}

let VIEW_MODE$: Observable<IAgeViewMode> | undefined;

export function viewMode$(breakpointObserverService: AgeBreakpointObserverService): Observable<IAgeViewMode> {
    return VIEW_MODE$ || (VIEW_MODE$ = _viewMode$(breakpointObserverService));
}

export function _viewMode$(breakpointObserverService: AgeBreakpointObserverService): Observable<IAgeViewMode> {
    return combineLatest([
        //
        // Purpose of the breakpoints is to prevent the combined view
        // (current route including current emulation) on any viewport that is
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
        breakpointObserverService.matches$(`(min-width: ${MOBILE_WIDTH_PX}px)`),
        breakpointObserverService.matches$(`(min-height: ${MOBILE_HEIGHT_PX}px)`),
    ]).pipe(
        map<[boolean, boolean], IAgeViewMode>(values => {
            const mobileWidth = !values[0];
            const mobileHeight = !values[1];
            return {
                mobileWidth,
                mobileHeight,
                mobileView: mobileWidth || mobileHeight,
            };
        }),
        // cache the result for all subscriptions
        shareReplay(1),
    );
}
