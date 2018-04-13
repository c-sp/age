import {ChangeDetectionStrategy, Component, EventEmitter, Input, OnDestroy, Output} from '@angular/core';
import {AgeRomFile} from '../../rom-file-selector/age-rom-file';
import {AgeLoaderState} from './age-loader-state';


class FileLoader {

    private readonly fileReader: FileReader;

    constructor(file: File,
                onLoad: (fileContents: ArrayBuffer) => void,
                onError: (ev: ErrorEvent) => void) {

        this.fileReader = new FileReader();

        this.fileReader.onload = event => {
            /* tslint:disable:no-any */
            const eventTarget: any = event.target;
            /* tslint:enable:no-any */
            onLoad(eventTarget.result);
        };
        this.fileReader.onerror = event => {
            onError(event);
        };

        this.fileReader.readAsArrayBuffer(file);
    }

    abortReading(): void {
        // readyState LOADING = 1
        if (this.fileReader.readyState === 1) {
            this.fileReader.abort();
        }
    }
}


@Component({
    selector: 'age-rom-file-loader',
    template: `
        <ng-container *ngIf="showState">

            <age-loader-state [state]="fileLoaderState">
                <ng-container ageLoaderWorking>loading rom file ...</ng-container>
                <ng-container ageLoaderSuccess>rom file loaded</ng-container>
                <ng-container ageLoaderError>error loading rom file</ng-container>
            </age-loader-state>

        </ng-container>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeRomFileLoaderComponent implements OnDestroy {

    @Input()
    showState = false;

    @Output()
    readonly fileLoaded = new EventEmitter<ArrayBuffer>();

    private _loaderState: AgeLoaderState | undefined;
    private _fileLoader: FileLoader | undefined;

    ngOnDestroy(): void {
        this.cleanupLoader();
    }


    get fileLoaderState(): AgeLoaderState | undefined {
        return this._loaderState;
    }

    @Input()
    set romFile(romFile: AgeRomFile) {
        this.cleanupLoader(); // stop any ongoing loader before starting a new one

        this._fileLoader = new FileLoader(
            romFile.file,
            this.loaderFinished.bind(this),
            this.loaderError.bind(this)
        );

        this._loaderState = AgeLoaderState.WORKING;
    }


    private loaderFinished(fileContents: ArrayBuffer): void {
        this._loaderState = AgeLoaderState.SUCCESS;
        this.fileLoaded.emit(fileContents);
    }

    private loaderError(): void {
        this._loaderState = AgeLoaderState.ERROR;
    }

    private cleanupLoader(): void {
        if (this._fileLoader) {
            this._fileLoader.abortReading();
            this._fileLoader = undefined;
        }
    }
}
