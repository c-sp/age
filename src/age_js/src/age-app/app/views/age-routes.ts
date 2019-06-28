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

import {Route, Routes} from '@angular/router';
import {ROUTE_FRAGMENT_ABOUT_AGE, ROUTE_FRAGMENT_LIBRARY, ROUTE_FRAGMENT_ROM_URL, ROUTE_PARAM_ROM_URL} from '../common';
import {AgeAboutComponent} from './about';
import {AgeRomLibraryComponent} from './rom-library';
import {AgeRomUrlComponent} from './rom-url';


const REDIRECT_TO_ROOT: Route = {
    path: '**',
    redirectTo: '',
};


export const ROUTES: Routes = [
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
        path: `${ROUTE_FRAGMENT_ROM_URL}/:${ROUTE_PARAM_ROM_URL}`,
        component: AgeRomUrlComponent,
        // redirect all child routes
        children: [REDIRECT_TO_ROOT],
    },
    // redirect all other routes to "/"
    REDIRECT_TO_ROOT,
];
