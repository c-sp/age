import {Component} from '@angular/core';
import {VERSION_INFO} from '../environments/version';

@Component({
    selector: 'age-app-root',
    templateUrl: './app.component.html'
})
export class AppComponent {

    private _title = 'AGE-JS';

    get title(): string {
        return this._title;
    }

    get versionDate(): string {
        return VERSION_INFO.date;
    }

    get versionAuthor(): string {
        return VERSION_INFO.author;
    }

    get versionHash(): string {
        return VERSION_INFO.hash;
    }
}
