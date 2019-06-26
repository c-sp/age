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

import {ChangeDetectionStrategy, ChangeDetectorRef, Component, HostBinding, OnInit, ViewChild} from "@angular/core";
import {AgeSubscriptionSink, TAgeRomFile} from "age-lib";
import {AgeView, AgeViewService} from "./age-view.service";
import {AgeAppEmulatorComponent} from "./emulator";
import {AgeCurrentRouteService} from "./routing";


@Component({
    selector: "age-app-root",
    template: `
        <age-app-emulator [forcePause]="showOnlyLibrary"></age-app-emulator>

        <age-rom-library (openLocalRomFile)="openRomFile($event)"
                         [mobileMode]="showOnlyLibrary"></age-rom-library>
    `,
    styles: [`
        :host {
            display: block;
            height: 100%;
            overflow: auto;
        }

        age-app-emulator {
            /*
             inner elements nicht have a min-height we should take into account
             when setting any height
              */
            min-height: min-content;
        }

        :host.only-emulator age-app-emulator {
            height: 100%;
        }

        :host.only-emulator age-rom-library {
            display: none;
        }

        :host.only-library age-app-emulator {
            display: none;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeAppComponent extends AgeSubscriptionSink implements OnInit {

    @ViewChild(AgeAppEmulatorComponent, {static: true})
    private _emulatorComp?: AgeAppEmulatorComponent;
    private _view = AgeView.COMBINED;

    constructor(private readonly _currentRouteService: AgeCurrentRouteService,
                private readonly _viewService: AgeViewService,
                private readonly _changeDetectorRef: ChangeDetectorRef) {
        super();
    }

    ngOnInit(): void {
        this.newSubscription = this._viewService.viewChange$.subscribe(view => {
            this._view = view;
            this._changeDetectorRef.markForCheck();
        });
        this.newSubscription = this._currentRouteService.currentRoute$.subscribe(route => {
            const fileUrl = (route.route === "romUrl") && route.romUrl;
            if (fileUrl) {
                this.openRomFile({type: "rom-file-url", fileUrl});
            }
        });
    }


    @HostBinding("class.only-emulator") get showOnlyEmulator() {
        return this._view === AgeView.ONLY_EMULATOR;
    }

    @HostBinding("class.only-library") get showOnlyLibrary() {
        return this._view === AgeView.ONLY_LIBRARY;
    }


    openRomFile(romFile: TAgeRomFile): void {
        if (this._emulatorComp) {
            this._emulatorComp.openRomFile(romFile);
        }
    }
}
