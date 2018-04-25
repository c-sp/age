import {NgModule} from '@angular/core';
import {AgeLoaderErrorDirective, AgeLoaderSuccessDirective, AgeLoaderWorkingDirective} from './age-loader-label.directive';
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
