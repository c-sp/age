import {Component} from '@angular/core';
import {VERSION_INFO} from '../environments/version';
import {AgeRomFile} from './rom-file-selector/age-rom-file';


// TODO h1 Text

@Component({
    selector: 'age-app-root',
    template: `
        <div style="text-align:center">

            <h1>Welcome to the AGE-JS prototype</h1>

            <div>
                <div><b>Build Details</b></div>
                <div>Commit Hash: {{versionHash}}</div>
                <div>Committed on: {{versionDate | date:'y-MM-dd HH:mm:ss'}}</div>
            </div>

            <div class="container">
                <age-local-rom-file-selector (fileSelected)="selectFile($event)"></age-local-rom-file-selector>
            </div>

            <div class="container">
                <age-emulator-container *ngIf="romFile"
                                        [romFile]="romFile"></age-emulator-container>
            </div>

        </div>
    `,
    styles: [`
        .container {
            margin-top: 2em;
        }
    `]
})
export class AppComponent {

    romFile: AgeRomFile | undefined;

    private _versionInfo = VERSION_INFO;

    get versionDate(): string {
        return this._versionInfo.date;
    }

    get versionHash(): string {
        return this._versionInfo.hash;
    }

    selectFile(file: File): void {
        this.romFile = !file ? undefined : new AgeRomFile(file);
    }
}
