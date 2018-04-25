import {ChangeDetectionStrategy, Component, EventEmitter, Input, OnDestroy, Output} from '@angular/core';
import {AgeLoaderState} from './age-loader-state.component';
import {AgeRomFileToLoad} from '../common/age-rom-file-to-load';
import {HttpClient, HttpErrorResponse} from '@angular/common/http';


@Component({
    selector: 'age-rom-file-loader',
    template: `
        <age-loader-state [state]="fileLoaderState">
            <ng-container ageLoaderWorking>loading rom file ...</ng-container>
            <ng-container ageLoaderSuccess>rom file loaded</ng-container>
            <ng-container ageLoaderError>error loading rom file</ng-container>
        </age-loader-state>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeRomFileLoaderComponent implements OnDestroy {

    @Output()
    readonly fileLoaded = new EventEmitter<ArrayBuffer>();

    private _loaderState: AgeLoaderState | undefined;
    private _fileReader: FileReader | undefined;

    constructor(private _httpClient: HttpClient) {
    }

    ngOnDestroy(): void {
        this.cleanupReader();
    }


    get fileLoaderState(): AgeLoaderState | undefined {
        return this._loaderState;
    }

    @Input()
    set romFileToLoad(romFileToLoad: AgeRomFileToLoad) {
        this.cleanupReader(); // stop any ongoing loader before starting a new one

        if (romFileToLoad.file) {
            this.loadLocalFile(romFileToLoad.file);

        } else if (romFileToLoad.url) {
            this.loadFileFromUrl(romFileToLoad.url);
        }

        this._loaderState = AgeLoaderState.WORKING;
    }


    private loadLocalFile(file: File) {
        this._fileReader = new FileReader();

        this._fileReader.onload = event => {
            /* tslint:disable:no-any */
            const eventTarget: any = event.target;
            /* tslint:enable:no-any */
            this.loaderFinished(eventTarget.result);
        };
        this._fileReader.onerror = () => {
            this.loaderError();
        };

        this._fileReader.readAsArrayBuffer(file);
    }

    private loadFileFromUrl(url: string) {
        this._httpClient.get(
            url,
            {
                responseType: 'arraybuffer',
                observe: 'response'
            }
        ).subscribe(
            httpResponse => {
                const contentType = httpResponse.headers.get('content-type');
                if (contentType !== 'application/octet-stream') {
                    throw new Error(`unknown content-type: ${contentType}`);
                }

                const fileContents = httpResponse.body;
                if (fileContents) {
                    this.loaderFinished(fileContents);
                } else {
                    this.loaderError();
                }
            },
            (httpErrorResponse: HttpErrorResponse) => {
                httpErrorResponse.headers.keys().map(key => {
                    console.log(`header ${key}: ${httpErrorResponse.headers.get(key)}`);
                });
                console.log(`error reading url ${url}`, httpErrorResponse);
            }
        );
    }


    private loaderFinished(fileContents: ArrayBuffer): void {
        this._loaderState = AgeLoaderState.SUCCESS;
        this.fileLoaded.emit(fileContents);
    }

    private loaderError(): void {
        this._loaderState = AgeLoaderState.ERROR;
    }

    private cleanupReader(): void {
        if (this._fileReader) {

            // readyState LOADING = 1
            if (this._fileReader.readyState === 1) {
                this._fileReader.abort();
            }

            this._fileReader = undefined;
        }
    }
}
