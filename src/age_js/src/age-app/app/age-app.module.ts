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

import {NgModule} from "@angular/core";
import {BrowserModule} from "@angular/platform-browser";
import {Route, RouterModule, Routes} from "@angular/router";
import {ServiceWorkerModule} from "@angular/service-worker";

import {FontAwesomeModule} from "@fortawesome/angular-fontawesome";
import {AgeLibModule} from "age-lib";
import {environment} from "../../environments/environment";
import {AgeAppComponent} from "./age-app.component";
import {AgeRouteLibraryComponent} from "./library/age-route-library.component";
import {AgeRouteUrlComponent, URL_PARAM} from "./url/age-route-url.component";


const redirectToRoot: Route = {
    path: "**",
    redirectTo: "",
};

const routes: Routes = [
    {
        path: "library",
        component: AgeRouteLibraryComponent,
        children: [redirectToRoot],
    },
    {
        path: "url",
        pathMatch: "full",
        component: AgeRouteUrlComponent,
    },
    {
        path: `url/:${URL_PARAM}`,
        component: AgeRouteUrlComponent,
        children: [redirectToRoot],
    },
    redirectToRoot,
];


@NgModule({
    imports: [
        BrowserModule,
        RouterModule.forRoot(routes),
        ServiceWorkerModule.register("ngsw-worker.js", {enabled: environment.production}),

        FontAwesomeModule,

        AgeLibModule,
    ],
    declarations: [
        AgeRouteLibraryComponent,
        AgeRouteUrlComponent,
        AgeAppComponent,
    ],
    bootstrap: [AgeAppComponent],
})
export class AgeAppModule {
}
