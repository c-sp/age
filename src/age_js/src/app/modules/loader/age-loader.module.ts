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

import {NgModule} from '@angular/core';
import {
    AgeLoaderErrorDirective,
    AgeLoaderNoStateDirective,
    AgeLoaderSuccessDirective,
    AgeLoaderWorkingDirective
} from './age-loader-label.directive';
import {AgeLoaderStateComponent} from './age-loader-state.component';
import {AgeRomFileLoaderComponent} from './age-rom-file-loader.component';
import {AgeWasmLoaderComponent} from './age-wasm-loader.component';
import {CommonModule} from '@angular/common';
import {AgeLoaderComponent} from './age-loader.component';
import {AgeRomFileExtractorComponent} from './age-rom-file-extractor.component';


@NgModule({
    imports: [
        CommonModule
    ],
    declarations: [
        AgeLoaderComponent,
        AgeLoaderErrorDirective,
        AgeLoaderSuccessDirective,
        AgeLoaderWorkingDirective,
        AgeLoaderNoStateDirective,
        AgeLoaderStateComponent,
        AgeRomFileExtractorComponent,
        AgeRomFileLoaderComponent,
        AgeWasmLoaderComponent
    ],
    exports: [
        AgeLoaderComponent
    ]
})
export class AgeLoaderModule {
}
