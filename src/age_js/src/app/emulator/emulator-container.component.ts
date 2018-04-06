import {ChangeDetectionStrategy, ChangeDetectorRef, Component, OnDestroy, OnInit} from '@angular/core';
import {TaskState} from './task-state';


const SCRIPT_ELEMENT_NAME = 'emscripten_age_wasm_module';


@Component({
    selector: 'age-emulator-container',
    template: `
        <div>

            <div>
                <age-task-state [state]="javascriptLoadingState"></age-task-state>

                <ng-container [ngSwitch]="javascriptLoadingState">
                    <ng-container *ngSwitchCase="TaskState.WORKING">loading Javascript ...</ng-container>
                    <ng-container *ngSwitchCase="TaskState.SUCCESS">Javascript loaded</ng-container>
                    <ng-container *ngSwitchCase="TaskState.ERROR">error loading Javascript</ng-container>
                </ng-container>
            </div>

            <div *ngIf="javascriptLoadingState === TaskState.SUCCESS">
                <age-task-state [state]="runtimeInitState"></age-task-state>

                <ng-container [ngSwitch]="runtimeInitState">
                    <ng-container *ngSwitchCase="TaskState.WORKING">initializing WebAssembly ...</ng-container>
                    <ng-container *ngSwitchCase="TaskState.SUCCESS">WebAssembly initialized</ng-container>
                    <ng-container *ngSwitchCase="TaskState.ERROR">error initializing WebAssembly</ng-container>
                </ng-container>
            </div>

            <canvas id="emu_screen" width="160" height="144"></canvas>

        </div>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class EmulatorContainerComponent implements OnInit, OnDestroy {

    readonly TaskState = TaskState;

    // I currently see no other way than to use "any" here
    /* tslint:disable:no-any */
    private _window: any = window;
    /* tslint:enable:no-any */

    private _javascriptLoadingState: TaskState | undefined;
    private _runtimeInitState: TaskState | undefined;


    constructor(private _changeDetector: ChangeDetectorRef) {
    }

    ngOnInit(): void {
        this.createEmscriptenModule();
        this.createScriptElement();
    }

    ngOnDestroy(): void {
        this.removeEmscriptenModule();
        this.removeScriptElement();
    }


    get javascriptLoadingState(): TaskState | undefined {
        return this._javascriptLoadingState;
    }

    get runtimeInitState(): TaskState | undefined {
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
                this._runtimeInitState = TaskState.SUCCESS;
                this._changeDetector.detectChanges();
            },

            // notify us about any initialization failure
            onAbort: () => {
                this._runtimeInitState = TaskState.ERROR;
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
            this._javascriptLoadingState = TaskState.SUCCESS;
            this._runtimeInitState = TaskState.WORKING;
            this._changeDetector.detectChanges();
        };

        // notify us about any loading error
        script.onerror = () => {
            this._javascriptLoadingState = TaskState.ERROR;
            this._changeDetector.detectChanges();
        };

        document.body.appendChild(script);
        this._javascriptLoadingState = TaskState.WORKING;
    }

    private removeEmscriptenModule(): void {
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
    }

    private removeScriptElement(): void {
        const script = document.getElementById(SCRIPT_ELEMENT_NAME);

        if (script) {
            document.body.removeChild(script);
        }

        this._javascriptLoadingState = undefined;
    }
}
