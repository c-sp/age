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
import {FormsModule} from '@angular/forms';
import {MatButtonModule} from '@angular/material/button';
import {MatCardModule} from '@angular/material/card';
import {MatIconModule} from '@angular/material/icon';
import {MatInputModule} from '@angular/material/input';
import {MatMenuModule} from '@angular/material/menu';
import {MatToolbarModule} from '@angular/material/toolbar';
import {BrowserModule} from '@angular/platform-browser';
import {BrowserAnimationsModule} from '@angular/platform-browser/animations';
import {RouterModule} from '@angular/router';
import {ServiceWorkerModule} from '@angular/service-worker';
import {AgeLibModule} from 'age-lib';
import {environment} from '../../environments/environment';
import {AgeAppEmulatorComponent} from './age-app-emulator.component';
import {AgeAppComponent} from './age-app.component';
import {AgeHrefDirective} from './common';
import {
    AgeAboutComponent,
    AgeOpenRomComponent,
    AgeRomLibraryComponent,
    AgeRomUrlComponent,
    AgeToolbarActionLocalRomComponent,
    ROUTES,
} from './views';


@NgModule({
    imports: [
        BrowserModule,
        BrowserAnimationsModule,
        FormsModule, // [(ngModel)]
        RouterModule.forRoot(ROUTES),
        ServiceWorkerModule.register('ngsw-worker.js', {enabled: environment.production}),

        MatButtonModule,
        MatCardModule,
        MatIconModule,
        MatInputModule,
        MatMenuModule,
        MatToolbarModule,

        AgeLibModule,
    ],
    declarations: [
        AgeHrefDirective,

        AgeAboutComponent,

        AgeOpenRomComponent,
        AgeRomLibraryComponent,
        AgeToolbarActionLocalRomComponent,

        AgeRomUrlComponent,

        AgeAppComponent,
        AgeAppEmulatorComponent,
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
