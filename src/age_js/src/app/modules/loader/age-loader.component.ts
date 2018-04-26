import {ChangeDetectionStrategy, ChangeDetectorRef, Component, EventEmitter, Input, Output} from '@angular/core';
import {AgeEmulationPackage} from '../common/age-emulation-package';
import {AgeRomFileToLoad} from '../common/age-rom-file-to-load';


@Component({
    selector: 'age-loader',
    template: `

        <!--
         The WASM loader is special in that it also unloads the WASM when destroyed.
         Thus we keep it alive the whole time, though not always visible.
         -->
        <age-wasm-loader [showState]="loading"
                         (emGbModuleLoaded)="emGbModuleLoaded($event)"></age-wasm-loader>

        <ng-container *ngIf="loading">

            <age-rom-file-loader #fileLoader
                                 [romFileToLoad]="romFileToLoad"></age-rom-file-loader>

            <age-rom-file-extractor [fileContents]="fileLoader.fileLoaded|async"
                                    (fileExtracted)="romFileExtracted($event)"></age-rom-file-extractor>

        </ng-container>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeLoaderComponent {

    @Output()
    readonly loadingComplete = new EventEmitter<AgeEmulationPackage>();

    private _emGbModule?: EmGbModule;
    private _romFileToLoad?: AgeRomFileToLoad;
    private _romFileContents?: ArrayBuffer;

    constructor(private _changeDetector: ChangeDetectorRef) {
    }


    @Input()
    set romFileToLoad(romFileToLoad: AgeRomFileToLoad | undefined) {
        // stop any loading process, if the file was cleared
        this._romFileToLoad = romFileToLoad;
        this._romFileContents = undefined;
    }

    get romFileToLoad(): AgeRomFileToLoad | undefined {
        return this._romFileToLoad;
    }

    get loading(): boolean {
        return !!this._romFileToLoad && (!this._emGbModule || !this._romFileContents);
    }


    emGbModuleLoaded(emGbModule?: EmGbModule): void {
        this._emGbModule = emGbModule;
        this.checkForLoadingComplete();
    }

    romFileExtracted(romFileContents: ArrayBuffer): void {
        this._romFileContents = romFileContents;
        this.checkForLoadingComplete();
    }


    private checkForLoadingComplete(): void {
        this._changeDetector.detectChanges();
        if (!!this._emGbModule && !!this._romFileContents) {
            this.loadingComplete.emit(new AgeEmulationPackage(this._emGbModule, this._romFileContents));
        }
    }
}
