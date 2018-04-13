import {ChangeDetectionStrategy, Component, ElementRef, EventEmitter, Output, ViewChild} from '@angular/core';


@Component({
    selector: 'age-local-rom-file-selector',
    template: `
        <input #fileInput
               type="file"
               id="fileInput"
               accept=".gb, .gbc"
               (change)="selectFile()">
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeLocalFileSelectorComponent {

    @Output()
    readonly fileSelected = new EventEmitter<File>();

    @ViewChild('fileInput')
    private _fileInput: ElementRef;

    selectFile() {
        const files: FileList = this._fileInput.nativeElement.files;

        if (files && files.length) {
            this.fileSelected.emit(files[0]);
        }
    }
}
