import {NgModule} from '@angular/core';
import {CommonModule} from '@angular/common';
import {AgeLocalFileSelectorComponent} from './age-local-file-selector.component';


@NgModule({
    imports: [
        CommonModule
    ],
    declarations: [
        AgeLocalFileSelectorComponent
    ],
    exports: [
        AgeLocalFileSelectorComponent
    ],
    providers: []
})
export class AgeRomFileSelectorModule {
}
