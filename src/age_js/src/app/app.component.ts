import {Component} from '@angular/core';
import {VERSION_INFO} from '../environments/version';


// https://componenthouse.com/2018/02/15/how-to-make-angular-and-webassembly-work-together/


@Component({
    selector: 'age-app-root',
    templateUrl: './app.component.html'
})
export class AppComponent {

    private _versionInfo = VERSION_INFO;

    get versionDate(): string {
        return this._versionInfo.date;
    }

    get versionHash(): string {
        return this._versionInfo.hash;
    }
}
