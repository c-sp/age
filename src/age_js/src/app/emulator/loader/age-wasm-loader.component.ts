import {ChangeDetectionStrategy, ChangeDetectorRef, Component, EventEmitter, Inject, Input, OnDestroy, OnInit, Output} from '@angular/core';
import {AgeLoaderState} from './age-loader-state';


const SCRIPT_ELEMENT_NAME = 'emscripten_age_wasm_module';


@Component({
    selector: 'age-wasm-loader',
    template: `
        <ng-container *ngIf="showState">

            <div>
                <age-loader-state-icon [state]="javascriptLoadingState"></age-loader-state-icon>

                <ng-container [ngSwitch]="javascriptLoadingState">
                    <ng-container *ngSwitchCase="AgeLoaderState.WORKING">loading Javascript ...</ng-container>
                    <ng-container *ngSwitchCase="AgeLoaderState.SUCCESS">Javascript loaded</ng-container>
                    <ng-container *ngSwitchCase="AgeLoaderState.ERROR">error loading Javascript</ng-container>
                </ng-container>
            </div>

            <div *ngIf="javascriptLoadingState === AgeLoaderState.SUCCESS">
                <age-loader-state-icon [state]="runtimeInitState"></age-loader-state-icon>

                <ng-container [ngSwitch]="runtimeInitState">
                    <ng-container *ngSwitchCase="AgeLoaderState.WORKING">initializing WebAssembly ...</ng-container>
                    <ng-container *ngSwitchCase="AgeLoaderState.SUCCESS">WebAssembly initialized</ng-container>
                    <ng-container *ngSwitchCase="AgeLoaderState.ERROR">error initializing WebAssembly</ng-container>
                </ng-container>
            </div>

        </ng-container>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeWasmLoaderComponent implements OnInit, OnDestroy {

    readonly AgeLoaderState = AgeLoaderState;

    @Input()
    showState = false;

    @Output()
    readonly emGbModuleLoaded = new EventEmitter<EmGbModule | undefined>();

    // I currently see no other way than to use "any" here
    /* tslint:disable:no-any */
    private _window: any = window;
    /* tslint:enable:no-any */
    private _javascriptLoadingState: AgeLoaderState | undefined;
    private _runtimeInitState: AgeLoaderState | undefined;


    constructor(@Inject(ChangeDetectorRef) private _changeDetector: ChangeDetectorRef) {
    }

    ngOnInit(): void {
        this.createEmscriptenModule();
        this.createScriptElement();
    }

    ngOnDestroy(): void {
        this.unloadEmscriptenModule();
        this.removeScriptElement();
    }


    get javascriptLoadingState(): AgeLoaderState | undefined {
        return this._javascriptLoadingState;
    }

    get runtimeInitState(): AgeLoaderState | undefined {
        return this._runtimeInitState;
    }


    private createEmscriptenModule(): void {
        if (this._window.Module) {
            console.warn('window.Module already defined!');
        }

        // augment the emscripten Module
        this._window.Module = {

            // tell the emscripten Module where to find the wasm binary
            locateFile: (file: string) => {
                return 'assets/' + file;
            },

            // notify us once WebAssembly compilation is complete
            onRuntimeInitialized: () => {
                this._runtimeInitState = AgeLoaderState.SUCCESS;
                this.emGbModuleLoaded.emit(this._window.Module);
                this._changeDetector.detectChanges();
            },

            // notify us about any initialization failure
            onAbort: () => {
                this._runtimeInitState = AgeLoaderState.ERROR;
                this._changeDetector.detectChanges();
            }
        };
    }

    private createScriptElement(): void {
        if (document.getElementById(SCRIPT_ELEMENT_NAME)) {
            console.warn(`script element "${SCRIPT_ELEMENT_NAME}" already exists!`);
        }

        // insert a script tag for loading the emscripten Module
        const script = document.createElement('script');
        script.id = SCRIPT_ELEMENT_NAME;
        script.src = 'assets/age_wasm.js';

        // notify us once the script has been successfully loaded
        script.onload = () => {
            this._javascriptLoadingState = AgeLoaderState.SUCCESS;
            this._runtimeInitState = AgeLoaderState.WORKING;
            this._changeDetector.detectChanges();
        };

        // notify us about any loading error
        script.onerror = () => {
            this._javascriptLoadingState = AgeLoaderState.ERROR;
            this._changeDetector.detectChanges();
        };

        document.body.appendChild(script);
        this._javascriptLoadingState = AgeLoaderState.WORKING;
    }

    private unloadEmscriptenModule(): void {
        const mod = this._window.Module;

        // if the Module.exit function is available, call it
        if (mod && mod.exit) {
            mod.noExitRuntime = false;

            try {
                mod.exit(0);
            }
            catch (err) {
                // ignore emscripten cleanup errors
            }
        }

        this._window.Module = undefined;
        this._runtimeInitState = undefined;

        this.emGbModuleLoaded.emit(undefined);
    }

    private removeScriptElement(): void {
        const script = document.getElementById(SCRIPT_ELEMENT_NAME);

        if (script) {
            document.body.removeChild(script);
        }

        this._javascriptLoadingState = undefined;
    }
}
