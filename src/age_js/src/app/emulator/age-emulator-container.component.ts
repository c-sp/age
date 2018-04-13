import {ChangeDetectionStrategy, ChangeDetectorRef, Component, Inject, Input} from '@angular/core';
import {AgeRomFile} from '../rom-file-selector/age-rom-file';


@Component({
    selector: 'age-emulator-container',
    template: `
        <age-wasm-loader [showState]="!allLoaded"
                         (emGbModuleLoaded)="setEmGbModule($event)"></age-wasm-loader>

        <age-rom-file-loader [showState]="!allLoaded"
                             [romFile]="romFile"
                             (fileLoaded)="setRomFileContents($event)"></age-rom-file-loader>

        <ng-container *ngIf="allLoaded">
            <age-emulator [emGbModule]="emGbModule"
                          [romFileContents]="romFileContents"></age-emulator>
        </ng-container>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeEmulatorContainerComponent {

    private _emGbModule: EmGbModule | undefined;
    private _romFile: AgeRomFile | undefined;
    private _romFileContents: ArrayBuffer | undefined;

    constructor(@Inject(ChangeDetectorRef) private _changeDetector: ChangeDetectorRef) {
    }

    get allLoaded(): boolean {
        return !!this._emGbModule && !!this._romFileContents;
    }


    get emGbModule(): EmGbModule {
        if (!this._emGbModule) {
            // should never happen, as the <age-emulator> component is guarded
            // with a check on allLoaded in the template
            throw new Error('EmGbModule not available');
        }
        return this._emGbModule;
    }

    setEmGbModule(emGbModule: EmGbModule | undefined): void {
        this._emGbModule = emGbModule;
        this._changeDetector.detectChanges();
    }


    get romFile(): AgeRomFile | undefined {
        return this._romFile;
    }

    @Input()
    set romFile(romFile: AgeRomFile | undefined) {
        this._romFile = romFile;
        this._romFileContents = undefined;
    }


    get romFileContents(): ArrayBuffer | undefined {
        return this._romFileContents;
    }

    setRomFileContents(romFileContents: ArrayBuffer | undefined): void {
        this._romFileContents = romFileContents;
        this._changeDetector.detectChanges();
    }
}
