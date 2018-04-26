import {ChangeDetectionStrategy, Component, ElementRef, EventEmitter, Output, ViewChild} from '@angular/core';


@Component({
    selector: 'age-file-selector-url',
    template: `
        Open gameboy rom file from URL:
        <!--<input #urlInput type="url" value="https://raw.githubusercontent.com/mills32/CUTE_DEMO/master/0_rom/CUTEDEMO.gbc">-->
        <input #urlInput type="url" value="http://gameboy.modermodemet.se/files/PHT-PZ.ZIP">
        <!--<input #urlInput type="url" value="http://www.pouet.net/prod.php?which=73290">-->
        <button (click)="openUrl()">open</button>
    `,
    styles: [`
        input {
            margin-left: 1em;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeFileSelectorURLComponent {

    readonly corsAnywhereUrl = 'https://cors-anywhere.herokuapp.com/';

    @Output()
    readonly urlSelected = new EventEmitter<string>();

    @ViewChild('urlInput')
    private _urlInput: ElementRef;

    openUrl(): void {
        const url = this._urlInput.nativeElement.value;
        if (!!url) {
            this.urlSelected.emit(this.corsAnywhereUrl + url);
        }
    }
}
