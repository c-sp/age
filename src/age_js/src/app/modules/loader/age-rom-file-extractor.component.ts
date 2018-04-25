import {ChangeDetectionStrategy, Component, EventEmitter, Input, Output} from '@angular/core';
import {AgeLoaderState} from './age-loader-state.component';


@Component({
    selector: 'age-rom-file-extractor',
    template: `
        <age-loader-state [state]="fileExtractorState">
            <ng-container ageLoaderWorking>extracting rom file ...</ng-container>
            <ng-container ageLoaderSuccess>rom file extracted</ng-container>
            <ng-container ageLoaderError>error extracting rom file</ng-container>
        </age-loader-state>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeRomFileExtractorComponent {

    @Output()
    readonly fileExtracted = new EventEmitter<ArrayBuffer>();

    private _extractorState: AgeLoaderState | undefined = undefined;


    get fileExtractorState(): AgeLoaderState | undefined {
        return this._extractorState;
    }

    @Input()
    set fileContents(fileContents: ArrayBuffer) {
        // TODO check for zip file and extract rom file
        setTimeout(() => this.fileExtracted.emit(fileContents), 0);
    }
}
