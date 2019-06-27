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

import {HashLocationStrategy, LocationStrategy} from '@angular/common';
import {NgModule} from '@angular/core';
import {MatButtonModule} from '@angular/material/button';
import {MatIconModule} from '@angular/material/icon';
import {MatMenuModule} from '@angular/material/menu';
import {MatToolbarModule} from '@angular/material/toolbar';
import {BrowserModule} from '@angular/platform-browser';
import {BrowserAnimationsModule} from '@angular/platform-browser/animations';
import {Route, RouterModule, Routes} from '@angular/router';
import {ServiceWorkerModule} from '@angular/service-worker';
import {AgeLibModule} from 'age-lib';
import {environment} from '../../environments/environment';
import {AgeAboutComponent} from './about';
import {AgeAppComponent} from './age-app.component';
import {
    AgeEmptyComponent,
    AgeHrefDirective,
    ROUTE_FRAGMENT_ABOUT_AGE,
    ROUTE_FRAGMENT_LIBRARY,
    ROUTE_FRAGMENT_URL,
    ROUTE_PARAM_ROM_URL,
} from './common';
import {AgeAppEmulatorComponent} from './emulator';
import {
    AgeRomLibraryComponent,
    AgeRomLibraryContentsComponent,
    AgeRomLinkComponent,
    AgeToolbarActionLocalRomComponent,
} from './rom-library';


const REDIRECT_TO_ROOT: Route = {
    path: '**',
    redirectTo: '',
};

const ROUTES: Routes = [
    {
        path: ROUTE_FRAGMENT_ABOUT_AGE,
        component: AgeAboutComponent,
        // redirect all child routes
        children: [REDIRECT_TO_ROOT],
    },
    {
        path: ROUTE_FRAGMENT_LIBRARY,
        component: AgeRomLibraryComponent,
        // redirect all child routes
        children: [REDIRECT_TO_ROOT],
    },
    {
        path: ROUTE_FRAGMENT_URL,
        component: AgeEmptyComponent,
        children: [
            {
                path: `:${ROUTE_PARAM_ROM_URL}`,
                component: AgeEmptyComponent,
                // redirect all child routes
                children: [REDIRECT_TO_ROOT],
            },
        ],
    },
    // redirect all other routes to "/"
    REDIRECT_TO_ROOT,
];


@NgModule({
    imports: [
        BrowserModule,
        BrowserAnimationsModule,
        RouterModule.forRoot(ROUTES),
        ServiceWorkerModule.register('ngsw-worker.js', {enabled: environment.production}),

        MatButtonModule,
        MatIconModule,
        MatMenuModule,
        MatToolbarModule,

        AgeLibModule,
    ],
    declarations: [
        AgeAboutComponent,

        AgeEmptyComponent,
        AgeHrefDirective,
        AgeAppEmulatorComponent,

        AgeRomLibraryComponent,
        AgeRomLibraryContentsComponent,
        AgeRomLinkComponent,
        AgeToolbarActionLocalRomComponent,

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
