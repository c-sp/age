import {Component} from '@angular/core';
import {VERSION_INFO} from '../environments/version';


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

            <div class="emulator">
                <button (click)="showEmulator = true">load emulator</button>
                <button (click)="showEmulator = false">exit emulator</button>
                <age-emulator-container *ngIf="showEmulator"></age-emulator-container>
            </div>

        </div>
    `,
    styles: [`
        .emulator {
            margin-top: 2em;
        }

        .emulator button {
            margin-bottom: 1em;
        }
    `]
})
export class AppComponent {

    showEmulator = false;

    private _versionInfo = VERSION_INFO;

    get versionDate(): string {
        return this._versionInfo.date;
    }

    get versionHash(): string {
        return this._versionInfo.hash;
    }
}
