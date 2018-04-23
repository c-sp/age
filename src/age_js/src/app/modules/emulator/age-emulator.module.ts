import {NgModule} from '@angular/core';
import {AgeEmulatorComponent} from './age-emulator.component';
import {CommonModule} from '@angular/common';
import {AgeCanvasRendererModule} from './renderer/canvas/age-canvas-renderer.module';


@NgModule({
    imports: [
        CommonModule,
        AgeCanvasRendererModule
    ],
    declarations: [
        AgeEmulatorComponent
    ],
    exports: [
        AgeEmulatorComponent
    ],
    providers: []
})
export class AgeEmulatorModule {
}
