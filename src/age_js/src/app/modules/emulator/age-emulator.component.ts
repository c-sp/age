import {ChangeDetectionStrategy, ChangeDetectorRef, Component, HostListener, Inject, Input, OnDestroy, OnInit} from '@angular/core';
import {AgeEmulationRunner, AgeGbEmulation} from './age-emulation';
import {AgeGbKeyMap} from './age-emulator-keymap';
import {AgeRect} from '../common/age-rect';
import {AgeEmulationPackage} from '../common/age-emulation-package';


@Component({
    selector: 'age-emulator',
    template: `
        <ng-container *ngIf="emulationRunner as emulator">
            <age-canvas-renderer [screenSize]="emulator.screenSize"
                                 [newFrame]="emulator.screenBuffer"
                                 [viewport]="viewport"></age-canvas-renderer>
        </ng-container>
    `,
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeEmulatorComponent implements OnInit, OnDestroy {

    @Input() viewport = new AgeRect(1, 1);

    private readonly _keyMap = new AgeGbKeyMap();

    private _emulationRunner?: AgeEmulationRunner;
    private _timerHandle?: number;

    constructor(@Inject(ChangeDetectorRef) private readonly _changeDetector: ChangeDetectorRef) {
    }

    ngOnInit(): void {
        this._timerHandle = window.setInterval(
            () => {
                if (this._emulationRunner) {
                    this._emulationRunner.emulate();
                    this._changeDetector.detectChanges();
                }
            },
            10
        );
    }

    ngOnDestroy(): void {
        if (this._timerHandle) {
            window.clearInterval(this._timerHandle);
            this._timerHandle = undefined;
        }
    }


    get emulationRunner(): AgeEmulationRunner | undefined {
        return this._emulationRunner;
    }

    @Input()
    set emulationPackage(emulationPackage: AgeEmulationPackage | undefined) {
        if (!emulationPackage) {
            this._emulationRunner = undefined;
        } else {
            this._emulationRunner = new AgeEmulationRunner(
                new AgeGbEmulation(
                    emulationPackage.emGbModule,
                    emulationPackage.romFileContents
                )
            );
        }
    }


    @HostListener('document:keydown', ['$event'])
    handleKeyDown(event: KeyboardEvent) {
        const gbButton = this._keyMap.getButtonForKey(event.key);

        if (this._emulationRunner && gbButton) {
            this._emulationRunner.buttonDown(gbButton);
            event.preventDefault();
        }
    }

    @HostListener('document:keyup', ['$event'])
    handleKeyUp(event: KeyboardEvent) {
        const gbButton = this._keyMap.getButtonForKey(event.key);

        if (this._emulationRunner && gbButton) {
            this._emulationRunner.buttonUp(gbButton);
            event.preventDefault();
        }
    }
}
