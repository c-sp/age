import {NgModule} from '@angular/core';
import {AgeEmulatorContainerComponent} from './age-emulator-container.component';
import {AgeEmulatorComponent} from './age-emulator.component';
import {AgeWasmLoaderComponent} from './loader/age-wasm-loader.component';
import {CommonModule} from '@angular/common';
import {AgeLoaderErrorDirective, AgeLoaderSuccessDirective, AgeLoaderWorkingDirective} from './loader/age-loader-label.directive';
import {AgeLoaderStateComponent} from './loader/age-loader-state.component';
import {AgeRomFileLoaderComponent} from './loader/age-rom-file-loader.component';


@NgModule({
    imports: [
        CommonModule
    ],
    declarations: [
        // ./emulator/loader
        AgeLoaderWorkingDirective,
        AgeLoaderSuccessDirective,
        AgeLoaderErrorDirective,
        AgeLoaderStateComponent,
        AgeRomFileLoaderComponent,
        AgeWasmLoaderComponent,

        // ./emulator
        AgeEmulatorComponent,
        AgeEmulatorContainerComponent
    ],
    exports: [
        AgeEmulatorContainerComponent
    ],
    providers: []
})
export class AgeEmulatorModule {
}
