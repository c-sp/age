import {BrowserModule} from '@angular/platform-browser';
import {NgModule} from '@angular/core';

import {AppComponent} from './app.component';
import {AgeEmulatorModule} from './modules/emulator/age-emulator.module';
import {AgeFileSelectorModule} from './modules/rom-file-selector/age-file-selector.module';
import {AgeLoaderModule} from './modules/loader/age-loader.module';


@NgModule({
    imports: [
        BrowserModule,
        AgeEmulatorModule,
        AgeLoaderModule,
        AgeFileSelectorModule
    ],
    declarations: [
        AppComponent
    ],
    providers: [],
    bootstrap: [AppComponent]
})
export class AppModule {
}
