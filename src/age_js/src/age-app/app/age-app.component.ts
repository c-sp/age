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

import {ChangeDetectionStrategy, ChangeDetectorRef, Component, HostBinding} from "@angular/core";
import {AgeSubscriptionSink} from "age-lib";
import {Observable} from "rxjs";
import {map} from "rxjs/operators";
import {AgeView, AgeViewService} from "./age-view.service";
import {AgeCurrentRouteService} from "./routing";


@Component({
    selector: "age-app-root",
    template: `
        <age-app-emulator [romUrl]="romUrl$ | async"></age-app-emulator>
        <age-app-rom-library></age-app-rom-library>
    `,
    styles: [`
        :host {
            display: block;
            height: 100%;
            overflow: auto;
        }

        :host.only-emulator age-app-emulator {
            height: 100%;
        }

        :host.only-emulator age-app-rom-library {
            display: none;
        }

        :host.only-library age-app-emulator {
            display: none;
        }

        :host.focus-emulator age-app-emulator {
            height: 80%;
        }

        :host.focus-library age-app-emulator {
            height: 30%;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeAppComponent extends AgeSubscriptionSink {

    readonly romUrl$: Observable<string | undefined>;

    private _view = AgeView.COMBINED_FOCUS_EMULATOR;

    constructor(viewService: AgeViewService,
                currentRouteService: AgeCurrentRouteService,
                changeDetectorRef: ChangeDetectorRef) {
        super();

        this.newSubscription = viewService.viewChange$.subscribe(view => {
            this._view = view;
            changeDetectorRef.markForCheck();
        });

        this.romUrl$ = currentRouteService.currentRoute$.pipe(
            map(route => {
                const romUrl = (route.route === "romUrl") && route.romUrl;
                return romUrl ? romUrl : undefined;
            }),
        );
    }

    @HostBinding("class.only-emulator") get cssOnlyEmulator() {
        return this._view === AgeView.ONLY_EMULATOR;
    }

    @HostBinding("class.only-library") get cssOnlyLibrary() {
        return this._view === AgeView.ONLY_LIBRARY;
    }

    @HostBinding("class.focus-emulator") get cssFocusEmulator() {
        return this._view === AgeView.COMBINED_FOCUS_EMULATOR;
    }

    @HostBinding("class.focus-library") get cssFocusLibrary() {
        return this._view === AgeView.COMBINED_FOCUS_LIBRARY;
    }
}
