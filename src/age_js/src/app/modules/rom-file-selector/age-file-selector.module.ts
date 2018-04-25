import {NgModule} from '@angular/core';
import {CommonModule} from '@angular/common';
import {AgeFileSelectorLocalComponent} from './age-file-selector-local.component';
import {AgeFileSelectorURLComponent} from './age-file-selector-url.component';
import {HttpClientModule} from '@angular/common/http';
import {AgeFileSelectorComponent} from './age-file-selector.component';


@NgModule({
    imports: [
        CommonModule,
        HttpClientModule
    ],
    declarations: [
        AgeFileSelectorComponent,
        AgeFileSelectorLocalComponent,
        AgeFileSelectorURLComponent
    ],
    exports: [
        AgeFileSelectorComponent
    ],
    providers: []
})
export class AgeFileSelectorModule {
}
