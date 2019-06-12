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

import {ChangeDetectionStrategy, Component} from "@angular/core";
import {Router} from "@angular/router";
import {
    AgeEmulationFactoryService,
    AgeEmulationRunner,
    AgeSubscriptionSink,
    AgeTaskStatusHandlerService,
    IAgeOnlineRom,
    IAgeRomFileUrl,
} from "age-lib";
import {from, Observable} from "rxjs";
import {AgeLayoutService} from "./common/age-layout.service";


@Component({
    selector: "age-app-root",
    template: `
        <age-emulator [emulationRunner]="emulationRunner$ | async"></age-emulator>
        <age-rom-library (romClicked)="runRom($event)"></age-rom-library>
        <router-outlet></router-outlet>
    `,
    styles: [`
        :host {
            display: flex;
            height: 100%;
            max-width: 80em;
            margin-left: auto;
            margin-right: auto;
        }

        age-emulator {
            background-color: darkblue;
            flex: 1 1;
            min-width: 160px;
            min-height: 144px;
        }

        age-rom-library {
            flex: 4 4;
            align-self: flex-start;
            overflow: auto;
            height: 100%;
        }
    `],
    providers: [
        AgeTaskStatusHandlerService,
        AgeEmulationFactoryService,
    ],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeAppComponent extends AgeSubscriptionSink {

    private _emulationRunner$?: Observable<AgeEmulationRunner>;

    constructor( // hostElementRef: ElementRef,
                layoutService: AgeLayoutService,
                private readonly _emulationFactory: AgeEmulationFactoryService,
                private readonly _router: Router) {
        super();

        // this.newSubscription = new AgeResizeObserver(
        //     hostElementRef,
        //     entry => console.log(entry.contentRect),
        // );

        this.newSubscription = layoutService.layoutChange$.subscribe(layout => console.log("### layout", layout));
    }


    get emulationRunner$(): Observable<AgeEmulationRunner> | undefined {
        return this._emulationRunner$;
    }

    runRom(onlineRom: IAgeOnlineRom) {
        from(this._router.navigate(["url", onlineRom.romUrl])).subscribe();
        const romFileToLoad: IAgeRomFileUrl = {
            type: "rom-file-url",
            fileUrl: onlineRom.romUrl,
        };
        this._emulationRunner$ = this._emulationFactory.newEmulation$(romFileToLoad);
    }
}
