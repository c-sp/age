import {BrowserModule} from '@angular/platform-browser';
import {NgModule} from '@angular/core';

import {AppComponent} from './app.component';
import {AgeEmulatorModule} from './modules/emulator/age-emulator.module';
import {AgeRomFileSelectorModule} from './modules/rom-file-selector/age-rom-file-selector.module';
import {AgeLoaderModule} from './modules/loader/age-loader.module';


@NgModule({
    imports: [
        BrowserModule,
        AgeEmulatorModule,
        AgeLoaderModule,
        AgeRomFileSelectorModule
    ],
    declarations: [
        AppComponent
    ],
    providers: [],
    bootstrap: [AppComponent]
})
export class AppModule {
}
