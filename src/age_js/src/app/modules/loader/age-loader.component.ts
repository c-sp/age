import {ChangeDetectionStrategy, Component, EventEmitter, Input, Output} from '@angular/core';
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

            <age-rom-file-loader [romFileToLoad]="romFileToLoad"
                                 (fileLoaded)="romFileLoaded($event)"></age-rom-file-loader>

        </ng-container>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeLoaderComponent {

    @Output()
    readonly loadingComplete = new EventEmitter<AgeEmulationPackage>();

    private _loading = false;
    private _romFileToLoad: AgeRomFileToLoad | undefined = undefined;

    private _emGbModule: EmGbModule | undefined = undefined;
    private _romFileContents: ArrayBuffer | undefined = undefined;


    @Input()
    set loadRomFile(romFileToLoad: AgeRomFileToLoad | undefined) {
        // stop any loading process, if the file was cleared
        this._loading = !!romFileToLoad;
        this._romFileToLoad = romFileToLoad;
        this._romFileContents = undefined;
    }


    get loading(): boolean {
        return this._loading;
    }

    get romFileToLoad(): AgeRomFileToLoad | undefined {
        return this._romFileToLoad;
    }


    emGbModuleLoaded(emGbModule?: EmGbModule): void {
        this._emGbModule = emGbModule;
        this.checkForLoadingComplete();
    }

    romFileLoaded(romFileContents?: ArrayBuffer): void {
        this._romFileContents = romFileContents;
        this.checkForLoadingComplete();
    }

    private checkForLoadingComplete(): void {
        if (!!this._emGbModule && !!this._romFileContents) {
            this._loading = false;
            this.loadingComplete.emit(new AgeEmulationPackage(this._emGbModule, this._romFileContents));
        }
    }
}
