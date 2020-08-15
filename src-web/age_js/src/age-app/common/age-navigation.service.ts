//
// Copyright 2020 Christoph Sprenger
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

import {Injectable} from '@angular/core';
import {Router} from '@angular/router';
import {from} from 'rxjs';


export const ROUTE_FRAGMENT_ABOUT_AGE = 'about';
export const ROUTE_FRAGMENT_OPEN_ROM = 'open-rom';
export const ROUTE_FRAGMENT_ROM_URL = 'rom-url';
export const ROUTE_PARAM_ROM_URL = 'romUrl';


@Injectable({
    providedIn: 'root',
})
export class AgeNavigationService {

    readonly aboutAgeUrl = `/${ROUTE_FRAGMENT_ABOUT_AGE}`;
    readonly openRomUrl = `/${ROUTE_FRAGMENT_OPEN_ROM}`;
    readonly rootUrl = '/';

    constructor(private readonly _router: Router) {
    }

    navigateToRoot(): void {
        this._navigateTo(['']);
    }

    navigateToRomUrl(romUrl: string): void {
        this._navigateTo([ROUTE_FRAGMENT_ROM_URL, romUrl]);
    }

    // noinspection JSMethodCanBeStatic
    romUrlRouterLink(romUrl: string): string[] {
        return ['/', ROUTE_FRAGMENT_ROM_URL, romUrl];
    }

    private _navigateTo(commands: string[]): void {
        from(this._router.navigate(commands)).subscribe(/* we don't care for the result */);
    }
}
