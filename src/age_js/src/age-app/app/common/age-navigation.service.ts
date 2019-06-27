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

import {Injectable} from "@angular/core";
import {Router} from "@angular/router";
import {from} from "rxjs";


export const ROUTE_FRAGMENT_ABOUT_AGE = "about";
export const ROUTE_FRAGMENT_LIBRARY = "library";
export const ROUTE_FRAGMENT_URL = "url";
export const ROUTE_PARAM_ROM_URL = "romUrl";


@Injectable({
    providedIn: "root",
})
export class AgeNavigationService {

    readonly aboutAgeUrl = `/${ROUTE_FRAGMENT_ABOUT_AGE}`;
    readonly libraryUrl = `/${ROUTE_FRAGMENT_LIBRARY}`;
    readonly rootUrl = "/";

    constructor(private readonly _router: Router) {
    }


    navigateToOpenRomUrl(romUrl?: string): void {
        this._navigateTo(romUrl ? [ROUTE_FRAGMENT_URL, romUrl] : [ROUTE_FRAGMENT_URL]);
    }

    navigateToRoot(): void {
        this._navigateTo([""]);
    }


    private _navigateTo(commands: string[]): void {
        from(this._router.navigate(commands)).subscribe(/* we don't care for the result */);
    }
}
