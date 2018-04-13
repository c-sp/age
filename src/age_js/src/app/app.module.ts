import {BrowserModule} from '@angular/platform-browser';
import {NgModule} from '@angular/core';

import {AppComponent} from './app.component';
import {AgeEmulatorModule} from './emulator/age-emulator.module';
import {AgeRomFileSelectorModule} from './rom-file-selector/age-rom-file-selector.module';


@NgModule({
    imports: [
        BrowserModule,
        AgeEmulatorModule,
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
