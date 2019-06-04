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

import {CommonModule} from "@angular/common";
import {HttpClientModule} from "@angular/common/http";
import {NgModule} from "@angular/core";
import {FontAwesomeModule} from "@fortawesome/angular-fontawesome";
import {AgeOpenRomLocalComponent} from "./age-open-rom-local.component";
import {AgeOpenRomUrlComponent} from "./age-open-rom-url.component";
import {AgeOpenRomComponent} from "./age-open-rom.component";


@NgModule({
    imports: [
        CommonModule,
        HttpClientModule,
        FontAwesomeModule,
    ],
    declarations: [
        AgeOpenRomComponent,
        AgeOpenRomLocalComponent,
        AgeOpenRomUrlComponent,
    ],
    exports: [
        AgeOpenRomComponent,
    ],
})
export class AgeOpenRomModule {
}
