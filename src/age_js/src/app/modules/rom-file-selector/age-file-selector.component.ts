import {ChangeDetectionStrategy, Component, EventEmitter, Output} from '@angular/core';
import {AgeRomFileToLoad} from '../common/age-rom-file-to-load';


@Component({
    selector: 'age-file-selector',
    template: `
        <div>
            <age-file-selector-local (fileSelected)="selectLocalFile($event)"></age-file-selector-local>
        </div>

        <age-file-selector-url (urlSelected)="selectFileFromURL($event)"></age-file-selector-url>
    `,
    styles: [`
        div {
            margin-bottom: .5em;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeFileSelectorComponent {

    @Output()
    readonly fileSelected = new EventEmitter<AgeRomFileToLoad>();

    selectLocalFile(file: File) {
        this.fileSelected.emit(AgeRomFileToLoad.forLocalFile(file));
    }

    selectFileFromURL(url: string) {
        this.fileSelected.emit(AgeRomFileToLoad.forUrl(url));
    }
}
