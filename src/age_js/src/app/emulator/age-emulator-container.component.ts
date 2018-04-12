import {ChangeDetectionStrategy, ChangeDetectorRef, Component, Inject} from '@angular/core';


@Component({
    selector: 'age-emulator-container',
    template: `
        <age-wasm-loader [showState]="!allLoaded"
                         (emGbModuleLoaded)="setEmGbModule($event)"></age-wasm-loader>

        <ng-container *ngIf="allLoaded">
            <age-emulator [emGbModule]="emGbModule"></age-emulator>
        </ng-container>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeEmulatorContainerComponent {

    private _allLoaded = false;
    private _emGbModule: EmGbModule | undefined;

    constructor(@Inject(ChangeDetectorRef) private _changeDetector: ChangeDetectorRef) {
    }


    get allLoaded(): boolean {
        return this._allLoaded;
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
        this._allLoaded = !!this._emGbModule;
        this._changeDetector.detectChanges();
    }
}
