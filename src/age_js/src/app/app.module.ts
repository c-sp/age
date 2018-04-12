import {BrowserModule} from '@angular/platform-browser';
import {NgModule} from '@angular/core';

import {AppComponent} from './app.component';
import {AgeEmulatorModule} from './emulator/age-emulator.module';


@NgModule({
    imports: [
        BrowserModule,
        AgeEmulatorModule
    ],
    declarations: [
        AppComponent
    ],
    providers: [],
    bootstrap: [AppComponent]
})
export class AppModule {
}
