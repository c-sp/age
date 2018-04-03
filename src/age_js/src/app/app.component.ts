import {Component, EventEmitter, Output} from '@angular/core';

@Component({
    selector: 'age-app-root',
    templateUrl: './app.component.html'
})
export class AppComponent {

    @Output()
    fooOutput = new EventEmitter<number>();

    private _counter = 0;
    private _title = 'AGE-JS';

    getTitle(foo: boolean): string {
        let result = this._title;

        if (foo) {
            ++this._counter;
            result = this._title + 'foo';
        }
        else {
            result = result + 'bar';
        }

        return result;
    }

    get title(): string {
        return this._title;
    }

    get counter(): number {
        return this._counter;
    }
}
