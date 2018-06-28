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

import {BrowserModule} from '@angular/platform-browser';
import {NgModule} from '@angular/core';

import {AgeAppComponent} from './age-app.component';
import {AgeEmulatorModule} from './modules/emulator/age-emulator.module';
import {AgeLoaderModule} from './modules/loader/age-loader.module';
import {AgeTitleBarModule} from './modules/title-bar/age-title-bar.module';
import {AgeInfoModule} from './modules/info/age-info.module';
import {AgeOpenRomModule} from './modules/open-rom/age-open-rom.module';


@NgModule({
    imports: [
        BrowserModule,
        AgeInfoModule,
        AgeEmulatorModule,
        AgeLoaderModule,
        AgeOpenRomModule,
        AgeTitleBarModule
    ],
    declarations: [
        AgeAppComponent
    ],
    bootstrap: [AgeAppComponent]
})
export class AgeAppModule {
}
