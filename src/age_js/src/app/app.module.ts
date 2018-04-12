import {BrowserModule} from '@angular/platform-browser';
import {NgModule} from '@angular/core';

import {AppComponent} from './app.component';
import {AgeEmulatorContainerComponent} from './emulator/emulator-container.component';
import {AgeTaskStateComponent} from './emulator/task-state.component';
import {AgeEmulatorComponent} from './emulator/emulator.component';


@NgModule({
    declarations: [
        AppComponent,
        AgeEmulatorContainerComponent,
        AgeTaskStateComponent,
        AgeEmulatorComponent
    ],
    imports: [
        BrowserModule
    ],
    providers: [],
    bootstrap: [AppComponent]
})
export class AppModule {
}
