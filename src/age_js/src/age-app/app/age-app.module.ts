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

import {HashLocationStrategy, LocationStrategy} from "@angular/common";
import {NgModule} from "@angular/core";
import {BrowserModule} from "@angular/platform-browser";
import {RouterModule} from "@angular/router";
import {ServiceWorkerModule} from "@angular/service-worker";

import {FontAwesomeModule} from "@fortawesome/angular-fontawesome";
import {AgeLibModule} from "age-lib";
import {environment} from "../../environments/environment";
import {AgeAppComponent} from "./age-app.component";
import {AgeAppEmulatorComponent} from "./emulator/age-app-emulator.component";
import {AgeRomLibraryComponent, AgeRomLibraryContentsComponent, AgeRomLinkComponent} from "./rom-library";
import {AgeEmptyComponent, ROUTES} from "./routing";


@NgModule({
    imports: [
        BrowserModule,
        RouterModule.forRoot(ROUTES),
        ServiceWorkerModule.register("ngsw-worker.js", {enabled: environment.production}),
        FontAwesomeModule,
        AgeLibModule,
    ],
    declarations: [
        AgeAppEmulatorComponent,
        AgeRomLibraryComponent,
        AgeRomLibraryContentsComponent,
        AgeRomLinkComponent,
        AgeEmptyComponent,
        AgeAppComponent,
    ],
    providers: [
        {
            provide: LocationStrategy,
            useClass: HashLocationStrategy,
        },
    ],
    bootstrap: [AgeAppComponent],
})
export class AgeAppModule {
}
