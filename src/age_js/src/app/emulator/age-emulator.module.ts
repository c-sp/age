import {NgModule} from '@angular/core';
import {AgeEmulatorContainerComponent} from './age-emulator-container.component';
import {AgeLoaderStateIconComponent} from './loader/age-loader-state-icon.component';
import {AgeEmulatorComponent} from './age-emulator.component';
import {AgeWasmLoaderComponent} from './loader/age-wasm-loader.component';
import {CommonModule} from '@angular/common';


@NgModule({
    imports: [
        CommonModule
    ],
    declarations: [
        AgeEmulatorContainerComponent,
        AgeLoaderStateIconComponent,
        AgeWasmLoaderComponent,
        AgeEmulatorComponent
    ],
    exports: [
        AgeEmulatorContainerComponent
    ],
    providers: []
})
export class AgeEmulatorModule {
}
