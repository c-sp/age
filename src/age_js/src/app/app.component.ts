import {Component} from '@angular/core';
import {VERSION_INFO} from '../environments/version';
import {AgeRomFile} from './rom-file-selector/age-rom-file';


// TODO h1 Text

@Component({
    selector: 'age-app-root',
    template: `
        <div class="container">

            <div>
                Welcome to the AGE-JS prototype
            </div>

            <div>
                <div><b>Build Details</b></div>
                <div>Commit Hash: {{versionHash}}</div>
                <div>Committed on: {{versionDate | date:'y-MM-dd HH:mm:ss'}}</div>
            </div>

            <div>
                <age-local-rom-file-selector (fileSelected)="selectFile($event)"></age-local-rom-file-selector>
            </div>

            <div>
                <table>
                    <tr>
                        <th colspan="4">Key Mappings</th>
                    </tr>
                    <tr>
                        <th>Gameboy</th>
                        <th>Keyboard</th>
                        <th>Gameboy</th>
                        <th>Keyboard</th>
                    </tr>
                    <tr>
                        <td>Up</td>
                        <td>Up Arrow</td>
                        <td>A</td>
                        <td>A</td>
                    </tr>
                    <tr>
                        <td>Down</td>
                        <td>Down Arrow</td>
                        <td>B</td>
                        <td>S</td>
                    </tr>
                    <tr>
                        <td>Left</td>
                        <td>Left Arrow</td>
                        <td>Start</td>
                        <td>Space</td>
                    </tr>
                    <tr>
                        <td>Right</td>
                        <td>Right Arrow</td>
                        <td>Select</td>
                        <td>Enter</td>
                    </tr>
                </table>
            </div>

            <div>
                <age-emulator-container *ngIf="romFile"
                                        [romFile]="romFile"></age-emulator-container>
            </div>

        </div>
    `,
    styles: [`
        .container {
            display: flex;
            height: 100%;
            flex-direction: column;
            align-items: center;
        }

        .container > div {
            margin-top: 2em;
            text-align: center;
        }

        .container > div:nth-child(1) {
            font-size: larger;
            font-weight: bold;
        }

        .container > div:nth-child(2) {
            font-size: smaller;
        }

        .container > div:nth-child(5) {
            flex: 1 1;
            width: 100%;
            background-color: black;
        }

        table th {
            font-style: italic;
            font-weight: normal;
            text-align: right;
        }

        table th:nth-child(even) {
            text-align: left;
            padding-left: .5em;
        }

        table tr:nth-child(1) th {
            font-style: normal;
            font-weight: bold;
            text-align: center;
        }

        table td {
            text-align: right;
        }

        table td:nth-child(even) {
            text-align: left;
            padding-left: .5em;
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
